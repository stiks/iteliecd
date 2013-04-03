#ifndef __SOAP_H__
#define __SOAP_H__

#include "../iteliec-client.h"

void parseResponse (xmlDocPtr xml_doc, xmlNodePtr xml_node_parent);
void err_soap(herror_t err);
int iteliec_soap_init (char *hash);

typedef int bool;
#define false 0
#define true  1

typedef struct auth_struct auth_type;

struct auth_struct {
    bool auth;
    char* token;
};

auth_type* parse_auth_response (SoapCtx *response);

#endif /* __SOAP_H__ */
