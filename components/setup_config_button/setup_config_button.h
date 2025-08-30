#ifndef __SETUP_CONFIG_BUTTON__
#define __SETUP_CONFIG_BUTTON__
#include <common_headers.h>
#include <stdbool.h>

 
typedef struct 
{

    int button_pin_number;
    bool setup_config_button_mode;

}setup_config_button_config_t;

void setup_config_button_task(void* Pvparameter);


#endif