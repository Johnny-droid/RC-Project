#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H


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
} stateMachine_t;

typedef enum {
    event_flag, //flag_rcv
    event_a, //a_rcv
    event_c, //c_rcv
    event_bcc, //bcc_rcv
    event_any
}event_t;

typedef struct {
    state_t currState;
    event_t event;
    state_t nextState;
} stateTransMatrixRow_t;



void StateMachine_Init(stateMachine_t * stateMachine);
void StateMachine_RunIteration (stateMachine_t * stateMachine, event_t event);
state_t StateMachine_GetState (stateMachine_t * stateMachine);
void StateMahcine_Destroy (stateMachine_t * stateMachine);
#endif