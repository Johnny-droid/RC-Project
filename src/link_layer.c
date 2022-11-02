// Link layer protocol implementation

#include "../include/link_layer.h"

int fd;
struct termios oldtio;

LinkLayer connectionParameters;
stateMachine_t stateMachine;    // State of the state machine (individual frames)

int alarmEnabled = FALSE;
int error_count = 0;
int Ns = 0; //ns of the frame being worked on

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer newConnectionParameters)
{
    connectionParameters = newConnectionParameters;
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    

    if (fd < 0)
    {
        perror(connectionParameters.serialPort);
        exit(-1);
    }

    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }
    

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = newConnectionParameters.timeout; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;  

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }


    if(fd<=0){
        printf("failed open serialport in llopen\n");
        return -1;
    }


    /////////// Transmiter
    if (connectionParameters.role == LlTx) {

        // create the SET frame
        unsigned char frame[SUPERVISION_SIZE];

        if(createSupFrame(frame, C_SSET)<=0){
            printf("failed to create SET frame in Tx\n");
            return -1;
        }

        stateMachine.curr_global_stage = Waiting_UA;
        stateMachine.curr_state = state_start;

        (void)signal(SIGALRM, alarmHandler);
        error_count = 0;

        while (TRUE) {

            if (error_count > connectionParameters.nRetransmissions) {
                printf("Number of tries reached!\n");
                return -1;
            }

            if (alarmEnabled == FALSE) {
                alarm(connectionParameters.timeout);
                alarmEnabled = TRUE;

                sendFrame(frame, SUPERVISION_SIZE);
                stateMachine.curr_global_stage = Waiting_UA;
            }

            
            readFrame();
            if (stateMachine.curr_global_stage == Received_UA ) {
                alarm(0);
                alarmEnabled = FALSE;
                break;
            }
            
        }

  
        
        Ns = 0;
       return 0;

    ////////// Receiver
    } else {
        
        //read the Set frame
        stateMachine.curr_global_stage = Waiting_SET;
        stateMachine.curr_state = state_start;
        
        readFrame();

        //send UA
        unsigned char frame[SUPERVISION_SIZE];

        if(createSupFrame(frame, C_SUA)<=0){
            printf("failed to create UA frame in Rx\n");
            return -1;
        }

        sendFrame(frame, SUPERVISION_SIZE);

        Ns = 1;
        
        return 0;
    }
    

    
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(unsigned char *buf, unsigned int bufSize)
{
    int ctrl_info_field;
    int Ns_received;
    int frame_size = 6+bufSize; 
    unsigned char frame[MAX_BUF_SIZE]; // To make sure there is no overflow because of stuffing

    if (Ns == 0) {ctrl_info_field = C_INFO_0; } 
    else { ctrl_info_field = C_INFO_1; }

    
    
    if (createInfoFrame(frame, buf, bufSize, ctrl_info_field)<=0) {
        printf("failed to create UA frame in Rx\n");
        return -1;
    }

    frame_size = frameStuffer(frame, frame_size);  // returns real size of frame

    (void)signal(SIGALRM, alarmHandler);
    error_count = 0;
    stateMachine.curr_global_stage = Waiting_RR;
    stateMachine.curr_state = state_start;
    alarmEnabled = FALSE;

    while (TRUE) {

        if (error_count > connectionParameters.nRetransmissions) {
            printf("Number of tries reached!\n");
            return -1;
        }

        if (alarmEnabled == FALSE) {
            alarm(connectionParameters.timeout);
            alarmEnabled = TRUE;
            
            //printf("Ns sent: %d\n",Ns);
            
            sendFrame(frame, frame_size);
            stateMachine.curr_global_stage = Waiting_RR;
            //printf("%d bytes written\n", bytes);
        }

        
        readFrame();


        if (stateMachine.curr_global_stage == Received_RR) {
        
            if (stateMachine.buf[1] == C_RR_0 || stateMachine.buf[1] == C_REJ_0) {
                Ns_received = 0;
            } else if (stateMachine.buf[1] == C_RR_1 || stateMachine.buf[1] == C_REJ_1) {
                Ns_received = 1;
            }
            
            
            if (Ns != Ns_received) { // right Ns received
                alarm(0);
                alarmEnabled = FALSE;
                if (stateMachine.buf[1] == C_REJ_0 || stateMachine.buf[1] == C_REJ_1) {
                    stateMachine.curr_global_stage = Waiting_RR;
                    continue;
                }
                break;
            } else {                // wrong Ns
                error_count++;
                sendFrame(frame, frame_size);
                //alarm(connectionParameters.timeout);
            }
        }
    }


    Ns = (Ns+1) % 2; // switch between 0 and 1

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char* buffer)
{
    stateMachine.curr_global_stage = Waiting_I;
    int ctrl_rr_field, ctrl_rej_field;
    int Ns_received;
    unsigned char frame[SUPERVISION_SIZE];

    //read the I frame
    do {
        readFrame();

        //printf("State after reading: %d\n", stateMachine.curr_global_stage);

        // NEEDS MORE CONDITIONS (not only for set, but for previous read check Ns probably)
        // Received a previous set
        if (stateMachine.curr_global_stage == Received_SET) {
            //send UA
            if(createSupFrame(frame, C_SUA)<=0){
                printf("failed to create UA frame in Rx\n");
                return -1;
            }
            sendFrame(frame, SUPERVISION_SIZE);
            stateMachine.curr_global_stage = Waiting_I;
            continue;
        }; 

        if (stateMachine.curr_global_stage == Received_DISC) {
            // send DISC
            if(createSupFrame(frame, C_DISC)<=0){
                printf("failed to create UA frame in Rx\n");
                return -1;
            }
            sendFrame(frame, SUPERVISION_SIZE);
            stateMachine.curr_global_stage = Waiting_UA;
            break;
        }

        // checks if the frame received is from the previous read (if so, resend the corresponding RR)
        if(stateMachine.buf[1] == C_INFO_0){ Ns_received = 0; ctrl_rr_field = C_RR_1; ctrl_rej_field = C_REJ_1;} 
        else if(stateMachine.buf[1] == C_INFO_1){ Ns_received = 1; ctrl_rr_field = C_RR_0; ctrl_rej_field = C_REJ_0;}
        
        if (stateMachine.curr_global_stage == Received_I_Corrupted) {
            if (createSupFrame(frame, ctrl_rej_field)<=0) {
                printf("failed to create UA frame in Rx\n");
                return -1;
            }
            sendFrame(frame, SUPERVISION_SIZE);
            stateMachine.curr_global_stage = Waiting_I;
        }

        if (Ns == Ns_received) {
            if (createSupFrame(frame, ctrl_rr_field)<=0) {
                printf("failed to create UA frame in Rx\n");
                return -1;
            }
            sendFrame(frame, SUPERVISION_SIZE);
            stateMachine.curr_global_stage = Waiting_I;
        }

    } while (stateMachine.curr_global_stage != Received_I);

    if (stateMachine.curr_global_stage == Waiting_UA) {
        return -1;
    }

    //printf("Received Info frame\n");

    //send RR
    if (createSupFrame(frame, ctrl_rr_field)<=0) {
        printf("failed to create UA frame in Rx\n");
        return -1;
    }

    sendFrame(frame, SUPERVISION_SIZE);
    
    memcpy(buffer, stateMachine.buf + 2, stateMachine.counter-1); // if correct, copies only the data part of the buffer
    
    Ns = (Ns+1) % 2; // switch between 0 and 1

    return stateMachine.counter-1;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{   

    if (connectionParameters.role == LlTx) {
        
        // printf("Inside llclose in tx\n");

        unsigned char frame[SUPERVISION_SIZE];

        if (createSupFrame(frame, C_DISC)<=0) {
            printf("failed to create DISC frame in Tx\n");
            return -1;
        }

        stateMachine.curr_global_stage = Waiting_DISC;

        (void)signal(SIGALRM, alarmHandler);
        alarmEnabled = FALSE;
        error_count = 0;

        while (error_count < connectionParameters.nRetransmissions) {

            if (alarmEnabled == FALSE) {
                alarm(connectionParameters.timeout);
                alarmEnabled = TRUE;
                sendFrame(frame, SUPERVISION_SIZE);
                stateMachine.curr_global_stage = Waiting_DISC;
                
            }
            
            readFrame();

            if (stateMachine.curr_global_stage == Received_DISC ) {
                alarm(0);
                alarmEnabled = FALSE;
                if (createSupFrame(frame, C_SUA)<=0) {
                    printf("failed to create DISC frame in Tx\n");
                    return -1;
                }
                sendFrame(frame, SUPERVISION_SIZE);
                break;
            }
            
        } 
        printf("Connection closed successfully\n");



    } else {
        
        //printf("llclose inside rx\n");

        readFrame();

        if (stateMachine.curr_global_stage == Received_UA ) {
            printf("Connection closed successfully\n");
        }


    }


    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 1;
}


int createSupFrame(unsigned char *frame, unsigned char ctrl_field){
    unsigned char address_byte;
    
    switch (connectionParameters.role)
    {
    case LlTx:
        if(ctrl_field == C_SSET || ctrl_field == C_DISC){
            address_byte=A_TRANS_COMM;
        }else if(ctrl_field==C_SUA || ctrl_field==C_RR_0 || ctrl_field==C_RR_1 || ctrl_field==C_REJ_0 || ctrl_field==C_REJ_1){
            address_byte=A_TRANS_ANS;
        }else{
            return -1;
        }
        break;
        
    case LlRx:
        if(ctrl_field == C_SSET || ctrl_field == C_DISC){
            address_byte=A_REC_COMM;
        }else if(ctrl_field==C_SUA || ctrl_field==C_RR_0 || ctrl_field==C_RR_1 || ctrl_field==C_REJ_0 || ctrl_field==C_REJ_1){
            address_byte=A_REC_ANS;
        }else{
            return -1;
        }
        break;
    
    default:
        return -1;
        break;
    }

    frame[0] = FLAG;
    frame[1] = address_byte;
    frame[2] = ctrl_field;
    frame[3] = address_byte ^ ctrl_field;
    frame[4] = FLAG;

    return 1;

}

int createInfoFrame(unsigned char *frame, unsigned char * data, unsigned int data_size, unsigned char ctrl_field) {
    
    frame[0] = FLAG;
    frame[1] = A_TRANS_COMM;
    frame[2] = ctrl_field;
    frame[3] = A_TRANS_COMM ^ ctrl_field;
    frame[4] = data[0];
    char bcc2 = data[0];
    for (int i = 1; i < data_size; i++) {
        frame[4 + i] = data[i];
        bcc2 ^= data[i];
    }

    frame[4+data_size] = bcc2;
    frame[4+data_size+1]= FLAG;

    return 1;

}

int sendFrame(unsigned char * frame, int frame_size){
    int bytes_written;

    int b = 0;
    while (b != frame_size) {
        bytes_written = write(fd, frame+b, frame_size-b);
        b += bytes_written;
    }
    
    if (b<=0){
        return -1;
    }

    // Just testing
    /*
    printf("Inside sendFrame: ");
    for (int i = 0; i < frame_size; i++) {
        printf("%x", frame[i]);
    }
    printf("\n");
    */

    return b;
}

int readFrame() {
    unsigned char buf[2] = {0}; // +1: Save space for the final '\0' char
    stateMachine.counter = 0;
    stateMachine.curr_state = state_start; //starts state machine

    while (stateMachine.curr_state != state_stop) {
        int bytes = read(fd, buf, 1); 

        if (bytes == 0) {
            printf(".");
            fflush(stdout);
            continue;
        }
        
        /*
        printf("Init State: %d\n", stateMachine.curr_state);
        printf("Byte read: %x \n", buf[0]);
        fflush(stdout);
        */
        //buf[bytes] = '\0'; // Set end of string to '\0', so we can printf
        
        StateMachine_RunIteration(&stateMachine, buf[0]);
        //printf("After State : %d\n", stateMachine.curr_state);
    }

    return 1;
}

void alarmHandler(int signal)
{
    alarmEnabled = FALSE;
    error_count++;
    stateMachine.curr_state = state_stop;

    printf("Alarm #%d\n", error_count);
}

//NOT CORRECT, NEEDS FIXING
int frameStuffer(unsigned char *frame, int frame_size){ 
    unsigned char tempframe[frame_size*2];
    int counter = 0;

    tempframe[0] = frame[0];

    for (int i = 1; i < frame_size-1; i++) { //skip flags
        switch (frame[i])
        {
        case FLAG:
            tempframe[i+counter] = ESC;
            counter++;
            tempframe[i+counter] = FLAG^TRANSPARENCY; // 0x5e 
            break;

        case ESC:
            tempframe[i+counter] = ESC;
            counter++;
            tempframe[i+counter] = ESC^TRANSPARENCY; // 0x5d 
            break;
       
        default:
            tempframe[i+counter] = frame[i];
            break;
        }
    }

    tempframe[frame_size+counter-1] = frame[frame_size-1];

    memcpy(frame, tempframe, frame_size+counter);

    return frame_size+counter;
}


