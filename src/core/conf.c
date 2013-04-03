#include "../iteliec-client.h"

int iteliec_config_check (char *config) {
    config_t cfg;

    config_init (&cfg);

    /* Read the file. If there is an error, report it and exit. */
    if (!config_read_file (&cfg, config)) {
        //printf ("\n%s:%d - %s", config_error_file (&cfg), config_error_line (&cfg), config_error_text (&cfg));
        config_destroy (&cfg);
        
        return ITELIEC_FATAL;
    }
    
    return ITELIEC_OK;
}
