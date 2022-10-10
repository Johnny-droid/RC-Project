#ifndef READ_NONCAN_H
#define READ_NONCAN_H


#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include "macros.h"
#include "state_machine.h"

int read_noncanonical(int argc, char *argv[]);

#endif