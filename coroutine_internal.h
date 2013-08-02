/**
 * @file coroutine_internal.h
 * @brief 
 * @author colaghost
 * @version 1.0.0
 * @date 2013-07-31
 */

#ifndef _COROUTINE_INTERNAL_H_
#define _COROUTINE_INTERNAL_H_

/**
 * @brief used to map fd to a ptr
 */
struct coroutine_io_map
{
  void **entries;
  int nentries;
};

struct event_base;

struct coroutine_base
{
  struct event_base *ev_base;
  struct coroutine_io_map io_map;
};

/**
 * @brief initialize an coroutine_io_map
 *
 * @param ctx
 */
void iomap_coroutine_init_map(struct coroutine_io_map *ctx);
/**
 * @brief free the space of the coroutine_io_map used
 *
 * @param ctx
 */
void iomap_coroutine_clear(struct coroutine_io_map *ctx);
/**
 * @brief add a ptr on a given fd
 *
 * @param map the map that operate on
 * @param fd the file descriptor corresponding to p
 * @param p the point to add
 *
 * @return -1 failed 0 success
 */
int iomap_fd_add(struct coroutine_io_map *map, int fd, void *p);
/**
 * @brief clear a ptr on a given fd
 *
 * @param map the map tha operate on 
 * @param fd file descriptor
 *
 * @return 
 */
int iomap_fd_del(struct coroutine_io_map *map, int fd);
/**
 * @brief get a ptr on a given fd
 *
 * @param map
 * @param fd
 *
 * @return 
 */
void* iomap_fd_get(struct coroutine_io_map *map, int fd);

#endif
