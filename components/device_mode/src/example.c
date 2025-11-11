#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "protocol.h"
#include "rs485_context.h"
#include "rs485.h"
#include "level_sensor.h"
#include "tank.h"
#include "tank_monitor_task.h"
#include "tank_monitor.h"
#include "example.h"
#include "esp_log.h"


tank_t* tank1 = NULL;
tank_t* tank2 = NULL;

level_sensor_t* sensor1 = NULL;
level_sensor_t* sensor2 = NULL;
rs485_t* rs485_ttl;

tank_monitor_t* monitor1 = NULL;
tank_monitor_t* monitor2 = NULL;

tank_monitor_t* monitors[2];
tank_monitor_task_config_t tank_monitor_task_config;
        


void example(void)
{
    printf("Hello world!\n");

    tank_config_t tank_config1 = {.id = 1, .capacity_in_liters= 36, 
        .shape = TANK_SHAPE_RECTANGLE, .height_in_cm = 28,
        .full_level_in_mm =70, .low_level_in_mm = 50 }; 

    tank_config_t tank_config2 = {.id = 2, .capacity_in_liters= 40, 
        .shape = TANK_SHAPE_RECTANGLE, .height_in_cm = 22,
        .full_level_in_mm =50, .low_level_in_mm = 30 }; 

    tank1 = tank_create(tank_config1);
    tank_init(tank1);
    tank2 = tank_create(tank_config2);
    tank_init(tank2);
    
    if (!tank1 || !tank2) {
        ESP_LOGE("TANK", "Failed to create tanks");
    }

    protocol_callback_t protocol = protocol_gl_a01_read_level;
    send_receive_t send_receive =  rs485_context_send_receive;
    protocol_interpreter_t interpret = protocol_gl_a01_interpreter;
    rs485_config_t rs485_config = {2,17,16,5,9600};
    rs485_ttl = rs485_create(&rs485_config);
    rs485_init(rs485_ttl);

    level_sensor_config_t sensor_config1 = {.id = 1,.sensor_addr = 0x01, .protocol = protocol, 
            .medium_context = rs485_ttl, .send_recive = send_receive, .interpreter= interpret, .interface = LEVEL_SENSOR_INTERFACE_RS485, 
            .level_sensor_protocol = GL_A01_PROTOCOL};
        
    level_sensor_config_t sensor_config2 = {.id = 2,.sensor_addr = 0x01, .protocol = protocol, 
            .medium_context = rs485_ttl, .send_recive = send_receive, .interpreter= interpret,.interface = LEVEL_SENSOR_INTERFACE_RS485, 
            .level_sensor_protocol = GL_A01_PROTOCOL}; 
    sensor1 = level_sensor_create(sensor_config1);
    level_sensor_init(sensor1);
    sensor2 = level_sensor_create(sensor_config2);
     level_sensor_init(sensor2);
     
      
    if (!sensor1 || !sensor2) {
        ESP_LOGE("LEVEL_SENSOR", "level sensor failed to create");
    }

    tank_monitor_config_t config1 = {.id=1, .tank = tank1, .sensor = sensor1};
    tank_monitor_config_t config2 = {.id=2, .tank = tank2, .sensor = sensor2};

    monitor1 = tank_monitor_create(config1);
    tank_monitor_init(monitor1);
    monitor2 = tank_monitor_create(config2);
    tank_monitor_init(monitor2);
        if (!monitor1 || !monitor2) {
        ESP_LOGE("TANK_MONITOR", "Failed to create monitors");
    }
    
    monitors[0] = monitor1;
    monitors[1] = monitor2;

    tank_monitor_task_config.tank_monitor = monitors;
    tank_monitor_task_config.tank_monitor_count = 2;
     
    xTaskCreate(&tank_monitor_task,"tank_monitor_task",4096, &tank_monitor_task_config,1,NULL);


}
