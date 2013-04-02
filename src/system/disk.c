#include <statgrab.h>
#include <libcsoap/soap-client.h>

#include "log.h"
#include "disk.h"

int *iteliec_get_disk_info (SoapCtx *request) {
	sg_disk_io_stats *diskio_stats;
	int num_diskio_stats, x;

	diskio_stats = sg_get_disk_io_stats_diff (&num_diskio_stats);
	if (diskio_stats == NULL){
		iteliec_log (ITELIEC_ERR, "%s: Error. Failed to get disk stats", __func__);
	}

	sleep (1);

	diskio_stats = sg_get_disk_io_stats_diff (&num_diskio_stats);

	soap_env_push_item (request->env, "urn:DiskSoapArray", "disk");
	for(x = 0; x < num_diskio_stats; x++){	
		soap_env_push_item (request->env, "urn:DiskSoap", "disk");

		soap_env_add_item  (request->env, "xsd:string", "disk_name",   diskio_stats->disk_name);
		soap_env_add_itemf (request->env, "xsd:float",  "read_bytes",  "%lld",diskio_stats->read_bytes);
		soap_env_add_itemf (request->env, "xsd:float",  "write_bytes", "%lld",diskio_stats->write_bytes);

		soap_env_pop_item (request->env);

        diskio_stats++;
    }
	soap_env_pop_item (request->env);

	return 0;
}

int *iteliec_get_fs_info (SoapCtx *request) {
	sg_fs_stats *fs_stats;
	int fs_entries, x;

	fs_stats = sg_get_fs_stats (&fs_entries);

	soap_env_push_item (request->env, "urn:FsSoapArray", "fs");
    for(x = 0; x < fs_entries; x++){ 
		soap_env_push_item (request->env, "urn:FsSoap", "fs");

		soap_env_add_item  (request->env, "xsd:string",  "device_name", fs_stats->device_name);
		soap_env_add_item  (request->env, "xsd:string",  "fs_type", 	fs_stats->fs_type);
		soap_env_add_item  (request->env, "xsd:string",  "mnt_point", 	fs_stats->mnt_point);
		soap_env_add_itemf (request->env, "xsd:integer", "size", "%lld",fs_stats->size);
		soap_env_add_itemf (request->env, "xsd:integer", "used", "%lld",fs_stats->used);

		soap_env_pop_item (request->env);


        fs_stats++;
    }
	soap_env_pop_item (request->env);

	return 0;
}
