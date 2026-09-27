#ifndef DESERIALIZER_H
#define DESERIALIZER_H

#include <stddef.h>
#include <stdint.h>
#include "DistanceSensor.h"

#define DESERIALIZER_MAX_PAYLOAD 255

typedef void (*DeserializerCommandCallback)(DistanceSensorCommands command);

typedef struct {
	uint8_t state;
	uint8_t header_index;
	uint8_t payload_length;
	size_t payload_index;
	uint8_t frame[1 + 3 + DESERIALIZER_MAX_PAYLOAD + 1];
	DeserializerCommandCallback command_callback;
} Deserializer;

void deserializer_init(Deserializer *deserializer,
					   DeserializerCommandCallback command_callback);
void deserializer_process(Deserializer *deserializer,
						  const uint8_t *data, size_t length);

#endif