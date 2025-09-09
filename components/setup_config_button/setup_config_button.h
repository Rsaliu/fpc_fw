#ifndef __SETUP_CONFIG_BUTTON__
#define __SETUP_CONFIG_BUTTON__
#include <common_headers.h>
#include <stdbool.h>
#include <stdint.h>

typedef error_type_t (*setup_config_button_t)();
 
typedef struct 
{

    uint8_t button_pin_number;
    setup_config_button_t config_button;


}setup_config_button_config_t;
void setup_config_button_task(void* Pvparameter);
void setup_config_button_init(setup_config_button_config_t* config);


#endif