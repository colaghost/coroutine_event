/**
 * @file channel.c
 * @brief 
 * @author colaghost
 * @version 1.0.0
 * @date 2013-08-02
 */
#include "channel_internal.h"

#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "log.h"
#include "task.h"
#include "coroutine_internal.h"
#include "coroutine_event.h"

channel_t* channel_create(int elem_size, int buf_size)
{
  channel_t *c;

  c = (channel_t*)malloc(sizeof *c + buf_size * elem_size);
  if (c)
  {
    memset(c, 0, sizeof(*c));
    c->buf_size = buf_size;
    c->elem_size = elem_size;
    c->nbuf = 0;
    c->buf = (unsigned char*)(c + 1);
  }
  return c;
}

static void array_add_item(alt_array_t *arr, alt_t *alt)
{
  if (arr->arr_size == arr->elem_count)
  {
    arr->arr_size += 16;
    arr->a = realloc(arr->a, arr->arr_size * sizeof(arr->a[0]));
  }
  arr->a[arr->elem_count++] = alt;
}

static void array_del_item(alt_array_t *arr, int i)
{
  if (i >= arr->elem_count)
    return;

  arr->a[i] = arr->a[--arr->elem_count];
}

#define OTHER_OP(op) (CHANSND + CHANRCV - (op))

static alt_array_t* chan_get_array(channel_t *c, unsigned int op)
{
  switch (op)
  {
    case CHANSND:
      return &c->asend;
    case CHANRCV:
      return &c->arecv;
  }
  return NULL;
}

static int alt_can_exec(alt_t *a)
{
  alt_array_t *ar;
  channel_t *c;

  c = a->c;
  if (c->buf_size == 0)
  {
    ar = chan_get_array(c, OTHER_OP(a->op));
		dlog_debug("%s:%d\n", __func__, ar && ar->elem_count);
    return ar && ar->elem_count;
  }
  else
  {
    switch (a->op)
    {
      case CHANSND:
        return c->nbuf < c->buf_size;
      case CHANRCV:
        return c->nbuf > 0;
      default:
        return 0;
    }
  }
  return 0;
}

static void alt_queue(alt_t *a)
{
  alt_array_t *ar;

  ar = chan_get_array(a->c, a->op);
  if (ar)
    array_add_item(ar, a);
}

static void alt_dequeue(alt_t *a)
{
  int i;
  alt_array_t *ar;

  ar = chan_get_array(a->c, a->op);
  if (ar == NULL)
  {
    return;
  }
  for (i = 0; i < ar->elem_count; ++i)
  {
    if (ar->a[i] == a)
    {
      array_del_item(ar, i);
      return;
    }
  }
}

static void alt_all_dequeue(alt_t *a)
{
	int i; 
	for (i = 0; a[i].op != CHANEND && a[i].op != CHANNOBLK; ++i)
	{
		alt_dequeue(&a[i]);
	}
}

static void amove(void *dst, void *src, unsigned int n)
{
	if (dst)
	{
		if (src == NULL)
			memset(dst, 0, n);
		else
			memmove(dst, src, n);
	}
}

static void alt_copy(alt_t *send, alt_t *recv)
{
	alt_t *t;
	channel_t *c;
	unsigned char *cp;

	if (send == NULL && recv == NULL)
		return;
	assert(send != NULL);
	c = send->c;
	if (send->op == CHANRCV)
	{
		t = send;
		send = recv;
		recv = t;
	}
	assert(send == NULL || send->op == CHANSND);
	assert(recv == NULL || recv->op == CHANRCV);

	if (send && recv && c->nbuf == 0)
	{
		amove(recv->val, send->val, c->elem_size);
		return;
	}

	if (recv)
	{
		cp = c->buf + c->off * c->elem_size;
		amove(recv->val, cp, c->elem_size);
		--c->nbuf;
		if (++c->off == c->buf_size)
			c->off = 0;
	}

	if (send)
	{
		cp = c->buf + (c->off + c->nbuf) % c->buf_size * c->elem_size;
		amove(cp, send->val, c->elem_size);
		++c->nbuf;
	}
}

static void alt_signal(alt_t *a)
{
	write(a->pipe_fd[1], "o", 1);
}

static void alt_exec(alt_t *a)
{
	int i;
	alt_array_t *ar;
	alt_t *other;
	channel_t *c;

	c = a->c;
	ar = chan_get_array(c, OTHER_OP(a->op));
	if (ar && ar->elem_count)
	{
		i = rand() % ar->elem_count;
		other = ar->a[i];
		alt_copy(a, other);
		alt_all_dequeue(other->xalt);
		other->xalt[0].xalt = other;
		alt_signal(other);
	}
	else
		alt_copy(a, NULL);
}

int chan_alt(alt_t *a, coroutine_t *ct)
{
	int loop, rand_index, ncan, stop_index, canblock;
	ssize_t read_bytes;
	char c;

	for (loop = 0; 
			 a[loop].op != CHANEND && a[loop].op != CHANNOBLK;
			 ++loop);
	stop_index = loop;
	canblock = a[stop_index].op == CHANEND;

	for (loop = 0; loop < stop_index; ++loop)
	{
		a[loop].t = ct->task;
		a[loop].xalt = a;
	}

	ncan = 0;
	for (loop = 0; loop < stop_index; ++loop)
	{
		if (alt_can_exec(&a[loop]))
				++ncan;
	}
	if (ncan)
	{
		rand_index = rand() % ncan;
		for (loop = 0; loop < stop_index; ++loop)
		{
			if (alt_can_exec(&a[loop]))
			{
				if (rand_index-- == 0)
				{
					alt_exec(&a[loop]);
					return loop;
				}
			}
		}
	}
	if (!canblock)
		return -1;

	for (loop = 0; loop < stop_index; ++loop)
	{
		alt_queue(&a[loop]);
	}

	read_bytes = coroutine_read(a[0].pipe_fd[0], &c, 1, ct);
	if (read_bytes < 1)
		return -1;

	return a[0].xalt - a;
}

static int chan_op(channel_t *c, int op, void *p, coroutine_t *ct, int canblock)
{
	int flags;
	alt_t a[2];
	int op_result;
	if (pipe(a[0].pipe_fd))
		return -1;
  flags = fcntl(a[0].pipe_fd[0], F_GETFL, 0);
  fcntl(a[0].pipe_fd[0], F_SETFL, flags | O_NONBLOCK);
	if (coroutine_green(ct, a[0].pipe_fd[0]))
		return -1;
	a[0].c = c;
	a[0].op = op;
	a[0].val = p;
	a[1].op = canblock ? CHANEND : CHANNOBLK;
	op_result = chan_alt(a, ct);
	coroutine_ungreen(ct, a[0].pipe_fd[0]);
	close(a[0].pipe_fd[0]);
	close(a[0].pipe_fd[1]);
	return op_result < 0 ? -1 : 0;
}

int chan_sendul(channel_t *c, unsigned long *val, coroutine_t *ct)
{
	return chan_op(c, CHANSND, val, ct, 1);
}

int chan_recvul(channel_t *c, unsigned long *val, coroutine_t *ct)
{
	return chan_op(c, CHANRCV, val, ct, 1);
}
