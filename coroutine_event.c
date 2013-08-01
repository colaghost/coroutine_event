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

#include "log.h"


struct coroutine_req
{
  coroutine_scheduler_t *scheduler;
  coroutine_handler handler;
  int fd;
};

void coroutine_spawn_handler(Task *task, void *arg)
{
  struct coroutine_req *req = (struct coroutine_req*)arg;
  req->scheduler->task = task;
  if (req == NULL)
    return;
  req->handler(req->scheduler, req->fd);

  /* cleaner */
  coroutine_ungreen(req->scheduler, req->fd);
  close(req->fd);
  iomap_coroutine_clear(&(req->scheduler->io_map));
  free(req->scheduler);
  free(req);
}

int coroutine_spawn(int fd, void (*handler)(coroutine_scheduler_t *, int))
{
  coroutine_scheduler_t *scheduler = NULL;
  struct coroutine_req *req = NULL;
  Task *task = NULL;

  if (fd < 0)
    return -1;

  scheduler = (coroutine_scheduler_t*)malloc(sizeof(coroutine_scheduler_t));
  if (scheduler == NULL)
    return -1;

  scheduler->task = NULL;
  iomap_coroutine_init_map(&(scheduler->io_map));

  do 
  {
    req = (struct coroutine_req*)malloc(sizeof(struct coroutine_req));
    if (req == NULL)
      break;

    req->scheduler = scheduler;
    req->handler = handler;
    req->fd = fd;

    task = task_create_noblock(coroutine_spawn_handler, (void*)req, STACK);
    if (task == NULL)
      break;

    scheduler->task = task;

    if (coroutine_green(scheduler, fd))
      break;

    task_schedule(task);

    return 0;
  } while (0);

  if (scheduler)
    free(scheduler);
  if (req)
    free(req);
  return -1;
}

static void coroutine_task_schedule(int fd, short event, void *arg)
{
  Task *task;
  task = (Task*)arg;
  task_resume(task);
  if (!task->active)
    task_free(task);
}

int coroutine_green(coroutine_scheduler_t *scheduler, int fd)
{
  struct event *ev = NULL;
  if (fd < 0)
    return -1;
  do
  {
    ev = (struct event*)malloc(sizeof(struct event));
    if (ev == NULL)
      break;
    event_set(ev, fd, EV_READ | EV_PERSIST, coroutine_task_schedule, (void*)scheduler->task);
    event_base_set(event_get_base(ev), ev);
    event_add(ev, NULL);
    if (iomap_fd_add(&(scheduler->io_map), fd, (void*)ev))
      break;
    return 0;
  } while (0);
  if (ev)
    free(ev);
  return -1;
}

int coroutine_ungreen(coroutine_scheduler_t *scheduler, int fd)
{
  struct event *ev = NULL;
  if (fd < 0)
    return -1;
  ev = (struct event*)iomap_fd_get(&(scheduler->io_map), fd);
  if (ev)
  {
    event_del(ev);
    free(ev);
    iomap_fd_del(&(scheduler->io_map), fd);
  }
  return 0;
}

ssize_t coroutine_read(int fd, void *buf, size_t count, coroutine_scheduler_t *scheduler)
{
  ssize_t rd_count = 0;
  while (1)
  {
    rd_count = read(fd, buf, count);
    if (rd_count >= 0)
      break;
    else
    {
      if (errno == EINTR) 
        continue;
      else if (errno == EAGAIN || errno == EWOULDBLOCK)
      {
        dlog_debug("fd:%d has not data to read, task yield\n", fd);
        task_yield(scheduler->task);
      }
      else
        break;
    }
  }
  return rd_count;
}
