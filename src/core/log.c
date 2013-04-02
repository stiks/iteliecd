#include "log.h"

static int verbose = 0;
static int no_daemonize = 0;

void iteliec_log_init (int no_daemon, int noisy) {
   	no_daemonize = no_daemon;
   	verbose = noisy;

   	openlog ("iteliec-client", 0, 0);
}

void iteliec_log_close (void) {
   	if (!no_daemonize) {
		closelog ();
   	}
}

void iteliec_log (int priority, const char *fmt, ...) {
   	va_list args;

   	va_start (args, fmt);

   	if (!no_daemonize) {
      	int sysprio = -1;

      	switch (priority) {
         	case ITELIEC_ERR:
            	sysprio = LOG_ERR;
            break;

         	case ITELIEC_WARN:
            	sysprio = LOG_WARNING;
            break;

         	case ITELIEC_INFO:
            	if (verbose) {
               		sysprio = LOG_INFO;
           		}
            break;

         	default:
            break;
      	}

      	if (sysprio != -1) {
        	vsyslog (sysprio, fmt, args);
        }
   	} else {
      	switch (priority) {
         	case ITELIEC_ERR:
         	case ITELIEC_WARN:
            	vfprintf (stderr, fmt, args);
            	fputc ('\n', stderr);
            break;

         	case ITELIEC_INFO:
            	if (verbose) {
               		vprintf (fmt, args);
               		fputc ('\n', stdout);
            	}
            break;

         	default:
            break;
      	}
   	}

   	va_end (args);
}

