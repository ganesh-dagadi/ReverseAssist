#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#define DISTANCE_POLL_DELAY 100
typedef enum {
    SLEEPING,
    STARTING,
    IDLE,
    RUNNING,
    ERROR
} StateMachineState;

void start_state_machine(void *params);

#endif // STATE_MACHINE_H