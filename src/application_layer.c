// Application layer protocol implementation

#include "../include/application_layer.h"


void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{

    // temporary message
    unsigned char* message = "Hello world of RC! This is a new era of life!";
    unsigned int message_size = strlen((char*) message);

    LinkLayerRole link_role; 
    LinkLayer link_layer;  
    link_layer.nRetransmissions = nTries;
    link_layer.baudRate = baudRate;
    link_layer.timeout = timeout;

    if (strcmp((char*) role, "tx") == 0) {
        link_role = LlTx;
        link_layer.role = link_role;
        strcpy(link_layer.serialPort, "/dev/ttyS10");
        llopen(link_layer);

        llwrite(message, message_size);

        llclose(1);

        /*
        printf("write\n");
        char* argv_write[] = {"make", "/dev/ttyS10"};
        write_noncanonical(2, argv_write);
        */
    } else {
        link_role = LlRx;
        link_layer.role = link_role;
        strcpy(link_layer.serialPort, "/dev/ttyS11");
        llopen(link_layer);
        
        unsigned char message_received[DATA_SIZE_FRAME];
        int finished = FALSE;

        while (!finished) {
            finished = llread(message_received);
            printf("\n%s\n", message_received);
        }
        
        llclose(1);


        

        /*
        char* argv_read[] = {"make", "/dev/ttyS11"};
        read_noncanonical(2, argv_read);
        */

    }
    
    //LinkLayer link_config = { .serialPort = *serialPort, .role = link_role, baudRate = baudRate, .nRetransmissions = nTries, timeout = timeout};

    /*
    if(llopen(link_config)==1) {
        
    }else{
        printf("llopen() failed miserably, we thank your consideration");
    }
    */
}
