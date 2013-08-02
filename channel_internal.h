/**
 * @file channel_internal.h
 * @brief 
 * @author colaghost
 * @version 0.0.1
 * @date 2013-08-02
 */

#ifndef CHANNEL_INTERNAL_H
#define CHANNEL_INTERNAL_H

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
  struct channel *c;
  void *val;
  unsigned int op;
  Task *t;
  int fd;
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
