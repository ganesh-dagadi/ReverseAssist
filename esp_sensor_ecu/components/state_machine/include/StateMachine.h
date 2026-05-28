#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#define DISTANCE_POLL_DELAY 100
typedef enum {
    SLEEPING,
    STARTING,
    RUNNING,
    ERROR,
} StateMachineState;

int init_state_machine(void);
void start_state_machine(void *params);

#endif // STATE_MACHINE_H