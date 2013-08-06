/**
 * @file iomap.c
 * @brief 
 * @author colaghost
 * @version 1.0.0
 * @date 2013-07-31
 */
#include "coroutine_internal.h"

#include <stdlib.h>
#include <string.h>

void iomap_coroutine_init_map(struct coroutine_io_map *ctx)
{
  ctx->entries = 0;
  ctx->nentries = 0;
}

void iomap_coroutine_clear(struct coroutine_io_map *ctx)
{
  if (ctx->entries != NULL)
  {
    free(ctx->entries);
    ctx->entries = 0;
  }
  ctx->nentries = 0;
}

static int iomap_make_space(struct coroutine_io_map *map, int slot, int msize)
{
  if (map->nentries <= slot)
  {
    int nentries = map->nentries ? map->nentries : 32;
    void **tmp;

    while (nentries <= slot)
      nentries <<= 1;

    tmp = (void**)realloc(map->entries, nentries * msize);
    if (tmp == NULL)
      return -1;

    memset(&tmp[map->nentries], 0, (nentries - map->nentries) * msize);

    map->nentries = nentries;
    map->entries = tmp;
  }
  return 0;
}

int iomap_fd_add(struct coroutine_io_map *map, int fd, void *p)
{
  if (fd < 0)
    return 0;
  if (fd >= map->nentries)
  {
    if (iomap_make_space(map, fd, sizeof(void*)) == -1)
      return -1;
  }
  map->entries[fd] = p;
  return 0;
}

int iomap_fd_del(struct coroutine_io_map *map, int fd)
{
  if (fd < 0)
    return 0;
  if (map->nentries > fd)
    map->entries[fd] = NULL;
  return 0;
}

void* iomap_fd_get(struct coroutine_io_map *map, int fd)
{
  if (fd < 0 || map->nentries <= fd)
    return NULL;
  return map->entries[fd];
}
