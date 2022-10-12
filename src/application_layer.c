// Application layer protocol implementation

#include "../include/application_layer.h"
#include "../include/read_noncanonical.h"
#include "../include/write_noncanonical.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    if (strcmp((char*) role, "tx") == 0) {
        printf("write\n");
        char* argv_write[] = {"make", "/dev/ttyS10"};
        write_noncanonical(2, argv_write);
    } else {
        printf("read\n");
        char* argv_read[] = {"make", "/dev/ttyS11"};
        read_noncanonical(2, argv_read);
    }
}
