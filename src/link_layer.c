// Link layer protocol implementation

#include "../include/link_layer.h"





// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    int fd;
    struct termios oldtio;

    fd=nonCanonical(connectionParameters.serialPort, &oldtio, connectionParameters.timeout, 0);
    if(fd<=0){
        printf("failed open noncanonical\n");
        return -1;
    }



    switch (connectionParameters.role)
    {
    case LlRx:


        // Loop for input
        unsigned char buf[2] = {0}; // +1: Save space for the final '\0' char

        stateMachine_t stateMachine; //creates state machine
        StateMachine_Init(&stateMachine); //starts state machine

        while (StateMachine_GetState(&stateMachine) != state_stop) {
            if(readFrame(&buf, fd)<=0){
                printf("failed to read frame");
                return -1;
            }
            buf[1] = '\0'; // Set end of string to '\0', so we can printf
            printf(":%s", buf);
            StateMachine_RunIteration(&stateMachine, buf[0]);

        }

        printf("\nReached stop state");

        // Create ua to send
        unsigned char frame[10];

        if(createSupFrame(&frame, C_SUA, LlRx)<=0){
            printf("failed to create sup frame\n");
            return -1;
        }

        break;

         //int bytes = write(fd, buf, BUF_SIZE);
        int bytes = write(fd, frame, 10);
        printf("%d bytes written\n", bytes);

        // Wait until all bytes have been written to the serial port
        sleep(1);

    case LlTx:

        unsigned char frame[10];

        if(createSupFrame(&frame, C_SSET, LlTx)<=0){
            printf("failed to create sup frame\n");
            return -1;
        }

        int bytes = write(fd, frame, 10);
        printf("%d bytes written\n", bytes);

        sleep(1);



        unsigned char buf[2] = {0}; // +1: Save space for the final '\0' char

        stateMachine_t stateMachine; //creates state machine
        StateMachine_Init(&stateMachine); //starts state machine

        while (StateMachine_GetState(&stateMachine) != state_stop) {
            if(readFrame(&buf, fd)<=0){
                printf("failed to read frame");
                return -1;
            }
            buf[1] = '\0'; // Set end of string to '\0', so we can printf
            printf(":%s", buf);
            StateMachine_RunIteration(&stateMachine, buf[0]);

        }

        printf("\nReached stop state");

    
    default:
        printf("something is VERY wrong");
        break;
    }


    if(nonCanonicalClose(fd, &oldtio)<=0){
        printf("failed close noncanonical\n");
        return -1;
    }


    return 1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    // TODO

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
    // TODO

    return 1;
}


int createSupFrame(unsigned char *frame, unsigned char ctrl_field, LinkLayerRole role){
    unsigned char address_byte;
    
    switch (role)
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

int createInfoFrame(unsigned char *frame, unsigned char * data, unsigned char ctrl_field, int data_size){
    
    frame[0] = FLAG;
    frame[1] = A_TRANS_COMM;
    frame[2] = ctrl_field;
    frame[3] = A_TRANS_COMM ^ ctrl_field;
    frame[4] = data;
    frame[4+data_size]= A_TRANS_COMM ^ ctrl_field;
    frame[4+data_size+1]= FLAG;

    
    
}

int sendFrame(unsigned char * frame, int fd, int frame_size){
    int fine;
    fine=write(fd, frame, frame_size);
    if (fine<=0){
        return -1;
    }
    return fine;
}

int readFrame(unsigned char* byte, int fd ){
    int fine;
    fine=read(fd, byte, sizeof(unsigned char));
    if (fine<=0){
        return -1;
    }
    return fine;
}

int nonCanonicalOpen(char* port, struct termios * oldtio, int vtime, int vmin){

    int fd = open(port, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(port);
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
    newtio.c_cc[VTIME] = vtime; // Inter-character timer unused
    newtio.c_cc[VMIN] = vmin;  // Blocking read until 5 chars received

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
    return fd;
}

int nonCanonicalClose(int fd, struct termios *oldtio){
    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);
}

