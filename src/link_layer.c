// Link layer protocol implementation

#include "../include/link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

int fd;
struct termios oldtio;

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{

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

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
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
    return fd;



    if(fd<=0){
        printf("failed open noncanonical\n");
        return -1;
    }


    unsigned char frame[10];

    if(createSupFrame(frame, C_SSET, LlTx)<=0){
        printf("failed to create sup frame\n");
        return -1;
    }


    int bytes = sendFrame(frame, 10);
    printf("%d bytes written\n", bytes);

    sleep(1);


    return 0;
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
    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

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

int createInfoFrame(unsigned char *frame, unsigned char * data, unsigned char ctrl_field){
    
    frame[0] = FLAG;
    frame[1] = A_TRANS_COMM;
    frame[2] = ctrl_field;
    frame[3] = A_TRANS_COMM ^ ctrl_field;
    for (int i = 0; i < DATA_SIZE_FRAME; i++) {
        frame[4 + i] = data[i];
    }
    frame[4+DATA_SIZE_FRAME]= A_TRANS_COMM ^ ctrl_field;
    frame[4+DATA_SIZE_FRAME+1]= FLAG;

    return 0;

}

int sendFrame(unsigned char * frame, int frame_size){
    int fine;
    fine = write(fd, frame, frame_size);
    if (fine<=0){
        return -1;
    }
    return fine;
}

int readFrame(unsigned char* byte){
    int fine;
    fine = read(fd, byte, sizeof(unsigned char));
    if (fine<=0){
        return -1;
    }
    return fine;
}


