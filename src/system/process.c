#include "../iteliec-client.h"

int *iteliec_get_process_info (SoapCtx *request) {
	sg_process_count *process_stat;

    process_stat = sg_get_process_count ();

    soap_env_push_item (request->env, "urn:ProcessSoap", "process");

    soap_env_add_itemf (request->env, "xsd:integer", "running", "%d", process_stat->running);
    soap_env_add_itemf (request->env, "xsd:integer", "sleeping","%d", process_stat->sleeping);
    soap_env_add_itemf (request->env, "xsd:integer", "stopped", "%d", process_stat->stopped);
    soap_env_add_itemf (request->env, "xsd:integer", "zombie",  "%d", process_stat->zombie);
    soap_env_add_itemf (request->env, "xsd:integer", "total",   "%d", process_stat->total);

    soap_env_pop_item (request->env);

    return ITELIEC_OK;
}

int *iteliec_get_process_list (SoapCtx *request) {
    sg_process_stats *ps;
    int ps_size;
    int x;
    char *state = NULL;

    /* get process list */
    ps = sg_get_process_stats (&ps_size);    

    if (ps == NULL) {
        iteliec_log (ITELIEC_ERR, "Failed to get process snapshot");
        
        return;
    }

    soap_env_push_item (request->env, "urn:ProcessListSoapArray", "processlist");
    for (x = 0; x < ps_size; x++) {
        switch (ps->state) {
        case SG_PROCESS_STATE_RUNNING:
            state = "RUNNING";
            break;
        case SG_PROCESS_STATE_SLEEPING:
            state = "SLEEPING";
            break;
        case SG_PROCESS_STATE_STOPPED:
            state = "STOPPED";
            break;
        case SG_PROCESS_STATE_ZOMBIE:
            state = "ZOMBIE";
            break;
        case SG_PROCESS_STATE_UNKNOWN:
        default:
            state = "UNKNOWN";
            break;
        }

        soap_env_push_item (request->env, "urn:ProcessListSoap", "processlist");

        soap_env_add_itemf (request->env, "xsd:integer", "pid",     "%d", (int)ps->pid);
        soap_env_add_itemf (request->env, "xsd:integer", "parent",  "%d", (int)ps->parent);
        soap_env_add_itemf (request->env, "xsd:integer", "pgid",    "%d", (int)ps->pgid);
        soap_env_add_itemf (request->env, "xsd:integer", "uid",     "%d", (int)ps->uid);
        soap_env_add_itemf (request->env, "xsd:integer", "euid",    "%d", (int)ps->euid);
        soap_env_add_itemf (request->env, "xsd:integer", "gid",     "%d", (int)ps->gid);
        soap_env_add_itemf (request->env, "xsd:integer", "egid",    "%d", (int)ps->egid);
        soap_env_add_itemf (request->env, "xsd:integer", "nice",    "%d", (int)ps->nice);
        
        soap_env_add_itemf (request->env, "xsd:integer", "proc_size",    "%d", (int)ps->proc_size);
        soap_env_add_itemf (request->env, "xsd:integer", "proc_resident","%d", (int)ps->proc_resident);
        soap_env_add_itemf (request->env, "xsd:float",   "cpu_percent",  "%F", ps->cpu_percent);
        
        soap_env_add_item  (request->env, "xsd:string", "state", state);
        soap_env_add_item  (request->env, "xsd:string", "process_name", ps->process_name);
        soap_env_add_item  (request->env, "xsd:string", "proctitle", ps->proctitle);

        soap_env_pop_item (request->env);

        ps++;
    }
    soap_env_pop_item (request->env);

    return ITELIEC_OK;
}
