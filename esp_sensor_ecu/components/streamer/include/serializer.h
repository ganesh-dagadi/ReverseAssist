#ifndef SERIALIZER
#define SERIALIZER

#include <stdio.h>
#include <DistanceSensor.h>

size_t calculate_payload_size(const DistanceData*);
size_t calculate_status_payload_size(const SensorStatusData*);
size_t calculate_header_size();
void serialize_data(const DistanceData* data, uint8_t* buffer, size_t len);
void serialize_status_data(const SensorStatusData* data, uint8_t* buffer, size_t len);

#endif