#ifndef __EVENT_H__
#define __EVENT_H__

typedef enum{
    EVENT_TANK_NORMAL_STATE = 0, // Normal state of the tank
    EVENT_TANK_FULL_STATE = 1, // Tank is full
    EVENT_TANK_LOW_STATE = 2, // Tank is below low level
    EVENT_PUMP_OVERCURRENT = 3, // Pump is in overcurrent state
    EVENT_PUMP_NORMAL = 4, // Pump is in normal state
    EVENT_UNKNOWN = 5, // Unknown event
    EVENT_PUMP_UNDERCURRENT = 6, // Pump current supply is below normal
}event_type_t;


#endif // __EVENT_H__