/**
 * @file channel.c
 * @brief 
 * @author colaghost
 * @version 1.0.0
 * @date 2013-08-02
 */
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

void task_schedule(Task *t)
{
  dlog_debug("%s\n", __func__);
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

int task_create(void (*fn)(Task*, void*), void *arg, uint stack)
{
  Task *t;
  t = task_alloc(fn, arg, stack);

  t->active = 1;
  t->exit_code = 0;
  task_schedule(t);

  return 0;
}

Task* task_create_noblock(void (*fn)(Task*, void*), void *arg, uint stack)
{
  Task *t;
  t = task_alloc(fn, arg, stack);

  t->active = 1;
  t->exit_code = 0;
  return t;
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

  free(t);
}
