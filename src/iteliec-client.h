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
#include <statgrab.h>
#include <unistd.h>
#include <paths.h>

#include <libconfig.h>
#include <libcsoap/soap-client.h>

#include "core/soap.h"
#include "core/log.h"
#include "core/conf.h"


#define ITELIEC_FATAL -1
#define ITELIEC_OK 0
#define ITELIEC_ERR 1
#define ITELIEC_EXIT 2

typedef struct migation {
    char *version;
    char *apply_time;
} migration, *migrationPtr;

migrationPtr parseMigration (xmlDocPtr xml_doc, xmlNodePtr xml_node_parent, migrationPtr ret);
int soap_send_stats (SoapCtx *request);

const char *iteliec_config_file ();

#endif /* __ITELIEC_CLIENT_H__ */
