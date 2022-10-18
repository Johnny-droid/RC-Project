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



}


void StateMachine_Destroy(stateMachine_t * stateMachine){
    free(stateMachine);
}
