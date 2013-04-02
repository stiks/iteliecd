#include <statgrab.h>
#include <libcsoap/soap-client.h>

#include "log.h"
#include "process.h"

int *iteliec_get_process_info (SoapCtx *request) {
	sg_process_count *process_stat;

/*
    sg_init ();

    if (sg_drop_privileges () != 0) {
        iteliec_log (ITELIEC_ERR, "%s: Error. Failed to drop privileges", __func__);
    }
*/
    process_stat = sg_get_process_count ();

    soap_env_push_item (request->env, "urn:ProcessSoap", "process");

    soap_env_add_itemf (request->env, "xsd:integer", "running", "%d", process_stat->running);
    soap_env_add_itemf (request->env, "xsd:integer", "sleeping","%d", process_stat->sleeping);
    soap_env_add_itemf (request->env, "xsd:integer", "stopped", "%d", process_stat->stopped);
    soap_env_add_itemf (request->env, "xsd:integer", "zombie",  "%d", process_stat->zombie);
    soap_env_add_itemf (request->env, "xsd:integer", "total",   "%d", process_stat->total);

    soap_env_pop_item (request->env);

    return 0;
}
