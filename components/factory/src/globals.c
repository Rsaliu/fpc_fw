#include <globals.h>
#include <string.h>

static i2c_master_dev_handle_t i2c_dev_handle_one = NULL;
static i2c_master_bus_handle_t i2c_bus_handle_one = NULL;

const i2c_master_dev_handle_t globals_get_i2c_dev_handle_option(int option){
    switch(option){
        case 1:
            return i2c_dev_handle_one;
        default:
            return NULL;
    }
}
const i2c_master_bus_handle_t globals_get_i2c_bus_handle_option(int option){
    switch(option){
        case 1:
            return i2c_bus_handle_one;
        default:
            return NULL;
    }
}

error_type_t globals_set_i2c_dev_handle_option(int option, i2c_master_dev_handle_t handle){
    switch(option){
        case 1:
            i2c_dev_handle_one = handle;
            return SYSTEM_OK;
        default:
            return SYSTEM_INVALID_PARAMETER;
    }
}
error_type_t globals_set_i2c_bus_handle_option(int option, i2c_master_bus_handle_t handle){
    switch(option){
        case 1:
            i2c_bus_handle_one = handle;
            return SYSTEM_OK;
        default:
            return SYSTEM_INVALID_PARAMETER;
    }
}