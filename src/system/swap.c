#include "../iteliec-client.h"

int *iteliec_get_swap_info (SoapCtx *request) {
	sg_swap_stats *swap_stats;

    swap_stats = sg_get_swap_stats ();

    soap_env_push_item (request->env, "urn:SwapSoap", "swap");

    soap_env_add_itemf (request->env, "xsd:int", "used",  "%lld", swap_stats->used);
    soap_env_add_itemf (request->env, "xsd:int", "total", "%lld", swap_stats->total);

    soap_env_pop_item (request->env);

    return 0;
}
