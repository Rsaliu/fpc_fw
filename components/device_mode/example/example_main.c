#include "device_mode.h"
#include "example.h"
#include "dummy_webserver.h"

device_mode_config_t config = {
    .other_task = task_handler,
    .button_pin_number = 4,
    .webserver = create_dummy_webserver_task
};

void app_main(void)
{
    printf("Hello world!\n");

    device_mode_init(&config);
    device_mode_event(&config);
}