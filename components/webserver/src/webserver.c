#include <webserver.h>
#include <common_headers.h>
#include "esp_vfs_fat.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include "mdns.h"
#include "lwip/apps/netbiosns.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "nvs_flash.h"
#include <credential_store.h>

const char *TAG = "WEBSERVER";
const int RESERVED_SOCKETS = 3; // Number of reserved sockets for internal use
const int MAX_NUMBER_OF_URI_HANDLERS = 20;
static const char* ALLOWED_ORIGIN = "*";

typedef enum{
    WEBSERVER_NOT_INITIALIZED = 0, // Web server is not initialized
    WEBSERVER_INITIALIZED = 1, // Web server is initialized
    WEBSERVER_RUNNING = 2, // Web server is running
    WEBSERVER_STOPPED = 3, // Web server is stopped
}webserver_state_t;

struct webserver_t {
    const webserver_config_t* config; // Configuration for the web server
    httpd_handle_t server; // Handle for the HTTP server
    rest_server_context_t *rest_context; // Context for the REST server
    httpd_config_t httpd_config; // HTTP server configuration
    webserver_state_t status; // Current state of the web server
};
static esp_err_t cors_preflight_handler(httpd_req_t *req);
webserver_t* webserver_create(const webserver_config_t* config){
    if (config == NULL) {
        return NULL; // Handle null configuration
    }

    webserver_t *server = (webserver_t *)malloc(sizeof(webserver_t));
    if (server == NULL) {
        return NULL; // Handle memory allocation failure
    }

    server->config = config; // Copy the configuration
    server->status = WEBSERVER_NOT_INITIALIZED; // Initially not initialized
    return server;
}

static esp_err_t init_fs(const char* mount_point, const char* partition_label)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = mount_point,
        .partition_label = partition_label,
        .max_files = 10,
        .format_if_mount_failed = true
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    return ESP_OK;
}


static esp_err_t de_init_fs(const char* partition_label)
{
    if (partition_label == NULL) {
        ESP_LOGE(TAG, "Partition label is NULL");
        return ESP_ERR_INVALID_ARG; // Handle null partition label
    }
    esp_err_t ret = esp_vfs_spiffs_unregister(partition_label);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to unregister SPIFFS (%s)", esp_err_to_name(ret));
        return ret; // Return the error code
    }
    ESP_LOGI(TAG, "SPIFFS unregistered successfully");
    return ESP_OK;
}

static error_type_t initialise_mdns(const char * mdns_instance, const char * mdns_hostname,int port)
{
    esp_err_t err;
    err = mdns_init();
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "MDNS initialization failed: %s", esp_err_to_name(err));
        return err;
    }
    err = mdns_hostname_set(mdns_hostname);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "MDNS hostname set failed: %s", esp_err_to_name(err));
        return err;
    }
    err = mdns_instance_name_set(mdns_instance);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "MDNS instance name set failed: %s", esp_err_to_name(err));
        return err;
    }

    mdns_txt_item_t serviceTxtData[] = {
        {"board", "esp32"},
        {"path", "/"}
    };

    err = mdns_service_add("ESP32-WebServer", "_http", "_tcp", port, serviceTxtData,
                                     sizeof(serviceTxtData) / sizeof(serviceTxtData[0]));
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "MDNS service add failed: %s", esp_err_to_name(err));     
        return err;
    }
    return SYSTEM_OK;
}

static void cleanup_mdns_netbios(void)
{
    mdns_free();
    netbiosns_stop();
}
error_type_t webserver_init(webserver_t* server){
    if (server == NULL || server->config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null server or configuration
    }
    if (server->status != WEBSERVER_NOT_INITIALIZED) {
        return SYSTEM_INVALID_STATE; // Web server is already initialized
    }
    esp_err_t ret;
    ret = initialise_mdns(server->config->mdns_hostname, server->config->mdns_instance,server->config->port);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MDNS initialization failed: %s", esp_err_to_name(ret));
        return SYSTEM_OPERATION_FAILED; // Handle MDNS initialization failure
    }
    netbiosns_init();
    netbiosns_set_name(server->config->mdns_hostname);

    wifi_mode_t mode;
    ret = esp_wifi_get_mode(&mode);
    if (ret != ESP_OK) {
        cleanup_mdns_netbios();
        ESP_LOGE(TAG, "Failed to get WiFi mode: %s", esp_err_to_name(ret));
        return SYSTEM_OPERATION_FAILED; // Handle WiFi mode retrieval failure
    }
    if (mode != WIFI_MODE_AP) {
        cleanup_mdns_netbios();
        ESP_LOGE(TAG, "WiFi mode must be STA or APSTA for web server");
        return SYSTEM_INVALID_MODE; // Handle invalid WiFi mode
    }

    ret = init_fs(server->config->web_mount_point, server->config->web_partition_label);
    if (ret != ESP_OK) {
        cleanup_mdns_netbios();
        ESP_LOGE(TAG, "Failed to initialize file system: %s", esp_err_to_name(ret));
        return SYSTEM_OPERATION_FAILED; // Handle file system initialization failure
    }
    ret = credential_store_init();
    if (ret != ESP_OK) {
        cleanup_mdns_netbios();
        ESP_LOGE(TAG, "Failed to initialize file system: %s", esp_err_to_name(ret));
        return SYSTEM_OPERATION_FAILED; // Handle file system initialization failure
    }
    ESP_LOGI(TAG, "secure initialized successfully");
   server->rest_context = calloc(1, sizeof(rest_server_context_t));
    if(server->rest_context == NULL) {
        cleanup_mdns_netbios();
        ESP_LOGE(TAG, "Failed to allocate memory for REST server context");
        return SYSTEM_OPERATION_FAILED; // Handle memory allocation failure
    }
    memset(server->rest_context->base_path,0,sizeof(server->rest_context->base_path));
    strlcpy(server->rest_context->base_path, server->config->web_mount_point, sizeof(server->rest_context->base_path));
    memset(server->rest_context->scratch, 0, sizeof(server->rest_context->scratch));
    strlcpy(server->rest_context->config_file_path, server->config->config_file_path, sizeof(server->rest_context->config_file_path));
    ESP_LOGI(TAG, "REST server context initialized with base path: %s", server->rest_context->base_path);
    ESP_LOGI(TAG, "REST server context initialized with config file path: %s", server->rest_context->config_file_path);
    ESP_LOGI(TAG,"base bath set from webserver is: %s",server->rest_context->base_path);
    server->httpd_config = (httpd_config_t) HTTPD_DEFAULT_CONFIG();
    server->httpd_config.max_uri_handlers = MAX_NUMBER_OF_URI_HANDLERS;
    server->httpd_config.server_port = server->config->port; // Set the server port
    server->httpd_config.max_open_sockets = server->config->max_connections + RESERVED_SOCKETS; // Reserve 3 sockets for internal use
    server->httpd_config.uri_match_fn = httpd_uri_match_wildcard;
    server->status = WEBSERVER_INITIALIZED; // Set the status to initialized
    ESP_LOGI(TAG, "Web server initialized successfully");
    return SYSTEM_OK;
}
error_type_t webserver_start(webserver_t* server){
    if (server == NULL || server->config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null server or configuration
    }
    if (server->status != WEBSERVER_INITIALIZED) {
        return SYSTEM_INVALID_STATE; // Web server is not initialized
    }
    esp_err_t ret;
    // Start the web server logic here (e.g., start HTTP server, etc.)
    ret = httpd_start(&server->server, &server->httpd_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server: %s", esp_err_to_name(ret));
        return SYSTEM_OPERATION_FAILED; // Handle HTTP server start failure
    }
    //****** Leaving this code here, we may need it soon */
    //Register the OPTIONS handler for CORS support
    static httpd_uri_t options_uri = {
        .uri = "/*", // Match all URIs
        .method = HTTP_OPTIONS,
        .handler = cors_preflight_handler,
        .user_ctx = NULL
    };
    ret = httpd_register_uri_handler(server->server, &options_uri);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register OPTIONS handler: %s", esp_err_to_name(ret));
        httpd_stop(server->server); // Stop the server if registration fails
        return SYSTEM_OPERATION_FAILED; // Handle OPTIONS handler registration failure
    }
    ESP_LOGI(TAG, "OPTIONS handler registered successfully");
    server->status = WEBSERVER_RUNNING; // Set the status to running
    ESP_LOGI(TAG, "Web server started successfully");
    return SYSTEM_OK;
}
error_type_t webserver_stop(webserver_t* server){
    if (server == NULL || server->config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null server or configuration
    }
    if (server->status != WEBSERVER_RUNNING) {
        return SYSTEM_INVALID_STATE; // Web server is not running
    }
    esp_err_t ret;
    ret = httpd_stop(server->server);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop HTTP server: %s", esp_err_to_name(ret));
        return SYSTEM_OPERATION_FAILED; // Handle HTTP server stop failure
    }
    server->status = WEBSERVER_STOPPED; // Set the status to stopped
    ESP_LOGI(TAG, "Web server stopped successfully");
    return SYSTEM_OK;
}
error_type_t webserver_destroy(webserver_t** server){
    if (server == NULL || *server == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null server pointer
    }

    if ((*server)->status != WEBSERVER_NOT_INITIALIZED) {
        error_type_t err = webserver_deinit(*server);
        if (err != SYSTEM_OK) {
            ESP_LOGE(TAG, "Deinitialization failed: %d", err);
            return err; // Handle error in deinitialization
        }
    }

    // Free the allocated memory for the web server
    free(*server);
    *server = NULL; // Set pointer to NULL after freeing
    ESP_LOGI(TAG, "Web server destroyed successfully");
    return SYSTEM_OK;
}
error_type_t webserver_deinit(webserver_t* server){
    if (server == NULL || server->config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null server or configuration
    }
    if (server->status == WEBSERVER_NOT_INITIALIZED) {
        return SYSTEM_INVALID_STATE; // Web server is not initialized
    }
    if (server->status == WEBSERVER_RUNNING) {
        error_type_t err = webserver_stop(server);
        if (err != SYSTEM_OK) {
            ESP_LOGE(TAG, "Failed to stop web server: %d", err);
            return err; // Handle error in stopping the web server
        }
    }
    if(server->rest_context != NULL) {
        free(server->rest_context); // Free the REST server context
        server->rest_context = NULL; // Set pointer to NULL after freeing
    }
    if(credential_store_deinit() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinitialize NVS partition");
        return SYSTEM_OPERATION_FAILED; // Handle NVS deinitialization failure
    }
    ESP_LOGI(TAG, "secure deinitialized successfully");
    if(de_init_fs(server->config->web_partition_label) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinitialize file system");
        return SYSTEM_OPERATION_FAILED; // Handle file system deinitialization failure
    }
    mdns_free();
    netbiosns_stop();
    server->status = WEBSERVER_NOT_INITIALIZED; // Set the status to not initialized
    ESP_LOGI(TAG, "Web server deinitialized successfully");
    return SYSTEM_OK;
}

error_type_t webserver_add_route(webserver_t* server,httpd_uri_t* uri){
    if (server == NULL || uri == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null server or URI
    }
    if (server->status != WEBSERVER_RUNNING) {
        ESP_LOGE(TAG, "Web server is not running");
        return SYSTEM_INVALID_STATE; // Web server is not running
    }

    esp_err_t ret = httpd_register_uri_handler(server->server, uri);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add route: %s", esp_err_to_name(ret));
        return SYSTEM_OPERATION_FAILED; // Handle route addition failure
    }
    ESP_LOGI(TAG, "Route added successfully: %s", uri->uri);
    return SYSTEM_OK;
}

error_type_t webserver_remove_route(webserver_t* server,const char* uri,httpd_method_t method)
{
    if (server == NULL || uri == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null server or URI
    }
    if (server->status != WEBSERVER_RUNNING) {
        ESP_LOGE(TAG, "Web server is not running");
        return SYSTEM_INVALID_STATE; // Web server is not running
    }


    esp_err_t ret = httpd_unregister_uri_handler(server->server, uri, method);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to remove route: %s", esp_err_to_name(ret));
        return SYSTEM_OPERATION_FAILED; // Handle route removal failure
    }
    ESP_LOGI(TAG, "Route removed successfully: %s", uri);
    return SYSTEM_OK;
}

error_type_t webserver_get_scratch_buffer(webserver_t* server, char** buffer, size_t* size){
    if (server == NULL || server->rest_context == NULL || buffer == NULL || size == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null parameters
    }
    if (server->status != WEBSERVER_RUNNING) {
        return SYSTEM_INVALID_STATE; // Web server is not running
    }

    *buffer = server->rest_context->scratch;
    *size = SCRATCH_BUFSIZE; // Set the size of the scratch buffer
    ESP_LOGI(TAG, "Scratch buffer retrieved successfully");
    return SYSTEM_OK;
}

error_type_t webserver_get_context(webserver_t* server, rest_server_context_t** context_ptr){
    if (server == NULL || server->rest_context == NULL || context_ptr == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null parameters
    }
    if (server->status != WEBSERVER_RUNNING) {
        return SYSTEM_INVALID_STATE; // Web server is not running
    }

    *context_ptr = server->rest_context;
    ESP_LOGI(TAG, "context buffer retrieved successfully");
    return SYSTEM_OK;  
}

static esp_err_t cors_preflight_handler(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", ALLOWED_ORIGIN);
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type, Authorization");
    // If you want to allow cookies/auth:
    httpd_resp_set_hdr(req, "Access-Control-Allow-Credentials", "true");
    // No body for OPTIONS
    httpd_resp_set_status(req, "204 No Content");
    return httpd_resp_send(req, NULL, 0);
}



