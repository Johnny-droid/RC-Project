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


    /////////////////// State Machine of Reading UA frame ////////////////
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


    /////////////////// State Machine of Reading I frames ////////////////
    } else if (stateMachine->curr_global_stage == Waiting_I) {

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
            if (byte == C_INFO_0 || byte == C_INFO_1) { stateMachine->curr_state = state_c; stateMachine->buf[1] = byte; } 
            else if (byte == C_SSET) { stateMachine->curr_global_stage = Waiting_SET; stateMachine->curr_state = state_c; stateMachine->buf[1] = byte; } 
            else if (byte == FLAG) {stateMachine->curr_state = state_flag; }
            else { stateMachine->curr_state = state_start; }
            break;

        case state_c:
            if (byte == ((stateMachine->buf[0])^(stateMachine->buf[1]))) {
                stateMachine->curr_state = state_bcc;
            } else { stateMachine->curr_state = state_start; }
            break;

        case state_bcc:
            if (byte == ESC) { stateMachine->curr_state = state_esc; }
            else { 
                stateMachine->curr_state = state_data; 
                stateMachine->counter++; 
                stateMachine->buf[1+stateMachine->counter] = byte;
            }
            break;

        case state_esc:
            if (byte == (FLAG^TRANSPARENCY)) {
                stateMachine->counter++; 
                stateMachine->buf[1+stateMachine->counter] = FLAG;
                stateMachine->curr_state = state_data;
            } else if (byte == (ESC^TRANSPARENCY)) {
                stateMachine->counter++; 
                stateMachine->buf[1+stateMachine->counter] = ESC;
                stateMachine->curr_state = state_data;
            } // what should we do if it's neither of these? Not sure.
            break;

        case state_data:
            if (byte == ESC) { stateMachine->curr_state = state_esc; }
            else if (stateMachine->counter == DATA_SIZE_FRAME) {
                int bcc2 = stateMachine->buf[2];
                for (int i = 1; i < DATA_SIZE_FRAME; i++) {
                    bcc2 ^= stateMachine->buf[2+i];
                }
                if (bcc2 == byte) { stateMachine->curr_state = state_bcc2; }
                else {stateMachine->curr_state = state_start;} 
            } else {
                stateMachine->counter++; 
                stateMachine->buf[1+stateMachine->counter] = byte;
            }
            break;

        case state_bcc2:
            if (byte == FLAG) {
                stateMachine->curr_state = state_stop; 
                stateMachine->curr_global_stage = Received_I;
            }
            else { stateMachine->curr_state = state_start; }
            break;

        default:
            break;
        }




    /////////////////// State Machine of Reading RR frame /////////////
    } else if (stateMachine->curr_global_stage == Waiting_RR) {

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
            if (byte == C_RR_0 || byte == C_RR_1) { stateMachine->curr_state = state_c; stateMachine->buf[1] = byte; } 
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
                stateMachine->curr_global_stage = Received_RR;
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
