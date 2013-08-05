/**
 * @file channel.c
 * @brief 
 * @author colaghost
 * @version 1.0.0
 * @date 2013-08-02
 */
#ifndef _LOG_H_
#define _LOG_H_
#include <stdio.h>

#define dlog_info(args...) fprintf(stdout, args)

#ifdef USE_DEBUG
#define dlog_debug(args...) fprintf(stdout, args)
#else
#define dlog_debug(args...) do {;} while (0)
#endif

#define dlog_err(args...) fprintf(stderr, args)

#endif
