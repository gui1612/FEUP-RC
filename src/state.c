#include "state.h"
#include "constants.h"

#include <stdio.h>

state_machine state_m;

unsigned char get_address()
{
    return state_m.address;
}

unsigned char get_control()
{
    return state_m.control;
}

state get_curr_state()
{
    return state_m.curr_state;
}

role get_curr_role()
{
    return state_m.curr_role;
}

command get_curr_command()
{
    return state_m.curr_command;
}

response get_prev_response()
{
    return state_m.prev_response;
}

void set_address(unsigned char s)
{
    state_m.address = s;
}

void set_control(unsigned char c)
{
    state_m.control = c;
}

void set_state(state s)
{
    state_m.curr_state = s;
}

void set_role(role r)
{
    state_m.curr_role = r;
}

void set_command(command c)
{
    state_m.curr_command = c;
}

void set_response(response r)
{
    state_m.prev_response = r;
}

void update_state(unsigned char byte)
{
    switch (state_m.curr_state)
    {
    case START:
        if (byte == FLAG)
            set_state(FLAG_RCV);
        break;
    case FLAG_RCV:
        if (byte == FLAG)
            break;
        else if (byte == ADDR)
        {
            set_address(byte);
            set_state(A_RCV);
        }
        else
        {
            set_state(START);
        }

        break;
    case A_RCV:
        if (byte == FLAG)
            set_state(FLAG_RCV);
        else if (byte == 0x03 && get_curr_command() == CMD_SET)
        {
            set_control(byte);
            set_state(C_RCV);
        }
        else if (byte == 0x07 && get_curr_command() == R_UA)
        {
            set_control(byte);
            set_state(C_RCV);
        }
        else if (byte == 0x0B && get_curr_command() == CMD_DISC)
        {
            set_control(byte);
            set_state(C_RCV);
        }
        else if ((byte == 0x00 || byte == 0x40) && get_curr_command() == CMD_DATA)
        {
            set_control(byte);
            set_state(C_RCV);
        }
        else if ((byte == 0x05 || byte == 0x85 || byte == 0x01 || byte == 0x81) && get_curr_command() == R_RR_REJ)
        {
            set_control(byte);
            set_state(C_RCV);
            switch (byte)
            {
            case 0x05:
                set_response(RR_0);
                break;
            case 0x85:
                set_response(RR_1);
                break;
            case 0x01:
                set_response(REJ_0);
                break;
            case 0x81:
                set_response(REJ_1);
                break;
            }
        }
        else
        {
            set_state(START);
        }
        break;
    case C_RCV:
        if (byte == (state_m.address ^ state_m.control))
        {
            if (get_curr_command() == CMD_DATA)
                set_state(RCV_DATA);
            else
                set_state(BCC_OK);
        }
        else if (byte == FLAG)
            set_state(FLAG_RCV);
        else
            set_state(START);
        break;
    case BCC_OK:
        if (byte == FLAG)
            set_state(STOP);
        else
            set_state(START);
        break;
    case RCV_DATA:
        if (byte == FLAG)
            set_state(STOP);
        break;
    case STOP:
        break;
    }
}

void reset_state()
{
    state_m.curr_state = START;
    state_m.prev_response = R_NULL;
}