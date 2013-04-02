#include "../iteliec-client.h"

int *iteliec_get_load_info (SoapCtx *request) {
	sg_load_stats *load_stat;

	load_stat = sg_get_load_stats ();

    soap_env_push_item (request->env, "urn:LoadSoap", "load");

    soap_env_add_itemf (request->env, "xsd:double", "min1",  "%lf", load_stat->min1);
    soap_env_add_itemf (request->env, "xsd:double", "min5",  "%lf", load_stat->min5);
    soap_env_add_itemf (request->env, "xsd:double", "min15", "%lf", load_stat->min15);
    
    soap_env_pop_item (request->env);

    return 0;
}
