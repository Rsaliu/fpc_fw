#include <logout_handler.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <session.h>
#include <webserver_utils.h>
#include <auth.h>

const char *LOGOUT_HANDLER_TAG = "LOGOUT_HANDLER";

esp_err_t logout_handler(httpd_req_t *req) {
    if (req == NULL) {
        //httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "null request");
        return ESP_ERR_NO_MEM; // Handle null request
    }
    inject_cors_options(req); // Set CORS headers for the request
    session_t *session = NULL;
    esp_err_t err= auth_handler(req,&session);
    if (err != ESP_OK) {
        return err; // Handle authentication failure
    } 
    // although this if statement is not necessary, it is a good practice to check if session is NULL
    if (session == NULL) {
        httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Unauthorized access");
        return ESP_FAIL; // Handle unauthorized access
    }
    // Remove the session
    remove_session(session);
    
    // Clear the session cookie
    httpd_resp_set_hdr(req, "Set-Cookie", "SID=; Max-Age=0; Path=/; HttpOnly");
    
    // Send a response indicating successful logout
    httpd_resp_set_status(req, HTTPD_200);
    err = httpd_resp_sendstr(req, "Logout successful");
    
    if (err != ESP_OK) {
        ESP_LOGE(LOGOUT_HANDLER_TAG, "Failed to send response: %s", esp_err_to_name(err));
        return err; // Handle response sending failure
    }
    ESP_LOGI(LOGOUT_HANDLER_TAG, "User %s logged out successfully", session->username);
    return ESP_OK; // Return success
}