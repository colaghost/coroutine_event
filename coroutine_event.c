/**
 * @file coroutine_event.c
 * @brief 
 * @author colaghost
 * @version 0.0.1
 * @date 2013-07-31
 */
#include "coroutine_event.h"

#include <errno.h>
#include <stdlib.h>

#include "coroutine_internal.h"
#include "log.h"


struct coroutine_req
{
  coroutine_t *ct;
  coroutine_handler handler;
  int fd;
};

static void coroutine_set(coroutine_t *ct,
                          struct coroutine_base *base,
                          struct Task *task);
static void coroutine_task_schedule(int fd, short event, void *arg);
static void coroutine_spawn_handler(Task *task, void *arg);

struct coroutine_base* coroutine_base_new(struct event_base *ev_base)
{
  struct coroutine_base *base;
  base = (struct coroutine_base*)malloc(sizeof(struct coroutine_base));
  if (base)
  {
    base->ev_base = ev_base;
    iomap_coroutine_init_map(&(base->io_map));
  }
  return base;
}

int coroutine_spawn(int fd, coroutine_handler handler, struct coroutine_base *base)
{
  coroutine_t *ct = NULL;
  struct coroutine_req *req = NULL;
  Task *task = NULL;

  if (fd < 0)
    return -1;

  ct = (coroutine_t*)malloc(sizeof(coroutine_t));
  if (ct == NULL)
    return -1;

  do 
  {
    req = (struct coroutine_req*)malloc(sizeof(struct coroutine_req));
    if (req == NULL)
      break;

    req->ct = ct;
    req->handler = handler;
    req->fd = fd;

    task = task_create_noblock(coroutine_spawn_handler, (void*)req, STACK);
    if (task == NULL)
      break;

    coroutine_set(ct, base, task);

    if (coroutine_green(ct, fd))
      break;

    task_schedule(task);

    return 0;
  } while (0);

  if (ct)
    free(ct);
  if (req)
    free(req);
  return -1;
}

int coroutine_green(coroutine_t *ct, int fd)
{
  struct event *ev = NULL;
  if (fd < 0)
    return -1;
  do
  {
    ev = (struct event*)malloc(sizeof(struct event));
    if (ev == NULL)
      break;
    event_set(ev, fd, EV_READ | EV_PERSIST, coroutine_task_schedule, (void*)ct->task);
    event_base_set(ct->base->ev_base, ev);
    event_add(ev, NULL);
    if (iomap_fd_add(&(ct->base->io_map), fd, (void*)ev))
      break;
    return 0;
  } while (0);
  if (ev)
    free(ev);
  return -1;
}

int coroutine_ungreen(coroutine_t *ct, int fd)
{
  struct event *ev = NULL;
  if (fd < 0)
    return -1;
  ev = (struct event*)iomap_fd_get(&(ct->base->io_map), fd);
  if (ev)
  {
    event_del(ev);
    free(ev);
    iomap_fd_del(&(ct->base->io_map), fd);
  }
  return 0;
}

ssize_t coroutine_read(int fd, void *buf, size_t count, coroutine_t *ct)
{
  ssize_t read_bytes = 0;
  while (1)
  {
    read_bytes = read(fd, buf, count);
    if (read_bytes >= 0)
      break;
    else
    {
      if (errno == EINTR) 
        continue;
      else if (errno == EAGAIN || errno == EWOULDBLOCK)
      {
        dlog_debug("fd:%d has not data to read, task yield\n", fd);
        task_yield(ct->task);
      }
      else
        break;
    }
  }
  return read_bytes;
}

ssize_t coroutine_write(int fd, const void *buf, size_t count, coroutine_t *ct)
{
  ssize_t left_bytes = count;
  ssize_t write_bytes = 0;
  struct event ev;
  char first_write_flag = 1;
  const char *p;
  p = (const char*)buf;
  while (left_bytes > 0)
  {
    write_bytes = write(fd, p, left_bytes);
    if (write_bytes <= 0)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
      {
        if (first_write_flag)
        {
          first_write_flag = 0;
          event_set(&ev, fd, EV_WRITE | EV_PERSIST, coroutine_task_schedule, (void*)ct->task);
          event_base_set(ct->base->ev_base, &ev);
          event_add(&ev, NULL);
        }
        dlog_debug("fd:%d sock buffer is full, task yield\n", fd);
        task_yield(ct->task);
        continue;
      }
      else
      {
        break;
      }
    }
    left_bytes -= write_bytes;
    p += write_bytes;
  }

  /* cleaner */
  if (!first_write_flag)
  {
    event_del(&ev);
  }

  return left_bytes <= 0 ? count : write_bytes;
}

static void coroutine_set(coroutine_t *ct,
                          struct coroutine_base *base,
                          struct Task *task)
{
  ct->base = base;
  ct->task = task;
}

static void coroutine_task_schedule(int fd, short event, void *arg)
{
  Task *task;
  task = (Task*)arg;
  task_resume(task);
  if (!task->active)
    task_free(task);
}

static void coroutine_spawn_handler(Task *task, void *arg)
{
  struct coroutine_req *req = (struct coroutine_req*)arg;
  req->ct->task = task;
  if (req == NULL)
    return;
  req->handler(req->ct, req->fd);

  /* cleaner */
  coroutine_ungreen(req->ct, req->fd);
  close(req->fd);
  free(req->ct);
  free(req);
}
