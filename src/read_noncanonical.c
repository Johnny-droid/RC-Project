// Read from serial port in non-canonical mode
//
// Modified by: Eduardo Nuno Almeida [enalmeida@fe.up.pt]

#include "../include/read_noncanonical.h"


int read_noncanonical(int argc, char *argv[])
{
    // Program usage: Uses either COM1 or COM2
    const char *serialPortName = argv[1];

    if (argc < 2)
    {
        printf("Incorrect program usage\n"
               "Usage: %s <SerialPort>\n"
               "Example: %s /dev/ttyS1\n",
               argv[0],
               argv[0]);
        exit(1);
    }

    // Open serial port device for reading and writing and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    int fd = open(serialPortName, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(serialPortName);
        exit(-1);
    }

    struct termios oldtio;
    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 10; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;  // Blocking read until 5 chars received

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    // Loop for input
    unsigned char buf[2] = {0}; // +1: Save space for the final '\0' char

    stateMachine_t stateMachine; //creates state machine
    StateMachine_Init(&stateMachine); //starts state machine

    while (StateMachine_GetState(&stateMachine) != state_stop) {
        // Returns after 5 chars have been input
        printf("Didn't read anything \n");
        fflush(stdout);
        int bytes = read(fd, buf, 1);   // CHANGE to read one byte at a time
        if (bytes == 0) continue;
        printf("Init State: %d\n", StateMachine_GetState(&stateMachine));
        printf("Byte read:%x \n", buf[0]);
        fflush(stdout);
        buf[bytes] = '\0'; // Set end of string to '\0', so we can printf
        
        StateMachine_RunIteration(&stateMachine, buf[0]);
        
        /*
        for(int i=0; i<bytes; i++){ //iterates through every byte of the buf received

            switch (buf[i]) 
            {
            case SET_FLAG: //flag
                StateMachine_RunIteration(&stateMachine, event_flag);
                break;
            case A_TRANSMITTER: //a
                StateMachine_RunIteration(&stateMachine, event_a);
                temp[0]=buf[i];
                break;
            case C_SSET: //c
                StateMachine_RunIteration(&stateMachine, event_c);
                temp[1]=buf[i];
                break;
            //case (temp[0]^temp[1]): //compares byte to A xor C (they are saved in temp)
            case 0:
                StateMachine_RunIteration(&stateMachine, event_bcc);
                break;
            
            default:
                StateMachine_RunIteration(&stateMachine, event_any); //any other byte resets the machine
                break;
            }
        
        }
        */

    }

    printf("\n\n\nReached stop state!!!!!!\n\n\n");

    // The while() cycle should be changed in order to respect the specifications
    // of the protocol indicated in the Lab guide

    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 0;
}
