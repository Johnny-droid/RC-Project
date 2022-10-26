// Application layer protocol implementation

#include "../include/application_layer.h"


void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    /*
    // temporary message
    unsigned char* message = "Hello world of RC! This is a new era of life!"
    " I'll never turn to the dark side. You've failed, your highness. I am a Jedi, like my father before me."
    " You can't stop the change, any more than you can stop the suns from setting."
    " ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ Bananaaaaaa!";
    unsigned int message_size = strlen((char*) message);
    */

    LinkLayerRole link_role; 
    LinkLayer link_layer;  
    link_layer.nRetransmissions = nTries;
    link_layer.baudRate = baudRate;
    link_layer.timeout = timeout;
    strcpy(link_layer.serialPort, serialPort);

    if (strcmp((char*) role, "tx") == 0) {

        
        //printf("Full message: %s", message);
        char filename_sent[MAX_BUF_SIZE] = "received_";
        unsigned char buf[AL_DATA_SIZE];
        unsigned char chunck[DATA_SIZE_FRAME];
        unsigned int file_size, file_size_sent = 0;
        //unsigned int message_size_sent = 0;
        unsigned int sequence_number = 0;
        int chunck_size;
        link_role = LlTx;
        link_layer.role = link_role;

        // Open file, get the size, and make sure it points back to the begining
        FILE* fp = fopen(filename, "r");
        if (fp == NULL) {
            printf("File Not Found!\n");
            return;
        }
        fseek(fp, 0L, SEEK_END);
        file_size = ftell(fp);
        fseek(fp, 0L, SEEK_SET);


        strcat(filename_sent,filename);
        
        if (llopen(link_layer) < 0) return;

        
        // Send the Start Control Packet 
        if ((chunck_size = packControl(chunck, AL_C_START, file_size, filename_sent)) < 0) {
            printf("Could not pack control start in aplication layer\n");
            return;
        }
        if (llwrite(chunck, chunck_size) < 0) return;
        

        
        // Send the Data Packets with data of size AL_DATA_SIZE
        while (file_size_sent + AL_DATA_SIZE < file_size) {
            fread(buf, sizeof(buf), 1, fp);
            if ((chunck_size = packData(chunck, buf, AL_DATA_SIZE, sequence_number)) < 0) {
                printf("Problem while packing data in application layer\n");
                return;
            }
            if (llwrite(chunck, chunck_size) < 0) return;
            file_size_sent += AL_DATA_SIZE;
            sequence_number = (sequence_number+1) % 256;
        }
        
        // Send the last Data Packet with the remaining data left


        fread(buf, file_size-file_size_sent, 1, fp);
        if ((chunck_size = packData(chunck, buf, file_size-file_size_sent, sequence_number)) < 0) {
            printf("Problem while packing data in application layer\n");
            return;
        }
        if (llwrite(chunck, chunck_size) < 0) return;

        // Send the End Control Packet
        if ((chunck_size = packControl(chunck, AL_C_END, file_size, filename_sent)) < 0) {
            printf("Could not pack control end in aplication layer\n");
        }
        llwrite(chunck, chunck_size);

        llclose(1);

        fclose(fp);

        
    } else {
        link_role = LlRx;
        link_layer.role = link_role;
        llopen(link_layer);
        
        unsigned char packet_received[DATA_SIZE_FRAME];
        unsigned char received[AL_DATA_SIZE+1];
        unsigned int received_size;
        unsigned int file_size_received = 0;
        unsigned int file_size;
        int type, packet_size = 0;
        
        
        packet_size = llread(packet_received);
        type = unpack(packet_received, received, &received_size, &file_size);

        if (type != AL_C_START) return;
        
        FILE* fp = fopen((char*) received, "w");
        if (fp == NULL) {
            printf("File Not Found!\n");
            return;
        };
        

        while (TRUE) {
            
            if ((packet_size = llread(packet_received)) < 0) {
                break;
            }

            if ((type = unpack(packet_received, received, &received_size, &file_size)) < 0) {
                printf("Unpacking not possible\n");
                return;
            }

            if (type == AL_C_DATA) {
                fwrite(received, 1, received_size, fp);
                file_size_received+=received_size;
            }

            
            printf("Receiving : %.2f %%\n", ( (double) file_size_received/ (double) file_size)*100);
        }

        
        llclose(1);

        fclose(fp);

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
    packet[4+sizeof(unsigned int)] = strlen(filename)+1;
    strncpy((char*) (packet + 5+sizeof(unsigned int)), filename, strlen(filename)+1);  

    unsigned int packet_size = 5 + sizeof(unsigned int) + strlen(filename)+1;

    return packet_size;

}

int packData(unsigned char* packet, unsigned char* data, unsigned int data_size, unsigned int n) {
    if (data_size > AL_DATA_SIZE) {
        printf("Data size too large to pack\n");
        return -1;
    }
    packet[0] = AL_C_DATA;
    packet[1] = n;
    packet[2] = data_size / 256;
    packet[3] = data_size % 256;
    memcpy(packet+4, data, data_size);
    return data_size + 4;
}


int unpack(unsigned char* packet, unsigned char* received, unsigned int* received_size, unsigned int* file_size) {

    if (packet[0] == AL_C_DATA) {
        
        unsigned int data_size = packet[2] * 256 + packet[3];
        memcpy(received, packet+4, data_size);
        *received_size = data_size;
        
    } else if (packet[0] == AL_C_START || packet[0] == AL_C_END) {
        
        if (packet[1] != AL_TYPE_FILE_SIZE) {
            printf("Could not get file length\n");
            return -1;
        }
        
        unsigned int length = packet[2];
        memcpy(file_size, packet+3, length);

        if (packet[3+length] != AL_TYPE_FILE_NAME) {
            printf("Could not get filename\n");
            return -1;
        }

        unsigned int length_filename = packet[4+length];
        memcpy(received, packet+length+5, length_filename);
        *received_size = length_filename;
        
    } else {
        return -1;
    }

    return packet[0];
}
