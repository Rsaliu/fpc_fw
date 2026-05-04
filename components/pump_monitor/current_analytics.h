#ifndef CURRENT_ANALYTICS_H
#define CURRENT_ANALYTICS_H
#include "pump_monitor.h"
#include "common_headers.h"


error_type_t current_analytics_basic_decision(float* sampled_current_values, int number_of_samples, float rated_current, float min_working_current, pump_state_machine_state_t* state);

#endif // CURRENT_ANALYTICS_H