#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "simpl.h"
#include "message.h"
#include <time.h>

MESSAGE msg, reply;
int courid;
int game_admin, in_admin, dis_admin;
void csend(int);
void registration_human(void);
void registration_display(void);
void error_msg(void);
void play_game(void);
void display_game(void);
int end = 0;

int main(int argc, char* argv[])
{
    char name[] = "Courier?";
    name[7] = argv[1][0];
    courid = argv[1][0] - '0';
    if (name_attach(name, NULL) == -1){
        fprintf(stderr, "Cannot attach name!\n");
        exit(0);
    }
    if ((game_admin = name_locate("Game_Admin")) == -1) error_msg();
    if ((in_admin = name_locate("Input_Admin")) == -1) error_msg();
    if ((dis_admin = name_locate("Display_Admin")) == -1) error_msg();

    if (courid  == 0 || courid == 1){
        registration_human();
        play_game();
    }
    else{
        registration_display();
        display_game();
    }

    if (name_detach() == -1) error_msg();
    return 0;
}

void registration_human(void){
    msg.type = REGISTER_COURIER;
    csend(in_admin);
    if (reply.type == FAIL){
        fprintf(stderr, "Registration failed!\n");
        exit(0);
    }
    msg.type = COURIER_READY;
    csend(in_admin);

    msg.type = REGISTER_HUMAN;
    csend(game_admin);
    if (reply.type == FAIL){
        msg.type = FAIL;
        msg.humanId = reply.humanId;
        csend(in_admin);
        if (name_detach() == -1) error_msg();
        exit(0);
    }
    msg.type = INIT;
    msg.humanId = reply.humanId;
    csend(in_admin);

    msg.type = HUMAN_READY;
    msg.humanId = reply.humanId;
    csend(game_admin);
}

void registration_display(void){
    msg.type = REGISTER_COURIER;
    csend(game_admin);
    if (reply.type == FAIL){
        fprintf(stderr, "Rrgistration failed!\n");
        exit(0);
    }
}

void play_game(void){
    msg.type = START;
    msg.humanId = reply.humanId;
    csend(in_admin);
    while(!end){
        switch (reply.type){
            case HUMAN_MOVE:
                msg.type = HUMAN_MOVE;
                msg.humanId = reply.humanId;
                msg.act = reply.act;
                csend(game_admin);
                break;
            case OKAY:
                end = 1;
                break;
            case END:
                reply.type = END;
                csend(in_admin);
                break;
            case UPDATE:
                msg.type = UPDATE;
                msg.humanId = reply.humanId;
                csend(in_admin);
            default: break;
        }
    }
}

void display_game(void){
    msg.type = COURIER_READY;
    csend(game_admin);
    while(!end){
        switch (reply.type)
        {
            case DISPLAY_ARENA:
                msg.type = DISPLAY_ARENA;
                msg.arena = reply.arena;
                csend(dis_admin);
                break;
            case OKAY:
                msg.type = OKAY;
                csend(game_admin);
                break;
            case END:
                msg.type = END;
                msg.humanId = reply.humanId;    // winner
                msg.arena = reply.arena;
                csend(dis_admin);
                end = 1;
                break;
            default: break;
        }
    }
}

void error_msg(){
    fprintf(stderr, "%s\n", whatsMyError());
    exit(0);
}

void csend(int admin){
    if (Send(admin, &msg, &reply, sizeof(msg), sizeof(reply)) == -1)    error_msg();
}
