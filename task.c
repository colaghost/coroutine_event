#include <signal.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "task.h"
#include "log.h"

static void task_start(uint y, uint x)
{
  Task *t;
  ulong z;

  z = x << 16;
  z <<= 16;
  z |= y;
  t = (Task*)z;

  t->startfn(t, t->startarg);
  task_exit(t);
}

static Task* task_alloc(void (*fn)(Task*, void*), void *arg, uint stack)
{
  Task *t;
  sigset_t zero;
  uint x, y;
  ulong z;

  t = (Task*)malloc(sizeof(*t) + stack);
  if (t == NULL)
  {
    dlog_err("task_alloc malloc: %s\n", strerror(errno));
    abort();
  }
  memset(t, 0, sizeof *t);
  t->stk = (uchar*)(t + 1);
  t->stksize = stack;
  t->startfn = fn;
  t->startarg = arg;

  sigemptyset(&zero);
  sigprocmask(SIG_BLOCK, &zero, &t->task_context.uc_sigmask);

  if (getcontext(&t->task_context) < 0)
  {
    dlog_err("getcontext: %s\n", strerror(errno));
    abort();
  }

  t->task_context.uc_stack.ss_sp = t->stk;
  t->task_context.uc_stack.ss_size = t->stksize;
  t->task_context.uc_link = &t->schedule_context;

  z = (ulong)t;
  y = z;
  z >>= 16;
  x = z >> 16;
  makecontext(&t->task_context, (void(*)())task_start, 2, y, x);

  return t;
}

static void task_schedule(/*int fd, short event, void *arg*/Task *t)
{
  /*
  char buf;
  Task *t;
  */

  dlog_debug("%s\n", __func__);
  /*
  read(fd, (void*)&buf, 1);
  t = (Task*)arg;
  if (!t)
    return;
  */
  task_resume(t);
  if (!t->active)
    task_free(t);
  dlog_debug("%s end\n", __func__);
}

void task_exit(Task *t)
{
  t->active = 0;
  task_yield(t);
}

int task_create(/*event_base *ev_base, */void (*fn)(Task*, void*), void *arg, uint stack)
{
  Task *t;
  t = task_alloc(fn, arg, stack);

  /*
  if (pipe(t->pipe_fd) < 0)
  {
    task_free(t);
    return -1;
  }

  event_set(&t->ev, t->pipe_fd[0], EV_READ, task_schedule, (void*)t);
  event_base_set(ev_base, &t->ev);
  event_add(&t->ev, NULL);
  write(t->pipe_fd[1], "c", 1);
  */

  t->active = 1;
  task_schedule(t);

  return 0;
}

void task_yield(Task *t)
{
  check_stack(t, 0);
  swapcontext(&t->task_context, &t->schedule_context);
}

void task_resume(Task *t)
{
  swapcontext(&t->schedule_context, &t->task_context);
}

void check_stack(Task *t, int n)
{
  Task *tmp;

  if ((char*)&tmp <= (char*)t->stk ||
      (char*)&tmp - (char*)t->stk < n)
  {
    dlog_err("task stack overflow: &t=%p tstk=%p n=%d\n",
             &tmp,
             t->stk,
             n);
    abort();
  }
}

void task_free(Task *t)
{
  dlog_debug("%s\n", __func__);
  assert(t);
  /*
  if (t->pipe_fd[0] > 0)
    close(t->pipe_fd[0]);
  if (t->pipe_fd[1] > 0)
    close(t->pipe_fd[1]);

  event_del(&t->ev);
  */

  free(t);
}
