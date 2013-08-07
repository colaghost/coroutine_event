/* just for test */
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

#include "coroutine_event.h"
#include "log.h"
#include "channel.h"

static void request_handler(coroutine_t *ct, int fd, void *arg);
static void handle_accept(int sock, short event, void *arg);
static void accept_handler(coroutine_t *ct, int fd, void *arg);
static void signal_int(int signum);
static void process_request(coroutine_t *ct, int, void*);

static int sock = 0;
struct event_base *ev_base;
channel_t *c = NULL;
struct coroutine_base *base;

int main()
{
  struct sockaddr_in local_addr;
  int flags;
  struct event ev;
  int reused;
  //struct coroutine_base *base;

  signal(SIGINT, signal_int);

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

	c = channel_create(sizeof(unsigned long), 1);
  ev_base = event_base_new();
  base = coroutine_base_new(ev_base);
  if (base == NULL)
  {
    perror("alloc coroutine_base  failed");
    return -1;
  }
  /*
  event_set(&ev, sock, EV_READ | EV_PERSIST, handle_accept, (void*)base);
  event_base_set(ev_base, &ev);
  event_add(&ev, NULL);
  */
	coroutine_spawn_with_fd(sock, 0, accept_handler, base, NULL);

	coroutine_spawn(process_request, base, NULL);

  event_base_loop(ev_base, 0);

  //event_del(&ev);
  close(sock);
  return 0;
}

static void request_handler(coroutine_t *ct, int fd, void *arg)
{
  char buf[128];
  ssize_t read_count;
  int flags;
	unsigned long val;

#if 0
  flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#endif

  while (1)
  {
		read_count = coroutine_read(fd, (void*)buf, sizeof(buf) - 1, ct);
		if (read_count <= 0)
    {
      if (errno == ETIMEDOUT)
        dlog_debug("fd:%d timedout\n", fd);
      dlog_debug("fd:%d close\n", fd);
      close(fd);
      return;
    }
    coroutine_write(fd, (void*)buf, read_count, ct);
  }
}

static void accept_handler(coroutine_t *ct, int fd, void *arg)
{
  int flags;
	socklen_t addr_len;
	int new_fd;
	struct sockaddr_in cli_addr;
	long val = 520;
  chan_peer_t *chan_peer;

#if 0
  flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#endif
	addr_len = sizeof(cli_addr);
  chan_peer = chan_peer_create(c, ct);
	while (1)
	{
		new_fd = coroutine_accept(fd, (struct sockaddr*)&cli_addr, &addr_len, ct);
		if (new_fd < 0)
		{
			dlog_err("accept failed\n");
			close(fd);
			abort();
		}
		val = new_fd;
		chan_sendl(chan_peer, val);
	}
}

static void process_request(coroutine_t *ct, int fd, void *arg)
{
	long val;
	int new_fd;
  chan_peer_t *chan_peer;
  chan_peer = chan_peer_create(c, ct);
	while (1)
	{
		if (chan_recvl(chan_peer, &val) == 0)
		{
			new_fd = val;
			//printf("new fd:%d\n", new_fd);
			coroutine_spawn_with_fd(new_fd, 5, request_handler, base, NULL);
		}
	}
}

static void handle_accept(int sock, short event, void *arg)
{
  int new_fd;
  socklen_t addr_len;
  struct sockaddr_in cli_addr;
  struct coroutine_base *base;
  addr_len = sizeof(cli_addr);
  base = (struct coroutine_base*)arg;
  new_fd = accept(sock, (struct sockaddr*)&cli_addr, &addr_len);
  if (new_fd < 0)
    return;
  dlog_debug("accept new fd:%d\n", new_fd);
  coroutine_spawn_with_fd(new_fd, 0, request_handler, base, NULL);
}


static void signal_int(int signum)
{
  if (sock > 0)
    close(sock);
  event_base_loopbreak(ev_base);
}
