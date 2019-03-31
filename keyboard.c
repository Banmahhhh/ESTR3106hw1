#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <simpl.h>
#include "message.h"

int admin;
int end = 0;
MESSAGE msg, reply;

void error_msg(void);
void registration(void);
void play_game(void);
void ksend(void);

int main(void)
{
    initscr();
    cbreak();
    nonl();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    if (name_attach("Keyboard", NULL) == -1) {
        fprintf(stderr, "Cannot attach name!\n");
        exit(0);
    }
    if ((admin = name_locate("Input_Admin") == -1))  error_msg();
    registration();
    play_game();

    if(name_detach() == -1){
        fprintf(stderr, "Cannot detach name!\n");
        exit(0);
    }
    return 0;
}

void registration(void){
    msg.type = REGISTER_KEYBOARD;
    ksend();
    if (reply.type == FAIL){
        fprintf(stderr, "Registration failed!\n");
        exit(0);
    }
}

void play_game(void){
    msg.type = KEYBOARD_READY;
    ksend();
    if (reply.type != START){
        fprintf(stderr, "Start failed!\n");
        exit(0);
    }

    msg.type = KEYBOARD_INPUT;
    while (!end){
        msg.key = getch();
        if (msg.key != -1){
            ksend();
        }
        switch (reply.type){
            case OKAY:
                break;
            case END:
                end = 1;
                break;
            default:    break;
        }
    }
    endwin();
}

void ksend(void){
    if (Send(admin, &msg, &reply, sizeof(msg), sizeof(reply)) == -1)
        error_msg();
}

void error_msg(void){
    fprintf(stderr, "%s\n", whatsMyError());
    exit(0);
}