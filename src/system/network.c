#include <statgrab.h>
#include <libcsoap/soap-client.h>

#include "log.h"
#include "network.h"

int *iteliec_get_network_info (SoapCtx *request) {
	sg_network_io_stats *network_stats;
	int x, num_network_stats;

/*
    sg_init ();

    if (sg_drop_privileges () != 0) {
        iteliec_log (ITELIEC_ERR, "%s: Error. Failed to drop privileges", __func__);
    }
*/

	/* We are not interested in the amount of traffic ever transmitted. */
	network_stats = sg_get_network_io_stats_diff (&num_network_stats);
	if (network_stats == NULL){
		iteliec_log (ITELIEC_ERR, "%s: Error. Failed to get network stats", __func__);

		soap_env_push_item (request->env, "urn:NetworkSoapArray", "network");
		soap_env_pop_item  (request->env);

		return;
	}

	/* wait for a second to get data difference */
	sleep (1);

	network_stats = sg_get_network_io_stats_diff (&num_network_stats);

	soap_env_push_item (request->env, "urn:NetworkSoapArray", "network");
    for(x = 0; x < num_network_stats; x++){ 
		soap_env_push_item (request->env, "urn:NetworkSoap", "network");

		soap_env_add_item  (request->env, "xsd:string", "interface_name",   network_stats->interface_name);
		soap_env_add_itemf (request->env, "xsd:integer","rx",  "%lld",		network_stats->rx);
		soap_env_add_itemf (request->env, "xsd:integer","tx", "%lld",		network_stats->tx);

		soap_env_pop_item (request->env);

        network_stats++;
    }
	soap_env_pop_item (request->env);
}


int *iteliec_get_iface_info (SoapCtx *request) {
	sg_network_iface_stats *network_iface_stats;
	int x, iface_count;

/*
    sg_init ();

    if (sg_drop_privileges () != 0) {
        iteliec_log (ITELIEC_ERR, "%s: Error. Failed to drop privileges", __func__);
    }
*/

	network_iface_stats = sg_get_network_iface_stats (&iface_count);

	soap_env_push_item (request->env, "urn:IfaceSoapArray", "iface");
    for(x = 0; x < iface_count; x++){ 
		soap_env_push_item (request->env, "urn:IfaceSoap", "iface");

		soap_env_add_item  (request->env, "xsd:string", "interface_name",   network_iface_stats->interface_name);
		soap_env_add_itemf (request->env, "xsd:integer","speed", "%d",		network_iface_stats->speed);
		soap_env_add_itemf (request->env, "xsd:integer","up", 	 "%d",		network_iface_stats->up);

		soap_env_pop_item (request->env);

        network_iface_stats++;
    }
	soap_env_pop_item (request->env);

	return 0;
}
