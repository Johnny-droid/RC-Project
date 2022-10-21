#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdio.h>
#include <stdlib.h>
#include "macros.h"

typedef enum {
    Waiting_UA,
    Received_UA,
    Waiting_SET,
    Received_SET,
    Waiting_RR,
    Received_RR,
    Waiting_I,
    Received_I
} stage;

typedef enum {
    state_start,
    state_flag,
    state_a,
    state_c,
    state_bcc,
    state_esc,
    state_data,
    state_bcc2,
    state_stop
} state_t;

typedef struct {
    stage curr_global_stage;            // Current global stage
    state_t curr_state;              // Individual frame reading state
    unsigned char buf[MAX_BUF_SIZE];
    unsigned int counter;
} stateMachine_t;

void StateMachine_RunIteration (stateMachine_t * stateMachine, unsigned char byte);
void StateMachine_Destroy (stateMachine_t * stateMachine);

#endif