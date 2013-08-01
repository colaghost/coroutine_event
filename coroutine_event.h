/**
 * @file coroutine_event.h
 * @brief 
 * @author colaghost
 * @version 0.0.1
 * @date 2013-07-31
 */

#ifndef _COROUTINE_EVENT_H_
#define _COROUTINE_EVENT_H_
#include <event.h>
#include <unistd.h>

#include "task.h"
#include "coroutine_internal.h"

typedef struct coroutine_scheduler
{
  struct Task *task;
  struct coroutine_io_map io_map;
}coroutine_scheduler_t;

typedef void (*coroutine_handler)(coroutine_scheduler_t*, int);

/**
 * @brief create a coroutine task, the handler will be run before return
 *
 * @param fd
 * @param handler
 *
 * @return -1 failed 0 success
 */
int coroutine_spawn(int fd, coroutine_handler handler);

/**
 * @brief green an fd
 *
 * @param scheduler
 * @param fd file descriptor
 *
 * @return 0 success -1 failed
 */
int coroutine_green(coroutine_scheduler_t *scheduler, int fd);
/**
 * @brief ungreen an fd
 *
 * @param scheduler
 * @param fd file descriptor
 *
 * @return 0 success -1 failed
 */
int coroutine_ungreen(coroutine_scheduler_t *scheduler, int fd);
/**
 * @brief use to read a file descriptor for data, it looks like sync,but actually it is async in low level.
 *
 * @param fd file descriptor that want to read
 * @param buf the buffer to save data
 * @param count the number of bytes of data to read
 * @param scheduler
 *
 * @return > 0 the count of data read < 0 error == 0 fd close
 */
ssize_t coroutine_read(int fd, 
                       void *buf, 
                       size_t count, 
                       coroutine_scheduler_t * scheduler);
/**
 * @brief write data to a file descriptor
 *
 * @param fd
 * @param buf
 * @param count
 *
 * @return 
 */
ssize_t coroutine_write(int fd, const void *buf, size_t count);

#endif
