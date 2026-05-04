#ifndef __GLOBALS_H__
#define __GLOBALS_H__
#include <common_headers.h>
#include <ads1115.h>

const i2c_master_dev_handle_t globals_get_i2c_dev_handle_option(int option);
error_type_t globals_set_i2c_dev_handle_option(int option, i2c_master_dev_handle_t handle);
const i2c_master_bus_handle_t globals_get_i2c_bus_handle_option(int option);
error_type_t globals_set_i2c_bus_handle_option(int option, i2c_master_bus_handle_t handle);


#endif // __GLOBALS_H__ 