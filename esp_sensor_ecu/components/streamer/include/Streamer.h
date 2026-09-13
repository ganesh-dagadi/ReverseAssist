#ifndef STREAMER
#define STREAMER

#include "DistanceSensor.h"
#include "os.h"

int start_streamer(Os_QueueHandle input_queue, Os_QueueHandle status_queue);
#endif