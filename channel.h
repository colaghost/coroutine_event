/**
 * @file channel.h
 * @brief 
 * @author colaghost
 * @version 0.0.1
 * @date 2013-08-02
 */


#ifndef CHANNEL_H
#define CHANNEL_H

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

#endif  /*CHANNEL_H*/
