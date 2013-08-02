/**
 * @file channel.c
 * @brief 
 * @author colaghost
 * @version 0.0.1
 * @date 2013-08-02
 */
#include "channel_internal.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"

channel_t* channel_create(int elem_size, int buf_size)
{
  channel_t *c;

  c = (channel_t*)malloc(sizeof(*c + bufsize * elemsize));
  if (c)
  {
    memset(c, 0, sizeof(*c));
    c->buf_size = bufsize;
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
    arr->a = realloc(arr->a, a->arr_size * sizeof(arr->a[0]));
  }
  arr->a[arr->elem_count++] = alt;
}

static void array_del_item(alt_array_t *arr, int i)
{
  if (i >= arr->elem_count)
    return;

  arr->a[i] = arr->a[--arr->elem_count];
}

#define otherop(op) (CHANSND + CHANRCV - (op))

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
  alt_array_t *a;
  channel_t *c;

  c = a->c;
  if (c->buf_size == 0)
  {
    ar = chan_get_array(c, otherop(a->op));
    return ar && ar->n;
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
  if (arr)
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
