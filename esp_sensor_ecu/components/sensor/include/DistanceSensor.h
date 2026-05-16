#ifndef DISTANCE_SENSOR
#define DISTANCE_SENSOR

typedef struct DistanceSensor DistanceSensor;

DistanceSensor* create_distance_sensor(int sensor_id);
int init_distance_sensor(DistanceSensor* sensor);
int poll_distance(DistanceSensor* sensor);
void close_sensor(DistanceSensor* sensor);

#endif