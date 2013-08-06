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

typedef struct coroutine coroutine_t;

/**
 * @brief the coroutine function handler
 *
 * @param coroutine_t*
 * @param int the fd you pass, if you call with coroutine_spawn,
 *        it should be ignored.
 * @param void* arg pointer
 *
 * @return 
 */
typedef void (*coroutine_handler)(coroutine_t*, int, void*);

struct coroutine_base* coroutine_base_new(struct event_base *ev_base);

/**
 * @brief create a coroutine task with a fd, the fd will  
 *        be green, the handler will run before return
 *
 * @param fd
 * @param timeout
 * @param handler
 * @param base
 * @param arg
 *
 * @return -1 failed 0 success
 */
int coroutine_spawn_with_fd(int fd, int timeout, coroutine_handler handler, struct coroutine_base *base, void *arg);

/**
 * @brief create a coroutine task, the handler will run before return
 *
 * @param handler
 * @param base
 * @param arg
 *
 * @return 
 */
int coroutine_spawn(coroutine_handler handler, struct coroutine_base *base, void *arg);

/**
 * @brief Green an fd, so you can use the coroutine_read[accept] to 
 *        read data and so on.
 *        When you call this function on a fd, the fd will be add to 
 *        event_loop, and please ensure the fd you pass is noblock.
 *
 * @param ct
 * @param fd file descriptor
 *
 * @return 0 success -1 failed
 */
int coroutine_green(coroutine_t *ct, int fd, int timeout);
/**
 * @brief ungreen an fd
 *
 * @param ct
 * @param fd file descriptor
 * @param timeout
 *
 * @return 0 success -1 failed
 */
int coroutine_ungreen(coroutine_t *ct, int fd);

/* the functions behind uses like sync, but actually it runs async in low level.
 * you can use the coroutine_read like this:
 * while (1)
 * {
 *  read_bytes = coroutine_read(fd, buf, count, ct);
 *  process(buf);
 * }
 * and you don't have to worry about it will block the thread,
 * when you use the coroutine_read and coroutine_accept, you should call the 
 * coroutine_green first(just one time), except the coroutine_write.
 * the fd should be noblock, otherwise it may block the thread.
 */

/**
 * @brief attempts to read up to count bytes from file 
 *        descriptor fd into the buffer starting at buf.
 *
 * @param fd
 * @param buf
 * @param count
 * @param ct
 *
 * @return > 0 the count of data read < 0 error == 0 fd close
 *             when timeout return < 0 and set the errorno = ETIMEDOUT
 */
ssize_t coroutine_read(int fd, 
                       void *buf, 
                       size_t count, 
                       coroutine_t * ct);
/**
 * @brief writes up to count bytes from the buffer pointed
 *        buf to the file referred to the file descriptor fd
 *
 * @param fd
 * @param buf
 * @param count
 * @param ct
 *
 * @return > 0 the bytes of data that write <= 0 error
 *             when timeout return < 0 and set the errorno = ETIMEDOUT
 */
ssize_t coroutine_write(int fd, 
                        const void *buf, 
                        size_t count, 
                        coroutine_t *ct);

/**
 * @brief extracts the first connection request on the queue
 *        of pending connection from the listening socket,fd,
 *        creates a new connected socket, and returns a new 
 *        file descriptor referring to that socket
 *
 * @param fd tcp socket that listen
 * @param cli_addr a pointer to a sockaddr structure.It is filled
 *                 in with the address of the peer socket.
 * @param addr_len the bytes of structure that the cli_addr point to,
 *                 base on protol, like sizeof(struct sockaddr_in)
 * @param ct
 *
 * @return > 0 the new fd < 0 error
 *             when timeout return < 0 and set the errorno = ETIMEDOUT
 */
int coroutine_accept(int fd, 
										 struct sockaddr *cli_addr, 
										 socklen_t *addr_len, 
										 coroutine_t *ct);

/**
 * @brief connects the socket referred  to by the file descriptor fd
 *        to the address specified by addr
 *
 * @param fd
 * @param addr
 * @param addr_len
 * @param ct
 *
 * @return 0 success < 0 error, and errno is set appropriately
 */
int coroutine_connect(int fd,
                      const struct sockaddr *addr,
                      socklen_t addr_len,
                      coroutine_t *ct);
#endif
