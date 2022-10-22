// Application layer protocol implementation

#include "../include/application_layer.h"


void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{

    // temporary message
    unsigned char* message = "Hello world of RC! This is a new era of life!"
    " I'll never turn to the dark side. You've failed, your highness. I am a Jedi, like my father before me."
    " You can't stop the change, any more than you can stop the suns from setting."
    " ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ Banana!";
    unsigned int message_size = strlen((char*) message);

    LinkLayerRole link_role; 
    LinkLayer link_layer;  
    link_layer.nRetransmissions = nTries;
    link_layer.baudRate = baudRate;
    link_layer.timeout = timeout;

    if (strcmp((char*) role, "tx") == 0) {
        
        printf("Full message: %s", message);

        unsigned int message_size_sent = 0;
        unsigned char chunck[DATA_SIZE_FRAME];
        link_role = LlTx;
        link_layer.role = link_role;
        strcpy(link_layer.serialPort, "/dev/ttyS10");
        llopen(link_layer);

        while (message_size_sent + DATA_SIZE_FRAME < message_size) {
            memcpy(chunck, message + message_size_sent, DATA_SIZE_FRAME);
            llwrite(chunck, DATA_SIZE_FRAME);
            message_size_sent += DATA_SIZE_FRAME;
            printf("Sent by cicle llwrite\n");
        }
        
        
        memcpy(chunck, message + message_size_sent, message_size - message_size_sent);
        llwrite(chunck, message_size - message_size_sent);

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
        
        unsigned char message_received[DATA_SIZE_FRAME+1];
        int finished = FALSE;

        while (!finished) {
            printf("\n%s\n", message_received);
            finished = llread(message_received);
            message_received[DATA_SIZE_FRAME] = '\0';
            
        }
        
        llclose(1);

    }
    
    



}
