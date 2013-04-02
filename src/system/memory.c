#include <statgrab.h>
#include <libcsoap/soap-client.h>

#include "log.h"
#include "memory.h"

int *iteliec_get_memory_info (SoapCtx *request) {
    sg_mem_stats *mem_stats;
/*
    sg_init ();

    if (sg_drop_privileges () != 0) {
        iteliec_log (ITELIEC_ERR, "%s: Error. Failed to drop privileges", __func__);
    }
*/
    mem_stats = sg_get_mem_stats ();

    soap_env_push_item (request->env, "urn:MemorySoap", "memory");

    soap_env_add_itemf (request->env, "xsd:integer", "used",  "%u", mem_stats->used);
    soap_env_add_itemf (request->env, "xsd:integer", "total", "%u", mem_stats->total);

    soap_env_pop_item (request->env);

    return 0;
}
