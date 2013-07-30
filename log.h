#ifndef _LOG_H_
#define _LOG_H_

#define dlog_info(args...) fprintf(stdout, args)
#define dlog_debug(args...) fprintf(stdout, args)
#define dlog_err(args...) fprintf(stderr, args)

#endif
