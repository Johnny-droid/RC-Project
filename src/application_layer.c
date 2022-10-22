// Application layer protocol implementation

#include "../include/application_layer.h"


void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{

    // temporary message
    unsigned char* message = "Hello world of RC! This is a new era of life!"
    " I'll never turn to the dark side. You've failed, your highness. I am a Jedi, like my father before me."
    " You can't stop the change, any more than you can stop the suns from setting."
    " ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ Bananaaaaaa!";
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
        int size_read = 0;

        while (size_read >= 0) {
            printf("\n%s\n", message_received);
            size_read = llread(message_received);
            message_received[size_read] = '\0';
        }
        
        llclose(1);

        unsigned char packet[DATA_SIZE_FRAME];
        for (int i = 0; i < DATA_SIZE_FRAME; i++) packet[i] = 0;
        packControl(packet, 2, 258, "pingu");
    }

}



int packControl(unsigned char* packet, unsigned int ctrl, unsigned int file_size, const char* filename) {

    if (ctrl == 2) { packet[0] = AL_C_START; }
    else if (ctrl == 3) { packet[0] = AL_C_END; }   
    else { return -1; }

    packet[1] = AL_TYPE_FILE_SIZE;
    packet[2] = sizeof(unsigned int);
    memcpy(packet+3, &file_size, sizeof(unsigned int));
    
    packet[3+sizeof(unsigned int)] = AL_TYPE_FILE_NAME;
    packet[4+sizeof(unsigned int)] = strlen(filename);
    strncpy((char*) (packet + 5+sizeof(unsigned int)), filename, strlen(filename));  

    unsigned int packet_size = 5 + sizeof(unsigned int) + strlen(filename);

    printf("Packet size: %d \nPacket: ", packet_size);
    for (int i = 0; i < packet_size; i++) {
        printf("%x ", packet[i]);
    }

    printf("\nLast element should be: %x\n",  packet[packet_size-1]);

    return packet_size;

}

int packData(unsigned char* packet, unsigned char* data, unsigned int n) {

}


int unpack(unsigned char* packet, unsigned char* received, unsigned int* file_size) {

    if (packet[0] == AL_C_DATA) {
        // TO DO


    } else if (packet[0] == AL_C_START || packet[0] == AL_C_END) {
        // TO DO

        
    } else {
        return -1;
    }

    return packet[0];
}
