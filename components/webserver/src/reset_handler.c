#include <reset_handler.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <session.h>
#include <credential_store.h>
#include <webserver_utils.h>
#include <webserver.h>
#include <auth.h>
esp_err_t reset_handler(httpd_req_t *req) {
    if (req == NULL) {
        //httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "null request");
        return ESP_ERR_NO_MEM; // Handle null request
    }
    inject_cors_options(req); // Set CORS headers for the request
    rest_server_context_t * context = (rest_server_context_t *)req->user_ctx;
    char *buf = (char *)(context->scratch);
    
    session_t *session = NULL;
    esp_err_t err;
    // removing the need for auth for reset for now because, there is no reset password feature at the moment

    // esp_err_t err = auth_handler(req,&session);
    // if (err != ESP_OK) {
    //     return err; // Handle authentication failure
    // }
    ESP_LOGI("RESET_HANDLER", "Handling reset request for URI: %s", req->uri);
    
    // clear sessions
    clear_sessions();
    // clear user credentials
    err = clear_credential_store();
    if (err != ESP_OK) {
        ESP_LOGE("RESET_HANDLER", "Failed to clear credential store: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
             create_response_json("Failed to clear credential store",buf,SCRATCH_BUFSIZE)
            );
        return err; // Handle credential store clearing failure
    }
    ESP_LOGI("RESET_HANDLER", "Credential store cleared successfully");
    // clear configuration file
    char *base_path = context->base_path;
    char *config_file_path = context->config_file_path;
    if (base_path == NULL || config_file_path == NULL) {
        ESP_LOGE("RESET_HANDLER", "Base path or config file path is null");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
            create_response_json("Base path or config file path is null",buf,SCRATCH_BUFSIZE)
            );
        return ESP_ERR_NO_MEM; // Handle null base path or config file path 
    }
    char full_config_path[50];
    snprintf(full_config_path, 50, "%s/%s", base_path, config_file_path);
    ESP_LOGI("RESET_HANDLER", "Config file path: %s", full_config_path);
    remove(full_config_path);
    ESP_LOGI("RESET_HANDLER", "Configuration file cleared successfully");
    httpd_resp_set_status(req, HTTPD_200);
    return httpd_resp_sendstr(req,
        create_response_json("Reset successful",buf,SCRATCH_BUFSIZE)
        );
}