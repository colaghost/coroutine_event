/**
 * @file channel.h
 * @brief 
 * @author colaghost
 * @version 1.0.0
 * @date 2013-08-02
 */


#ifndef CHANNEL_H
#define CHANNEL_H

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

int chan_sendul(channel_t *c, unsigned long *val, coroutine_t *ct);

int chan_sendul(channel_t *c, unsigned long *val, coroutine_t *ct);
#endif  /*CHANNEL_H*/
