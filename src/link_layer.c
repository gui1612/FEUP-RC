// Link layer protocol implementation


#include "link_layer.h"
#include "state.h"
#include "constants.h"
#include "alarm.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

struct termios oldtio;
int fd;
#define BUF_SIZE 16

////////////////////////////////////////////////
/// LLOPEN                                   ///   
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters) {

    // Open serial port device for reading and writing and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    
    
    if (fd < 0) {
        printf("%s", connectionParameters.serialPort);
        perror(connectionParameters.serialPort);
        exit(-1);
    }

    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1) {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; // Inter-character timer unused
    newtio.c_cc[VMIN] = 1;  // Blocking read until 1 chars received

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    switch (connectionParameters.role){
        case TRANSMITTER:
            set_role(TRANSMITTER);
            unsigned char buf[BUF_SIZE] = {0};
            buf[0] = FLAG;
            buf[1] = ADDR;
            buf[2] = 0x03;
            buf[4] = FLAG;

            // Set alarm function handler
            (void)signal(SIGALRM, alarmHandler);

            if (llwrite(buf, BUF_SIZE)>0)
                printf("SET message sent successfully\n");

            memset(buf, 0, BUF_SIZE);

            if (llread(buf) == 0){
                printf("UA received successfully\n");
                return 1;
            }
            
            break;
        case RECEIVER:
            set_role(RECEIVER);
            unsigned char packet[BUF_SIZE] = {0};

            if (llread(packet) == 0)
                printf("SET received successfully\n");

            unsigned char buf0[BUF_SIZE] = {0};

            buf0[0] = FLAG;
            buf0[1] = ADDR;
            buf0[2] = 0x07;
            buf0[3] = 0x04;
            buf0[4] = FLAG;

            if (llwrite(buf0, BUF_SIZE)>0){
                printf("UA sent successfully\n");
                return 1;
            }
            break;
    }

    return -1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize) {
    int bytes = write(fd, buf, bufSize);
    printf("%d bytes written\n", bytes);

    // Wait until all bytes have been written to the serial port
    sleep(1);
    

    return bytes;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet) {
    int i = 0;
    reset_state();

    while (get_curr_state() != STOP) {
        // Returns after 1 char has been input
        int bytes = read(fd, &packet[i], 1);


        printf("%x\n", packet[i]);
        update_state(packet[i]);

        i++;
    }



    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics) {

    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 1;
}
