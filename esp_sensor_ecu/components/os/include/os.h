#ifndef OS
#define OS

#include <stdint.h>
typedef void* Os_QueueHandle;
typedef void* Os_TaskHandle;
typedef void (*Os_TaskFunction)(void* arg);

int create_queue(uint8_t length, uint32_t size_of_ele, Os_QueueHandle* handle);
int destory_queue(Os_QueueHandle);
int push_queue(Os_QueueHandle, void* data);
int poll_queue_blocking(Os_QueueHandle, void* data);

int create_task(Os_TaskFunction task, const char* name, uint8_t priority, uint32_t size, Os_TaskHandle* handle);
int stop_task(Os_TaskHandle task);
void sleep_task(uint32_t delay);

int suspend_task(Os_TaskHandle task);
int resume_task(Os_TaskHandle task);

#endif