#ifndef STREAMER
#define STREAMER

#include "DistanceSensor.h"
#include "Filter.h"

extern QueueHandle_t filtered_distance_queue;
int start_streamer();
#endif