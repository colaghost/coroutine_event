/**
 * @file channel.h
 * @brief 
 * @author colaghost
 * @version 1.0.0
 * @date 2013-08-02
 */


#ifndef CHANNEL_H
#define CHANNEL_H

/**
 * @brief actually the channel like a block queue,
 *        but it isn't fifo now, it should be improved.
 */
typedef struct channel channel_t;

typedef struct chan_peer chan_peer_t;

struct coroutine;

/**
 * @brief create a new channel
 * the memory layout like this:
 *
 *   |                 |          |          |
 *  {|sizeof(channel_t)|elemsize  |elemsize  |...}
 *                    {|elemsize * bufsize      |}
 *
 * @param elemsize element size
 * @param bufsize buffer size
 *
 * @return the point to the new channel
 */
channel_t* channel_create(int elemsize, int bufsize);

/**
 * @brief free channel
 *
 * @param c
 */
void channel_free(channel_t *c);

/**
 * @brief create a new chan_peer structure
 *        when we read/write data from/to channel,all base on
 *        this structure.It's not thread safe.
 *
 * @param c
 * @param ct
 *
 * @return 
 */
chan_peer_t* chan_peer_create(channel_t *c, struct coroutine *ct);

void chan_peer_free(chan_peer_t *chan_peer);

/**
 * @brief send data from the buffer pointed val to the channel
 *        the bytes of the val should be less than or equal the 
 *        elemsize of the channel when you create it
 *        when the channel is full, it'll be blocked
 *
 * @param chan_peer
 * @param val
 *
 * @return 
 */
int chan_send(chan_peer_t *chan_peer, void *val);

/**
 * @brief recv data from channel into the buufer starting at val
 *        when the channel is empty, it'll be blocked
 *
 * @param chan_peer
 * @param val
 *
 * @return 
 */
int chan_recv(chan_peer_t *chan_peer, void *val);

/**
 * @brief the same as chan_send, but it'll not be blocked when
 *        the channel is full
 *
 * @param chan_peer
 * @param val
 *
 * @return 
 */
int chan_send_nb(chan_peer_t *chan_peer, void *val);

/**
 * @brief the same as chan_recv, but it'll not be blocked when
 *        the channel is empty
 *
 * @param chan_peer
 * @param val
 *
 * @return 
 */
int chan_recv_nb(chan_peer_t *chan_peer, void *val);

/**
 * @brief send an long val to the channel,
 *        when the channel is full, it'll be blocked
 *
 * @param c the channel
 * @param val the val you want to send
 * @param ct the coroutine 
 *
 * @return 
 */
int chan_sendl(chan_peer_t *chan_peer, long val);

/**
 * @brief recv an long val from the channel
 *        when the channel is empty, it'll be blocked
 *
 * @param c
 * @param val
 * @param ct
 *
 * @return 
 */
int chan_recvl(chan_peer_t *chan_peer, long *val);
#endif  /*CHANNEL_H*/
