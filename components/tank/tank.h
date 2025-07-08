#ifndef __TANK_LIBRARY_H__
#define __TANK_LIBRARY_H__
#include <common_headers.h>
typedef enum{
    TANK_SHAPE_RECTANGLE = 0,
    TANK_SHAPE_CYLINDER = 1,
}tank_shape_t;

typedef enum {
    TANK_NOT_INITIALIZED = 0,
    TANK_INITIALIZED = 1,
} tank_state_t;

typedef struct {
    int id; // Unique identifier for the tank
    float capacity_in_liters; // Total capacity of the tank in liters
    tank_shape_t shape; // Shape of the tank (rectangle or cylinder)
    float height_in_cm; // Height of the tank in centimeters
    int full_level_in_mm; // Full level of the tank in centimeters
    int low_level_in_mm; // Low level of the tank in centimeters

} tank_config_t;

typedef struct tank_t tank_t;

tank_t* tank_create(tank_config_t config);
error_type_t tank_init(tank_t *tank);
error_type_t tank_deinit(tank_t *tank);
error_type_t tank_destroy(tank_t **tank);
error_type_t tank_get_config(tank_t *tank, tank_config_t *config);
error_type_t tank_get_state(tank_t *tank, tank_state_t *state);

#endif //__TANK_LIBRARY_H__