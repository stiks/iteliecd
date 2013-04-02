#include "../iteliec-client.h"

int *iteliec_get_cpu_info (SoapCtx *request) {
	sg_cpu_percents *cpu_percent = sg_get_cpu_percents ();

    soap_env_push_item (request->env, "urn:CpuSoap", "cpu");

    soap_env_add_itemf (request->env, "xsd:float", "user",  "%f", cpu_percent->user);
    soap_env_add_itemf (request->env, "xsd:float", "nice",  "%f", cpu_percent->nice);
    soap_env_add_itemf (request->env, "xsd:float", "kernel","%f", cpu_percent->kernel);
    soap_env_add_itemf (request->env, "xsd:float", "iowait","%f", cpu_percent->iowait);
    soap_env_add_itemf (request->env, "xsd:float", "swap",  "%f", cpu_percent->swap);
    soap_env_add_itemf (request->env, "xsd:float", "idle",  "%f", cpu_percent->idle);

    soap_env_pop_item (request->env);

    return 0;
}
