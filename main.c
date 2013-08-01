#include "coroutine_event.h"
#include "log.h"

#include <string.h>
#include <unistd.h>
#include <fcntl.h>

static void request_handler(coroutine_scheduler_t *scheduler, int fd)
{
  char buf[128];
  ssize_t read_count;
  int flags;

  flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  while (1)
  {
    read_count = coroutine_read(fd, (void*)buf, sizeof(buf) - 1, scheduler);
    if (read_count <= 0)
    {
      dlog_debug("fd:%d close\n", fd);
      close(fd);
      return;
    }
    coroutine_write(fd, (void*)buf, read_count, scheduler);
  }
}

static void handle_accept(int sock, short event, void *arg)
{
  int new_fd;
  socklen_t addr_len;
  struct sockaddr_in cli_addr;
  addr_len = sizeof(cli_addr);
  new_fd = accept(sock, (struct sockaddr*)&cli_addr, &addr_len);
  if (new_fd < 0)
    return;
  dlog_debug("accept new fd:%d\n", new_fd);
  coroutine_spawn(new_fd, request_handler);
}

int main()
{
  struct sockaddr_in local_addr;
  int sock;
  int flags;
  struct event ev;
  int reused;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  memset(&local_addr, 0, sizeof(local_addr));
  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons(9090);
  local_addr.sin_addr.s_addr = INADDR_ANY;
  flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);
  reused = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reused, sizeof(reused));
  if (bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr)))
  {
    perror("bind failed");
    return -1;
  }
  listen(sock, 4);

  event_init();
  event_set(&ev, sock, EV_READ | EV_PERSIST, handle_accept, NULL);
  event_base_set(event_get_base(&ev), &ev);
  event_add(&ev, NULL);

  event_dispatch();

  event_del(&ev);
  close(sock);
  return 0;
}
