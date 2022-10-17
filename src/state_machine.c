#include "../include/state_machine.h"


void StateMachine_RunIteration(stateMachine_t * stateMachine, unsigned char byte){
    
    //////////////////  State Machine of Reading SET frame /////////////////
    if (stateMachine->curr_global_stage == Waiting_SET) {

        switch (stateMachine->curr_state)
        {
        case state_start:
            if (byte == FLAG) { stateMachine->curr_state = state_flag; }
            break;

        case state_flag:
            if (byte == A_TRANS_COMM) { stateMachine->curr_state = state_a; stateMachine->buf[0] = byte; }
            else if (byte == FLAG) { stateMachine->curr_state = state_flag; }
            else { stateMachine->curr_state = state_start; } 
            break;

        case state_a:
            if (byte == C_SSET) { stateMachine->curr_state = state_c; stateMachine->buf[1] = byte; } 
            else if (byte == FLAG) {stateMachine->curr_state = state_flag; }
            else { stateMachine->curr_state = state_start; }
            break;

        case state_c:
            if (byte == ((stateMachine->buf[0])^(stateMachine->buf[1]))) {
                stateMachine->curr_state = state_bcc;
            } else { stateMachine->curr_state = state_start; }
            break;

        case state_bcc:
            if (byte == FLAG) {
                stateMachine->curr_state = state_stop; 
                stateMachine->curr_global_stage = Received_SET;
            }
            else { stateMachine->curr_state = state_start; }
            break;

        default:
            break;
        }


    /////////////////// State Machine of Reading UA frame /////////////
    } else if (stateMachine->curr_global_stage == Waiting_UA) {

        switch (stateMachine->curr_state)
        {
        case state_start:
            if (byte == FLAG) { stateMachine->curr_state = state_flag; }
            break;

        case state_flag:
            if (byte == A_REC_ANS) { stateMachine->curr_state = state_a; stateMachine->buf[0] = byte; }
            else if (byte == FLAG) { stateMachine->curr_state = state_flag; }
            else { stateMachine->curr_state = state_start; } 
            break;

        case state_a:
            if (byte == C_SUA) { stateMachine->curr_state = state_c; stateMachine->buf[1] = byte; } 
            else if (byte == FLAG) {stateMachine->curr_state = state_flag; }
            else { stateMachine->curr_state = state_start; }
            break;

        case state_c:
            if (byte == ((stateMachine->buf[0])^(stateMachine->buf[1]))) {
                stateMachine->curr_state = state_bcc;
            } else { stateMachine->curr_state = state_start; }
            break;

        case state_bcc:
            if (byte == FLAG) {
                stateMachine->curr_state = state_stop; 
                stateMachine->curr_global_stage = Received_UA;
            }
            else { stateMachine->curr_state = state_start; }
            break;

        default:
            break;
        }



    }








    /*
    for(int i=0; i<sizeof(stateTransMatrix)/sizeof(stateTransMatrix[0]); i++){
        if(stateTransMatrix[i].curr_state == stateMachine->curr_state){
            if(stateTransMatrix[i].event==event){
                stateMachine->curr_state = stateTransMatrix[i].nextState;
                

            }
        }
    }
    */
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
