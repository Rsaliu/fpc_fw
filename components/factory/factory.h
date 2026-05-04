#ifndef __FACTORY_H__
#define __FACTORY_H__
#include <common_headers.h>
#include <adc_reader.h>
#include <ads1115.h>
#include <setup_config.h>
#include <current_sensor.h>
#include <acs712_current_sensor.h>
#include <pump.h>
#include <pump_monitor.h>
#include <relay_driver.h>
#include <tank.h>
#include <level_sensor.h>
#include <tank_monitor.h>

error_type_t setup_internal_adc_reader(adc_reader_t** adc_reader, adc_channel_t channel);
error_type_t setup_acs712_with_internal_adc(acs712_sensor_t** acs712_sensor, adc_reader_t* reader,acs712_read_mode_t read_mode);
error_type_t setup_current_sensor_with_acs712(current_sensor_t** current_sensor, int id, acs712_sensor_t* acs712_sensor, current_sensor_read_mode_t read_mode);
error_type_t setup_i2c_master_bus_one(i2c_master_bus_handle_t* i2c_handle);
error_type_t setup_i2c_device_one(i2c_master_bus_handle_t i2c_handle ,i2c_master_dev_handle_t* dev_handle);
error_type_t setup_ads1115_oneshot_with_i2c(ads1115_t** ads1115,i2c_master_dev_handle_t dev_handle, ads1115_input_channel_t channel);
error_type_t setup_acs712_with_ads1115(acs712_sensor_t** acs712_sensor, ads1115_t* ads1115, acs712_read_mode_t read_mode);
error_type_t setup_current_sensor_from_config(current_sensor_setup_config_t* config, current_sensor_t** current_sensor);
error_type_t setup_pump_from_config(pump_setup_config_t* config, pump_t** pump);
error_type_t setup_pump_monitor_from_config(pump_monitor_setup_config_t* config, pump_monitor_t** pump_monitor, pump_t* pump, current_sensor_t* current_sensor, current_analytics_callback_t analytics_cb);
error_type_t setup_relay_from_config(relay_setup_config_t* config, relay_t** relay);
error_type_t setup_tank_from_config(tank_setup_config_t* config, tank_t** tank);
error_type_t setup_level_sensor_from_config(level_sensor_setup_config_t* config, level_sensor_t** level_sensor, void* medium_context);
error_type_t setup_tank_monitor_from_config(tank_monitor_setup_config_t* config, tank_monitor_t** tank_monitor, tank_t* tank, level_sensor_t* level_sensor, level_analytics_callback_t analytics_cb);
#endif