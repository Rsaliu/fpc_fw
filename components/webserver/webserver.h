#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__
#include "nvs_flash.h"
#include "esp_vfs.h"
#include <common_headers.h>
#define LOGIN_MAX_USERNAME_LENGTH CONFIG_MAX_USERNAME_LENGTH
#define LOGIN_MAX_PASSWORD_LENGTH CONFIG_MAX_PASSWORD_LENGTH
#include "esp_http_server.h"



typedef struct{
    int port; // Port number for the web server
    char* document_root; // Root directory for serving files
    int max_connections; // Maximum number of concurrent connections
    const char * mdns_instance; // MDNS instance name
    const char * mdns_hostname; // MDNS hostname
    const char * base_path; // Base path for the web server
    const char *web_mount_point; // Web mount point for serving files
    const char *web_partition_label; // Partition label for the web server
}webserver_config_t;

#define SCRATCH_BUFSIZE (10240)
//const int SCRATCH_BUFSIZE = 10240; // Size of the scratch buffer for HTTP server

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

typedef struct webserver_t webserver_t;

webserver_t* webserver_create(const webserver_config_t* config);
error_type_t webserver_init(webserver_t* server);
error_type_t webserver_start(webserver_t* server);
error_type_t webserver_stop(webserver_t* server);
error_type_t webserver_destroy(webserver_t** server);
error_type_t webserver_deinit(webserver_t* server);
error_type_t webserver_add_route(webserver_t* server,httpd_uri_t* uri);
error_type_t webserver_remove_route(webserver_t* server,const char* uri,httpd_method_t method);
error_type_t webserver_get_scratch_buffer(webserver_t* server, char** buffer, size_t* size);

#endif // __WEBSERVER_H__