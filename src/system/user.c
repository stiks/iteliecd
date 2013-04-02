#include "../iteliec-client.h"

int *iteliec_get_user_info (SoapCtx *request) {
	sg_user_stats *users; 

    users = sg_get_user_stats ();

    soap_env_push_item (request->env, "urn:UserSoap", "user");

    soap_env_add_itemf (request->env, "xsd:integer", "num_entries", "%d", users->num_entries);
    soap_env_add_item  (request->env, "xsd:string", "name_list", users->name_list);

    soap_env_pop_item (request->env);

    return 0;
}
