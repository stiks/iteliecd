#ifndef __ITELIEC_CLIENT_H__
#define __ITELIEC_CLIENT_H__

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
