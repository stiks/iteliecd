#ifndef __LOG_H__
#define __LOG_H__

#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>

typedef enum {
   	ITELIEC_ERR,
   	ITELIEC_WARN,
   	ITELIEC_INFO,
} iteliec_log_priority;

void iteliec_log_init (int no_daemon, int noisy);
void iteliec_log_close (void);
void iteliec_log (int priority, const char *fmt, ...)
  __attribute__((format (printf, 2, 3)));

#endif /* __LOG_H__ */
