#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

typedef enum {
    SLEEPING,
    STARTING,
    STARTED,
    RESTARTING,
    RESTARTED,
    SHUTDOWN,
    PAUSE_SENSOR_INP,
    PAUSE_STREAM_OUT
} StateMachineState;

int init_state_machine(void);
void start_state_machine(void *params);

#endif // STATE_MACHINE_H