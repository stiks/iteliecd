#include <stdio.h>
#include <stdarg.h>

#include <string.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include <libcsoap/soap-client.h>
#include <libcsoap/soap-router.h>

#include "conf.h"
#include "soap.h"

#include "../iteliec-client.h"

void parseResponse (xmlDocPtr xml_doc, xmlNodePtr xml_node_parent) {
    xmlNodePtr xml_node;
    xmlChar *buf;

/*
    if ((!xmlStrcmp(xml_node_parent->name, (const xmlChar *)"return"))) {
        buf = (char*)xmlNodeListGetString(xml_doc, xml_node_parent->xmlChildrenNode, 1);
        printf ("Return %s Buf: %s\n", xml_node_parent->name, buf);

        xmlFree(buf);
        printf ("Type %s\n", xmlGetProp(xml_node_parent, "arrayType"));
    }
*/

/*
    if ((!xmlStrcmp(xml_node_parent->name, (const xmlChar *)"item"))) {
        if (xmlStrstr(xmlGetProp(xml_node_parent, "type"), "Migration")) {
            migrationPtr ret = NULL;
            ret = (migrationPtr) malloc(sizeof(migration));

            ret = parseMigration (xml_doc, xml_node_parent, ret);
            printf ("Version: %s | Time: %s\n", ret->version, ret->apply_time);

            return;
        }
    }
*/

    xml_node = xml_node_parent->xmlChildrenNode;
    while (xml_node != NULL) {
        buf = (char*)xmlNodeListGetString(xml_doc, xml_node->xmlChildrenNode, 1);
        printf ("%s : %s\n", xml_node->name, buf);
        xmlFree(buf); buf = NULL;

        parseResponse (xml_doc, xml_node);

        xml_node = xml_node->next;
    }

    return;
}

void err_soap(herror_t err) {
    if(err==H_OK) {
        return;
    }

    printf ("%s():%s [%d]\n",herror_func(err), herror_message(err), herror_code(err));
    
    herror_release (err);
}

int iteliec_soap_init (const char *hash) {
    herror_t err;
    SoapCtx *request;
    SoapCtx *response;

    char url[1024];
    //char hash[128];
    int error;

    xmlNodePtr function, node;

    /* ----------------------------------- */
    /*    Initialize SOAP Client           */
    /* ----------------------------------- */
    err = soap_client_init_args(0, NULL);
    if (err != H_OK)  {
        err_soap (err);
        
        return 1;
    }

    /* ----------------------------------- */
    /*    Create "request" envelope        */
    /* ----------------------------------- */
    err = soap_ctx_new_with_method ("", "setSystem", &request);
    if (err != H_OK) {
        err_soap(err);
        soap_client_destroy();

        return 1;
    }

    /* ----------------------------------- */
    /*    Create header element            */
    /* ----------------------------------- */

    //xmlDocFormatDump(stdout, request->env->header->doc,1);

    /* ----------------------------------- */
    /*    Start collecting data            */
    /* ----------------------------------- */
    iteliec_get_host_info (&request->env);
    iteliec_get_user_info (&request->env);
    iteliec_get_load_info (&request->env);
    iteliec_get_process_info (&request->env);
    iteliec_get_memory_info (&request->env);
    iteliec_get_swap_info (&request->env);
    iteliec_get_cpu_info (&request->env);

    iteliec_get_fs_info (&request->env);
    iteliec_get_disk_info (&request->env);

    iteliec_get_iface_info (&request->env);
    iteliec_get_network_info (&request->env);

    /* ----------------------------------- */
    /*    Trade for "response" envelope    */
    /* ----------------------------------- */
    sprintf (url, "http://api.iteliec.com/system/update/ws/?token=%s", hash);
    err = soap_client_invoke (request, &response, url, "");
    if (err != H_OK)  {
        err_soap(err);
        soap_ctx_free(request);
        soap_client_destroy();
        
        return ITELIEC_ERR;
    }

    function = soap_env_get_method (response->env);
    node = soap_xml_get_children (function);

    parseResponse (node->doc, node);

    /* ----------------------------------- */
    /*    Show "response" envelope          */
    /* ----------------------------------- */
/*
    xmlDocFormatDump(stdout, response->env->root->doc,1);
*/

    /* ----------------------------------- */
    /*    Destroy SOAP Client              */
    /* ----------------------------------- */
    soap_ctx_free(request);
    soap_ctx_free(response);
    soap_client_destroy();

    return 0;
}

int iteliec_soap_register (char *username, char *password) {
    herror_t err;
    SoapCtx *request;
    SoapCtx *response;

    config_t cfg;
    config_setting_t *setting, *root;
    const char *cfile;

    cfile = iteliec_config_file ();
    
    config_init (&cfg);

    /* Read the file. If there is an error, report it and exit. */
    if (!config_read_file (&cfg, cfile)) {
        printf ("Please ensure configuration file %s exists and is valid", cfile);
        printf ("\n%s:%d - %s", config_error_file (&cfg), config_error_line (&cfg), config_error_text (&cfg));
        
        config_destroy (&cfg);
        
        return;
    }

    root = config_root_setting (&cfg);

    /* Init */
    err = soap_client_init_args (0, NULL);
    if (err != H_OK)  {
        err_soap (err);
        
        return ITELIEC_ERR;
    }

    err = soap_ctx_new_with_method ("", "sendAuth", &request);
    if (err != H_OK) {
        err_soap (err);
        soap_client_destroy ();

        return ITELIEC_ERR;
    }

    /* Add login details */
    soap_env_add_item (request->env, "xsd:string", "username", username);
    soap_env_add_item (request->env, "xsd:string", "password", password);

    /* Trade for response */
    err = soap_client_invoke (request, &response, "http://api.iteliec.com/register/auth/ws/", "");
    if (err != H_OK)  {
        err_soap (err);
        soap_ctx_free (request);
        soap_client_destroy ();
        
        return ITELIEC_ERR;
    }

    /* Parse response */
    auth_type* ret = parse_auth_response (response);
    if (ret->auth == true) {
        printf ("Success\n");
        
        setting = config_setting_get_member (root, "api");
        if(!setting) {
            setting = config_setting_add (root, "api", CONFIG_TYPE_GROUP);        
        }

        setting = config_setting_add (setting, "hash", CONFIG_TYPE_STRING);
        config_setting_set_string (setting, ret->token);

        /* Write out the updated configuration. */
        if(! config_write_file(&cfg, cfile)) {
            fprintf (stderr, "Error while writing file.\n");
            config_destroy (&cfg);
            
            return (ITELIEC_OK);
        }

        printf("\nConfiguration file updated. Server is ready to run.\n");

    } else {
        printf("Auth failed\n");
    }

    /* Destroy */
    soap_ctx_free (request);
    soap_ctx_free (response);
    
    soap_client_destroy ();
    config_destroy (&cfg);

    return ITELIEC_OK;
}


auth_type* parse_auth_response (SoapCtx *response) {
    xmlXPathObjectPtr xpathObj = NULL;
    xmlNode* curNode = NULL;
    int i = 0;
    auth_type* ret;

    char* auth = NULL;
    char* token = NULL;

    xpathObj = soap_xpath_eval (response->env->root->doc, "//return");
    if(xpathObj == NULL) {
        printf("Unable to parse response\n");

        return NULL;
    }

    /* Parse response */
    if (xpathObj->nodesetval && xpathObj->nodesetval->nodeNr > 0) {
        for (i = 0; i < xpathObj->nodesetval->nodeNr; i++) {
            curNode = xpathObj->nodesetval->nodeTab[i]->xmlChildrenNode;

            while (curNode) {
                if (xmlStrEqual(curNode->name, (const xmlChar *)"auth")) {
                    auth = (char *) xmlNodeGetContent(curNode);
                } else if (xmlStrEqual(curNode->name, (const xmlChar *)"token")) {
                    token = (char *) xmlNodeGetContent(curNode);
                }

                curNode = curNode->next;
            }

            if (auth) {
                if (strcmp (auth, "true") == 0) {
                    ret->auth = true;
                } else {
                    ret->auth = false;
                }
            }

            if (token) {
                ret->token = token;
            }

            free((void*)auth);
            free((void*)token);
        }
    }

    xmlXPathFreeObject (xpathObj);

    return ret;
}
