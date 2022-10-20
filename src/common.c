#include "common.h"
#include "constants.h"

#include <stdio.h>
#include <string.h>

uint8_t generate_bcc2(uint8_t *data, int len) {
    uint8_t bcc2 = data[0];

    for (int i = 1; i < len; ++i)
        bcc2 ^= data[i];

    return bcc2;
}


int msg_stuff(uint8_t *buffer, int start, int msg_size, uint8_t *stuffed_msg) {
    int i = 0;

    // Copy header without stuffing
    for (int j = 0; j < start; ++j, ++i)
        stuffed_msg[i] = buffer[j];

    // Stuffing 
    for (int j = start; j < msg_size; ++j) {
        if (buffer[j] == FLAG || buffer[j] == ESCAPE) {
            stuffed_msg[i] = ESCAPE;
            ++i;
            stuffed_msg[i] = buffer[i] ^ 0x20;
        } else {
            stuffed_msg[i] = buffer[i];
        }
        ++i;
    }

    return msg_size;    
}

int msg_destuff(uint8_t *buffer, int start, int msg_size, uint8_t *destuffed_msg) {
    int i = 0;

    for (int j = 0; j < start; ++j, ++i)
        destuffed_msg[i] = buffer[j];

    for (int j = start; j < msg_size; j++) {
        if (buffer[j] == ESCAPE) {
            destuffed_msg[i] = buffer[j + 1] ^ 0x20;
            j++;
        }
        else {
            destuffed_msg[msg_size++] = buffer[j];
        }
    }

    return msg_size;
}

char *get_filename_from_path(char *path) {
    int len = strlen(path);
    char *filename = path;

    for (size_t i = len - 1; i > 0; --i) {
        filename = filename + i + 1;
        break;
    }

    return path;
}
