// Link layer protocol implementation

#include "../include/link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

int fd;
struct termios oldtio;


LinkLayer connectionParameters;
stateMachine_t stateMachine;    // State of the state machine (individual frames)

int alarmEnabled = FALSE;
int alarmCount = 0;

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

            //gotta change some of these values (VTIME and VMIN)

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 10; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;  // Blocking read until 5 chars received

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    if(fd<=0){
        printf("failed open serialport in llopen\n");
        return -1;
    }


    /////////// Transmiter
    if (connectionParameters.role == LlTx) {

        unsigned char frame[5];

        if(createSupFrame(frame, C_SSET)<=0){
            printf("failed to create SET frame in Tx\n");
            return -1;
        }


        stateMachine.curr_global_stage = Waiting_UA;

        (void)signal(SIGALRM, alarmHandler);

        while (alarmCount < connectionParameters.nRetransmissions) {

            if (alarmEnabled == FALSE) {
                alarm(connectionParameters.timeout);
                alarmEnabled = TRUE;

                int bytes = sendFrame(frame, 5);
                printf("%d bytes written\n", bytes);
            }

            
            readFrame();
            if (stateMachine.curr_global_stage == Received_UA ) {
                alarm(0);
                alarmEnabled = FALSE;
                break;
            }
            
        } 


        //Just testing
        if (stateMachine.curr_global_stage == Received_UA) {
            printf("\nReceived UA nicely\n");
        } else {
            printf("\nTimeout\n");
        }
        
       

    ////////// Receiver
    } else {
        
        //read the Set frame
        stateMachine.curr_global_stage = Waiting_SET;
        
        readFrame();

        //send UA
        unsigned char frame[5];

        if(createSupFrame(frame, C_SUA)<=0){
            printf("failed to create UA frame in Rx\n");
            return -1;
        }

        sendFrame(frame, 5);

        printf("\n\n\nReached stop state!!!!!!\n\n\n");
    }
    

    return 0;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(int id, const unsigned char *buf, int bufSize)
{
    // TODO
    unsigned char frame[6 + DATA_SIZE_FRAME];



    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
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

int createInfoFrame(unsigned char *frame, unsigned char * data, unsigned char ctrl_field){
    
    char bcc2 = data[0];
    frame[0] = FLAG;
    frame[1] = A_TRANS_COMM;
    frame[2] = ctrl_field;
    frame[3] = A_TRANS_COMM ^ ctrl_field;
    frame[4] = data[0];
    for (int i = 1; i < DATA_SIZE_FRAME; i++) {
        frame[4 + i] = data[i];
        bcc2 ^= data[i];
    }
    frame[4+DATA_SIZE_FRAME] = bcc2;
    frame[4+DATA_SIZE_FRAME+1]= FLAG;

    return 1;

}

int sendFrame(unsigned char * frame, int frame_size){
    int fine;
    fine = write(fd, frame, frame_size);
    if (fine<=0){
        return -1;
    }

    // Just testing
    printf("Inside sendFrame\n");
    for (int i = 0; i < 5; i++) {
        printf("%x\n", frame[i]);
    }

    return fine;
}

int readFrame() {
    unsigned char buf[2] = {0}; // +1: Save space for the final '\0' char

    stateMachine.curr_state = state_start; //starts state machine

    while (stateMachine.curr_state != state_stop) {
        int bytes = read(fd, buf, 1); 

        if (bytes == 0) {
            printf("Didn't read anything \n");
            fflush(stdout);
            continue;
        }
        
        printf("Init State: %d\n", stateMachine.curr_state);
        printf("Byte read: %x \n", buf[0]);
        fflush(stdout);
        buf[bytes] = '\0'; // Set end of string to '\0', so we can printf
        
        StateMachine_RunIteration(&stateMachine, buf[0]);
    }

    return 1;
}

void alarmHandler(int signal)
{
    alarmEnabled = FALSE;
    alarmCount++;
    stateMachine.curr_state = state_stop;

    printf("Alarm #%d\n", alarmCount);
}

