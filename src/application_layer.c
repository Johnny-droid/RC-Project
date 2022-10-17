// Application layer protocol implementation

#include "../include/application_layer.h"


void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    //LinkLayerRole link_role;
    

    if (strcmp((char*) role, "tx") == 0) {
        printf("write\n");
        char* argv_write[] = {"make", "/dev/ttyS10"};
        write_noncanonical(2, argv_write);
        //link_role = LlTx;
    } else {
        printf("read\n");
        char* argv_read[] = {"make", "/dev/ttyS11"};
        read_noncanonical(2, argv_read);
        //link_role = LlRx;

    }
    
    //LinkLayer link_config = { .serialPort = *serialPort, .role = link_role, baudRate = baudRate, .nRetransmissions = nTries, timeout = timeout};

    /*
    if(llopen(link_config)==1) {
        
    }else{
        printf("llopen() failed miserably, we thank your consideration");
    }
    */
}
