#ifndef FILTER
#define FILTER

#include "DistanceSensor.h"

typedef enum {
    MOVING_AVG_FILTER
}FilterStrategy;

int set_filter_strategy(FilterStrategy);
int set_sensor_count(int);
int start_distance_filter(Os_QueueHandle source_queue, Os_QueueHandle output_queue);
int stop_distance_filter();

#endif