#ifndef FILTER
#define FILTER

#include "freertos/queue.h"
#include "DistanceSensor.h"

extern QueueHandle_t distance_source_queue;
extern QueueHandle_t filtered_distance_queue;

typedef enum {
    MOVING_AVG_FILTER
}FilterStrategy;

int set_filter_strategy(FilterStrategy);
void start_distance_filter();
void stop_distance_filter();

#endif