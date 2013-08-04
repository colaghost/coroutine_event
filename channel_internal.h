/**
 * @file channel_internal.h
 * @brief 
 * @author colaghost
 * @version 1.0.0
 * @date 2013-08-02
 */

#ifndef CHANNEL_INTERNAL_H
#define CHANNEL_INTERNAL_H
#include <event.h>

enum
{
  CHANEND,
  CHANSND,
  CHANRCV,
  CHANNOBLK,
};

struct channel;
struct Task;

typedef struct alt
{
  int pipe_fd[2];
	struct event ev;
  struct channel *c;
  void *val;
  unsigned int op;
  struct Task *t;
  struct alt *xalt;
}alt_t;

typedef struct alt_array
{
  alt_t **a;
  unsigned int arr_size;
  unsigned int elem_count;
}alt_array_t;

typedef struct channel
{
  unsigned int buf_size;
  unsigned int elem_size;
  unsigned char *buf;
  unsigned int nbuf;
  unsigned int off;
  alt_array_t asend;
  alt_array_t arecv;
  char *name;
}channel_t;


#endif  /*CHANNEL_INTERNAL_H*/
