#include "../include/state_machine.h"


void StateMachine_Init(stateMachine_t * stateMachine) {
    stateMachine->currState = state_start;
}


void StateMachine_RunIteration(stateMachine_t * stateMachine, unsigned char byte){

    switch (stateMachine->currState)
    {
    case state_start:
        if (byte == FLAG_SET) { stateMachine->currState = state_flag; }
        break;

    case state_flag:
        if (byte == A_TRANSMITTER) { stateMachine->currState = state_a; stateMachine->buf[0] = byte; }
        else if (byte == FLAG_SET) { stateMachine->currState = state_flag; }
        else { stateMachine->currState = state_start; } 
        break;

    case state_a:
        if (byte == C_SSET) { stateMachine->currState = state_c; stateMachine->buf[1] = byte; }
        else if (byte == FLAG_SET) {stateMachine->currState = state_flag; }
        else { stateMachine->currState = state_start; }
        break;

    case state_c:
        if (byte == ((stateMachine->buf[0])^(stateMachine->buf[1]))) {
            stateMachine->currState = state_bcc;
        } else { stateMachine->currState = state_start; }
        break;

    case state_bcc:
        if (byte == FLAG_SET) {stateMachine->currState = state_stop; }
        else { stateMachine->currState = state_start; }
        break;

    default:
        break;
    }

    /*
    for(int i=0; i<sizeof(stateTransMatrix)/sizeof(stateTransMatrix[0]); i++){
        if(stateTransMatrix[i].currState == stateMachine->currState){
            if(stateTransMatrix[i].event==event){
                stateMachine->currState = stateTransMatrix[i].nextState;
                

            }
        }
    }
    */
}

state_t StateMachine_GetState(stateMachine_t * stateMachine){
    return stateMachine->currState;
}

void StateMachine_Destroy(stateMachine_t * stateMachine){
    free(stateMachine);
}



/*
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
*/
