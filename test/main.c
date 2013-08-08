/* just for test */
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "coroutine_event.h"
#include "log.h"
#include "channel.h"

static void request_handler(coroutine_t *ct, int fd, void *arg);
static void handle_accept(int sock, short event, void *arg);
static void accept_handler(coroutine_t *ct, int fd, void *arg);
static void signal_int(int signum);
static void process_request(coroutine_t *ct, int, void*);
static void* req_process_entrance(void *arg);

static int g_sock = 0;
struct event_base *g_ev_base[2];
channel_t *g_c = NULL;
struct coroutine_base *g_base[2];
static const char *resp = "HTTP/1.1 200 OK\r\n"
                          "Server: coroutine_event\r\n"
                          "Content-Length: 5\r\n\r\n"
                          "event";

int main()
{
  struct sockaddr_in local_addr;
  int flags;
  struct event ev;
  int reused;

  pthread_t tid;
  pthread_attr_t attr;

  signal(SIGINT, signal_int);

  g_sock = socket(AF_INET, SOCK_STREAM, 0);
  memset(&local_addr, 0, sizeof(local_addr));
  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons(9090);
  local_addr.sin_addr.s_addr = INADDR_ANY;
  flags = fcntl(g_sock, F_GETFL, 0);
  fcntl(g_sock, F_SETFL, flags | O_NONBLOCK);
  reused = 1;
  setsockopt(g_sock, SOL_SOCKET, SO_REUSEADDR, &reused, sizeof(reused));
  if (bind(g_sock, (struct sockaddr*)&local_addr, sizeof(local_addr)))
  {
    perror("bind failed");
    return -1;
  }
  listen(g_sock, 1024);

	g_c = channel_create(sizeof(unsigned long), 0);
  g_ev_base[0] = event_base_new();
  g_ev_base[1] = event_base_new();
  g_base[0] = coroutine_base_new(g_ev_base[0]);
  g_base[1] = coroutine_base_new(g_ev_base[1]);
  if (g_base[0] == NULL || g_base[1] == NULL)
  {
    perror("alloc coroutine_base  failed");
    return -1;
  }
  /*
  event_set(&ev, sock, EV_READ | EV_PERSIST, handle_accept, (void*)base);
  event_base_set(g_ev_base[0], &ev);
  event_add(&ev, NULL);
  */
	coroutine_spawn_with_fd(g_sock, 0, accept_handler, g_base[0], NULL);

	//coroutine_spawn(process_request, base, NULL);
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&tid, &attr, req_process_entrance, NULL);
  pthread_attr_destroy(&attr);

  event_base_loop(g_ev_base[0], 0);

  //event_del(&ev);
  close(g_sock);
  return 0;
}

static void request_handler(coroutine_t *ct, int fd, void *arg)
{
  char buf[128];
  ssize_t read_count;
  int flags;
	unsigned long val;
  size_t resp_len = strlen(resp);

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
    coroutine_write(fd, (void*)resp, resp_len, ct);
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
  chan_peer = chan_peer_create(g_c, ct);
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
  chan_peer = chan_peer_create(g_c, ct);
	while (1)
	{
		if (chan_recvl(chan_peer, &val) == 0)
		{
			new_fd = val;
			//printf("new fd:%d\n", new_fd);
			coroutine_spawn_with_fd(new_fd, 5, request_handler, g_base[1], NULL);
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
  if (g_sock > 0)
    close(g_sock);
  event_base_loopbreak(g_ev_base[0]);
  event_base_loopbreak(g_ev_base[1]);
}

static void* req_process_entrance(void *arg)
{
  coroutine_spawn(process_request, g_base[1], NULL);

  event_base_loop(g_ev_base[1], 0);

  pthread_exit(NULL);
}
