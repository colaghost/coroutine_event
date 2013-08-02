/**
 * @file coroutine_event.h
 * @brief 
 * @author colaghost
 * @version 1.0.0
 * @date 2013-07-31
 */

#ifndef _COROUTINE_EVENT_H_
#define _COROUTINE_EVENT_H_
#include <event.h>
#include <unistd.h>

#include "task.h"

struct coroutine_base;
struct event_base;

typedef struct coroutine
{
  struct coroutine_base *base;
  struct Task *task;
}coroutine_t;

typedef void (*coroutine_handler)(coroutine_t*, int);

struct coroutine_base* coroutine_base_new(struct event_base *ev_base);

/**
 * @brief create a coroutine task, the handler will be run before return
 *
 * @param fd
 * @param handler
 * @param base
 *
 * @return -1 failed 0 success
 */
int coroutine_spawn(int fd, coroutine_handler handler, struct coroutine_base *base);

/**
 * @brief green an fd
 *
 * @param ct
 * @param fd file descriptor
 *
 * @return 0 success -1 failed
 */
int coroutine_green(coroutine_t *ct, int fd);
/**
 * @brief ungreen an fd
 *
 * @param ct
 * @param fd file descriptor
 *
 * @return 0 success -1 failed
 */
int coroutine_ungreen(coroutine_t *ct, int fd);
/**
 * @brief use to read a file descriptor for data, it looks like sync,but actually it is async in low level.
 *
 * @param fd file descriptor that want to read
 * @param buf the buffer to save data
 * @param count the number of bytes of data to read
 * @param ct
 *
 * @return > 0 the count of data read < 0 error == 0 fd close
 */
ssize_t coroutine_read(int fd, 
                       void *buf, 
                       size_t count, 
                       coroutine_t * ct);
/**
 * @brief write data to a file descriptor
 *
 * @param fd
 * @param buf
 * @param count
 * @param ct
 *
 * @return > 0 the bytes of data that write <= 0 error
 */
ssize_t coroutine_write(int fd, 
                        const void *buf, 
                        size_t count, 
                        coroutine_t *ct);

#endif
