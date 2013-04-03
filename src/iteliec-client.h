#ifndef __ITELIEC_CLIENT_H__
#define __ITELIEC_CLIENT_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <getopt.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <stdarg.h>
#include <unistd.h>
#include <paths.h>
#include <errno.h>

#include <unistd.h>
#include <paths.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <statgrab.h>
#include <libconfig.h>
#include <libcsoap/soap-client.h>

#include "core/soap.h"
#include "core/log.h"
#include "core/conf.h"


#define ITELIEC_FATAL -1
#define ITELIEC_OK 0
#define ITELIEC_ERR 1
#define ITELIEC_EXIT 2

int soap_send_stats (SoapCtx *request);

const char *iteliec_config_file ();

#endif /* __ITELIEC_CLIENT_H__ */
