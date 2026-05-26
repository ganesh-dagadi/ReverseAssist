#ifndef DISTANCE_SENSOR
#define DISTANCE_SENSOR

typedef struct DistanceSensor DistanceSensor;

typedef struct {
    uint64_t timestamp;
    uint8_t distance;
} DistanceData;

extern QueueHandle_t distance_source_queue;

DistanceSensor* create_distance_sensor(int sensor_id);
int init_distance_sensor(DistanceSensor* sensor);
int poll_distance(DistanceSensor* sensor);
void close_sensor(DistanceSensor* sensor);
void sleep_sensor(DistanceSensor* sensor);
void resume_sensor(DistanceSensor* sensor);

#endif