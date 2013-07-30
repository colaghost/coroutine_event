#ifndef _TASK_H_
#define _TASK_H_
#include <stdio.h>
#include <ucontext.h>

enum
{
  STACK = 1024 * 1024,
};

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef struct Task Task;

struct Task
{
  Task *next;
  Task *prev;

  ucontext_t schedule_context;
  ucontext_t task_context;

  uchar *stk;
  uint stksize;
  int active;

  void (*startfn)(Task*, void*);
  void *startarg;
};

int task_create(void (*fn)(Task*, void*), void *arg, uint stack);
void task_free(Task *t);
void task_yield(Task *t);
void task_resume(Task *t);
void task_exit(Task *t);
void check_stack(Task *t, int n);

#endif
