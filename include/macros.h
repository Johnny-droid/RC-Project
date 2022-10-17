#ifndef MACROS_H
#define MACROS_H

// Baudrate settings are defined in <asm/termbits.h>, which is
// included by <termios.h>
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1

#define BUF_SIZE 256

#define DATA_SIZE_FRAME 1

#define FLAG 0x7E
#define A_TRANS_COMM 0x03
#define A_REC_ANS 0x03
#define A_TRANS_ANS 0x01
#define A_REC_COMM 0x01
#define C_SSET 0x03    // Supervision SET
#define C_SUA 0x07     // Supervision UA
#define C_DISC 0x0B    // Supervision DISC
#define C_RR_0 0x05    // Supervision RR N(0)
#define C_RR_1 0x85    // Supervision RR N(1)
#define C_REJ_0 0x01   // Supervision REJ N(0)
#define C_REJ_1 0x81   // Supervision REJ N(1)
#define C_INFO_0 0x00  // Information N(0)
#define C_INFO_1 0x50  // Information N(1)




#endif