#ifndef __WIFI_HOTSPOT_H__
#define __WIFI_HOTSPOT_H__
#include <stdint.h>
#include <common_headers.h>

typedef struct wifi_hotspot_t wifi_hotspot_t;

typedef enum {
    WIFI_HOTSPOT_AUTH_OPEN,      
    WIFI_HOTSPOT_AUTH_WPA2_PSK,  
    WIFI_HOTSPOT_AUTH_WPA3_PSK, 
} wifi_hotspot_auth_mode_t;

typedef struct{
    char ssid[32];
    char password[64];
    uint8_t channel;//default = 1, range 1-13
    uint8_t max_connections;//max is 10
    wifi_hotspot_auth_mode_t auth_mode;
}wifi_hotspot_config_t;



wifi_hotspot_t* wifi_hotspot_create(wifi_hotspot_config_t config);
error_type_t wifi_hotspot_init(wifi_hotspot_t *wifi_hotspot);
error_type_t wifi_hotspot_deinit(wifi_hotspot_t *wifi_hotspot);
error_type_t wifi_hotspot_destroy(wifi_hotspot_t **wifi_hotspot);
error_type_t wifi_hotspot_on(wifi_hotspot_t *wifi_hotspot);
error_type_t wifi_hotspot_off(wifi_hotspot_t *wifi_hotspot);

#endif