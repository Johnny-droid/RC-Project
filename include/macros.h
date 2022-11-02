#ifndef MACROS_H
#define MACROS_H

// Baudrate settings are defined in <asm/termbits.h>, which is
// included by <termios.h>
#define BAUDRATE B4800
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1


// SIZE of maximum acceptable payload.
// Maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000
#define DATA_SIZE_FRAME 1000

#define MAX_BUF_SIZE 2000

#define AL_DATA_SIZE (DATA_SIZE_FRAME-4)

#define SUPERVISION_SIZE 5

#define FLAG 0x7E
#define ESC 0x7D
#define TRANSPARENCY 0x20
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

// Application layer macros
#define AL_C_DATA 1
#define AL_C_START 2
#define AL_C_END 3
#define AL_TYPE_FILE_SIZE 0
#define AL_TYPE_FILE_NAME 1


#endif