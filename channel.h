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
 * @brief send an unsigned long val to the channel,
 *        when the channel is full, it'll be blocked
 *
 * @param c the channel
 * @param val the val you want to send
 * @param ct the coroutine 
 *
 * @return 
 */
int chan_sendul(channel_t *c, unsigned long *val, coroutine_t *ct);

/**
 * @brief recv an unsigned long val from the channel
 *        when the channel is empty, it'll be blocked
 *
 * @param c
 * @param val
 * @param ct
 *
 * @return 
 */
int chan_recvul(channel_t *c, unsigned long *val, coroutine_t *ct);
#endif  /*CHANNEL_H*/
