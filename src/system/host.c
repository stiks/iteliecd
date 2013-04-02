#include <statgrab.h>
#include <libcsoap/soap-client.h>

#include "log.h"
#include "host.h"

int *iteliec_get_host_info (SoapCtx *request) {
	sg_host_info *general_stats;

/*
    sg_init ();

    if (sg_drop_privileges () != 0) {
        iteliec_log (ITELIEC_ERR, "%s: Error. Failed to drop privileges", __func__);
    }
*/
	general_stats = sg_get_host_info ();

    soap_env_push_item (request->env, "urn:HostSoap", "host");

    soap_env_add_item  (request->env, "xsd:string", "os_name", general_stats->os_name);
    soap_env_add_item  (request->env, "xsd:string", "platform", general_stats->platform);
    soap_env_add_item  (request->env, "xsd:string", "hostname", general_stats->hostname);
    soap_env_add_itemf (request->env, "xsd:integer", "uptime", "%lu", general_stats->uptime);

    soap_env_pop_item (request->env);

    return 0;
}
