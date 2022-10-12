#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdio.h>
#include <stdlib.h>
#include "macros.h"

typedef enum {
    state_start,
    state_flag,
    state_a,
    state_c,
    state_bcc,
    state_stop
} state_t;

typedef struct{
    state_t currState;
    unsigned char buf[BUF_SIZE];
    unsigned char address_byte;
    unsigned char ctrl_byte;
} stateMachine_t;

void StateMachine_Init(stateMachine_t * stateMachine);
void StateMachine_RunIteration (stateMachine_t * stateMachine, unsigned char byte);
state_t StateMachine_GetState (stateMachine_t * stateMachine);
void StateMahcine_Destroy (stateMachine_t * stateMachine);

#endif