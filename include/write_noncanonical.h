#ifndef WRITE_NONCAN_H
#define WRITE_NONCAN_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include "macros.h"

int write_noncanonical(int argc, char *argv[]);


#endif
