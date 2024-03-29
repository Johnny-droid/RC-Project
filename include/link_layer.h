// Link layer header.
// NOTE: This file must not be changed.

#ifndef _LINK_LAYER_H_
#define _LINK_LAYER_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include "macros.h"
#include "state_machine.h"

typedef enum
{
    LlTx,
    LlRx,
} LinkLayerRole;

typedef struct
{
    char serialPort[50];
    LinkLayerRole role;
    int baudRate;
    int nRetransmissions;
    int timeout;

} LinkLayer;


// MISC
#define FALSE 0
#define TRUE 1



// Open a connection using the "port" parameters defined in struct linkLayer.
// Return "1" on success or "-1" on error.
int llopen(LinkLayer connectionParameters);

// Send data in buf with size bufSize.
// Return number of chars written, or "-1" on error.
int llwrite(unsigned char *buf, unsigned int bufSize);

// Receive data in packet.
// Return number of chars read, or "-1" on error.
int llread(unsigned char * buffer);

// Close previously opened connection.
// if showStatistics == TRUE, link layer should print statistics in the console on close.
// Return "1" on success or "-1" on error.
int llclose(int showStatistics);


int createSupFrame(unsigned char *frame, unsigned char ctrl_field);
int createInfoFrame(unsigned char *frame, unsigned char *data, unsigned int data_size, unsigned char ctrl_field);
int sendFrame(unsigned char * frame, int frame_size);
int readFrame();
void alarmHandler(int signal);

int frameStuffer(unsigned char *frame, int frame_size);
//void frameDeStuffer(unsigned char *frame, int bufSize);



#endif // _LINK_LAYER_H_
