
#include "state_machine.h"

struct stateTransMatrixRow_t stateTransMatrix[] = {
    {state_start, event_flag, state_flag},
    {state_start, event_a, state_start},
    {state_start, event_c, state_start},
    {state_start, event_bcc, state_start},
    {state_start, event_any, state_start},

    {state_flag, event_a, state_a},
    {state_flag, event_flag, state_flag},
    {state_flag, event_c, state_start},
    {state_flag, event_bcc, state_start},
    {state_flag, event_any, state_start},

    {state_a, event_c, state_c},
    {state_a, event_flag, state_flag},
    {state_a, event_a, state_start},
    {state_a, event_bcc, state_start},
    {state_a, event_any, state_start},

    {state_c, event_bcc, state_bcc},
    {state_c, event_flag, state_flag},
    {state_c, event_a, state_start},
    {state_c, event_c, state_start},
    {state_c, event_any, state_start},

    {state_bcc, event_flag, state_stop},
    {state_bcc, event_a, state_start},
    {state_bcc, event_c, state_start},
    {state_bcc, event_bcc, state_start},
    {state_bcc, event_any, state_start}
};

void StateMachine_Init(stateMachine_t * stateMachine){
    printf("Initializing state machine.\r\n");
    stateMachine->currState = state_start;
}

void StateMachine_RunIteration(stateMachine_t * stateMachine, event_t event){

    for(int i=0; i<sizeof(stateTransMatrix)/sizeof(stateTransMatrix[0]); i++){
        if(stateTransMatrix[i].currState == stateMachine->currState){
            if(stateTransMatrix[i].event==event){
                stateMachine->currState = stateTransMatrix[i].nextState;


            }
        }
    }
}

state_t StateMachine_GetState(stateMachine_t * stateMachine){
    return stateMachine->currState;
}

void StateMachine_Destroy(stateMachine_t * stateMachine){
    free(stateMachine);
}
