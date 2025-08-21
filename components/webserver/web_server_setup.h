#ifndef __WEB_HANDLERS_SETUP_H__
#define __WEB_HANDLERS_SETUP_H__

#include <esp_http_server.h>
#include <esp_log.h>
#include "webserver_utils.h"
#include "reset_handler.h"
#include "register_handler.h"
#include "logout_handler.h"
#include "login_handler.h"
#include "config_handler.h"
#include "webserver.h"
#include "frontend_pages_handler.h"
#include <common.h>



error_type_t setup_web_handlers(webserver_t* webserver, rest_server_context_t * context_ptr) {
    if (webserver == NULL || context_ptr == NULL) {
        ESP_LOGE("WEB_HANDLERS_SETUP", "Web server or context is null");
        return SYSTEM_NULL_PARAMETER; // Handle null parameters
    }
    ESP_LOGI("WEB_HANDLERS_SETUP", "Setting up web handlers");
    error_type_t result;
    httpd_uri_t reset_uri = {
        .uri = "/reset",
        .method = HTTP_GET,
        .handler = reset_handler,
        .user_ctx = (void*)context_ptr
    };
    result = webserver_add_route(webserver, &reset_uri);
    if (result != SYSTEM_OK) {
        ESP_LOGE("WEB_HANDLERS_SETUP", "Failed to add reset handler route");
        return result; // Handle error in adding route
    }

    httpd_uri_t register_uri = {
        .uri = "/register",
        .method = HTTP_POST,
        .handler = register_handler,
        .user_ctx = (void*)context_ptr
    };
    result = webserver_add_route(webserver, &register_uri);
    if (result != SYSTEM_OK) {  
        ESP_LOGE("WEB_HANDLERS_SETUP", "Failed to add register handler route");
        return result; // Handle error in adding route
    }

    httpd_uri_t logout_uri = {
        .uri = "/logout",
        .method = HTTP_GET,
        .handler = logout_handler,
        .user_ctx = (void*)context_ptr
    };
    result = webserver_add_route(webserver, &logout_uri);
    if (result != SYSTEM_OK) {
        ESP_LOGE("WEB_HANDLERS_SETUP", "Failed to add logout_handler route");
        return result; // Handle error in adding route
    }


    httpd_uri_t login_uri = {
        .uri = "/login",
        .method = HTTP_POST,
        .handler = login_handler,
        .user_ctx = (void*)context_ptr
    };
    result = webserver_add_route(webserver, &login_uri);
    if (result != SYSTEM_OK) {
        ESP_LOGE("WEB_HANDLERS_SETUP", "Failed to add login_handler route");
        return result; // Handle error in adding route
    }

    httpd_uri_t get_config_uri = {
        .uri = "/config",
        .method = HTTP_GET,
        .handler = get_config_handler,
        .user_ctx = (void*)context_ptr
    };
    result = webserver_add_route(webserver, &get_config_uri);
    if (result != SYSTEM_OK) {
        ESP_LOGE("WEB_HANDLERS_SETUP", "Failed to add get_config_handler route");
        return result; // Handle error in adding route
    }

    httpd_uri_t set_config_uri = {
        .uri = "/config",
        .method = HTTP_POST,
        .handler = set_config_handler,
        .user_ctx = (void*)context_ptr
    };
    result = webserver_add_route(webserver, &set_config_uri);
    if (result != SYSTEM_OK) {
        ESP_LOGE("WEB_HANDLERS_SETUP", "Failed to add set_config_handler route");
        return result; // Handle error in adding route
    }

    // httpd_uri_t options_uri = {
    //     .uri = "/",
    //     .method = HTTP_OPTIONS,
    //     .handler = cors_preflight_handler,
    //     .user_ctx = (void*)context_ptr
    // };
    // result = webserver_add_route(webserver, &cors_preflight_handler);
    // if (result != SYSTEM_OK) {
    //     ESP_LOGE("WEB_HANDLERS_SETUP", "Failed to add cors_preflight_handler route");
    //     return result; // Handle error in adding route
    // }

    httpd_uri_t root_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_page_handler,
        .user_ctx = (void*)context_ptr
    };

    result = webserver_add_route(webserver, &root_uri);
    if (result != SYSTEM_OK) {
        ESP_LOGE("WEB_HANDLERS_SETUP", "Failed to add root_page_handler route");
        return result; // Handle error in adding route
    }
    httpd_uri_t login_page_uri = {
        .uri = "/login.html",
        .method = HTTP_GET,
        .handler = login_page_handler,
        .user_ctx = (void*)context_ptr,    
    };
    result = webserver_add_route(webserver, &login_page_uri);
    if (result != SYSTEM_OK) {
        ESP_LOGE("WEB_HANDLERS_SETUP", "Failed to add login_page_handler route");
        return result; // Handle error in adding route
    }
    httpd_uri_t register_page_uri = {
        .uri = "/register.html",
        .method = HTTP_GET,
        .handler = register_page_handler,
        .user_ctx = (void*)context_ptr,    
    };  
    result = webserver_add_route(webserver, &register_page_uri);
    if (result != SYSTEM_OK) {
        ESP_LOGE("WEB_HANDLERS_SETUP", "Failed to add register_page_handler route");
        return result; // Handle error in adding route
    }
    httpd_uri_t config_page_uri = {
        .uri = "/config.html",
        .method = HTTP_GET,
        .handler = config_page_handler,
        .user_ctx = (void*)context_ptr,
    };
    result = webserver_add_route(webserver, &config_page_uri);
    if (result != SYSTEM_OK) {
        ESP_LOGE("WEB_HANDLERS_SETUP", "Failed to add config_page_handler route");
        return result; // Handle error in adding route
    }
    ESP_LOGE("WEB_HANDLERS_SETUP", "set up all uri successfully");
    return SYSTEM_OK; // Return success if all handlers are set up successfully

}


#endif // __WEB_HANDLERS_SETUP_H__