// Link layer protocol implementation

#include "../include/link_layer.h"





// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{


    int fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(connectionParameters.serialPort);
        exit(-1);
    }

    struct termios oldtio;
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
    newtio.c_cc[VTIME] = 0; // Inter-character timer unused
    newtio.c_cc[VMIN] = 5;  // Blocking read until 5 chars received

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




    switch (connectionParameters.role)
    {
    case LlRx:


        // Loop for input
        unsigned char buf[2] = {0}; // +1: Save space for the final '\0' char

        stateMachine_t stateMachine; //creates state machine
        StateMachine_Init(&stateMachine); //starts state machine

        while (StateMachine_GetState(&stateMachine) != state_stop) {
            int bytes = read(fd, buf, 1);   // CHANGE to read one byte at a time
            buf[bytes] = '\0'; // Set end of string to '\0', so we can printf
            printf(":%s", buf);
            StateMachine_RunIteration(&stateMachine, buf[0]);

        }

        printf("\nReached stop state");

        // Create ua to send
        unsigned char ua[10];

        ua[0] = FLAG_SET;
        ua[2] = A_TRANSMITTER;
        ua[4] = C_SUA;
        ua[6] = A_TRANSMITTER ^ C_SUA;
        ua[8] = FLAG_SET;

        break;

         //int bytes = write(fd, buf, BUF_SIZE);
        int bytes = write(fd, ua, 10);
        printf("%d bytes written\n", bytes);

        // Wait until all bytes have been written to the serial port
        sleep(1);

    case LlTx:


        // Create set to send
        unsigned char set[10];

        set[0] = FLAG_SET;
        set[2] = A_TRANSMITTER;
        set[4] = C_SSET;
        set[6] = A_TRANSMITTER ^ C_SSET;
        set[8] = FLAG_SET;

        for (int i = 0; i < 10; i+=2) {
            printf("%x\n", set[i]);
        }

        /*
        // In non-canonical mode, '\n' does not end the writing.
        // Test this condition by placing a '\n' in the middle of the buffer.
        // The whole buffer must be sent even with the '\n'.
        buf[5] = '\n';
        */

        //int bytes = write(fd, buf, BUF_SIZE);
        int bytes = write(fd, set, 10);
        printf("%d bytes written\n", bytes);

        // Wait until all bytes have been written to the serial port
        sleep(1);



        // Loop for input
        unsigned char buf[2] = {0}; // +1: Save space for the final '\0' char

        stateMachine_t stateMachine; //creates state machine
        StateMachine_Init(&stateMachine); //starts state machine

        while (StateMachine_GetState(&stateMachine) != state_stop) {
            int bytes = read(fd, buf, 1);   // CHANGE to read one byte at a time
            buf[bytes] = '\0'; // Set end of string to '\0', so we can printf
            printf(":%s", buf);
            StateMachine_RunIteration(&stateMachine, buf[0]);

        }

        printf("\nReached stop state");

    
    default:
        printf("something is VERY wrong");
        break;
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

