#ifndef MACROS_H
#define MACROS_H

// Baudrate settings are defined in <asm/termbits.h>, which is
// included by <termios.h>
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1

#define BUF_SIZE 256

#define FLAG_SET 0x7E
#define A_TRANSMITTER 0x03
#define A_RECEIVER 0x01
#define C_SSET 0x03   // Supervision Set
#define C_SUA 0x07    // Supervision UA

#endif