/* -------------------------
* ESTR 3106 - Assignment 1
* Name : LI Yunxiang
* Student ID : 1155092144
* Email : yxli7@link.cuhk.edu.hk
**
Failure/Success
* -----------------------*/
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <simpl.h>
#include <time.h>
#include "message.h"

int game_admin;
MESSAGE msg, reply;
TIMER_TYPE timer_type;
void mysend(void);
void registration(void);
void game(void);
int sleep_time;
void error_msg(void);
// 0: lancer, 1: hoplite, 2: mine

// FILE* time_file;

int main(int argc, char* argv[]){
    // time_file = fopen("time_file", "w");
    char name[] = "Timer?";
    name[5] = argv[1][0];
    if (name_attach(name, NULL) == -1){
        fprintf(stderr, "Cannot attach name!\n");
        exit(0);
    } 
    if ((game_admin = name_locate("Game_Admin")) == -1){
        fprintf(stderr, "Cannot locate game!\n");
        exit(0);
    }

    registration();
    game();

    if (name_detach() == -1){
        fprintf(stderr, "Cannot detach name!\n");
        exit(0);
    }
    // fclose(time_file);  
    return 0;
}

void registration(void){
    msg.type = REGISTER_TIMER;
    if (Send(game_admin, &msg, &reply, sizeof(msg), sizeof(reply)) == -1) error_msg();
    if (reply.type == FAIL){
        fprintf(stderr, "Registration failed!\n");
        exit(0);
    }
    timer_type = reply.timer_type;
}

void game(void){
    while(1){
        msg.type = TIMER_READY;
        msg.timer_type = timer_type;
        if (Send(game_admin, &msg, &reply, sizeof(msg), sizeof(reply)) == -1) error_msg();
        if (reply.type == SLEEP) {

            // fprintf(time_file, "time receives sleep");
            // fflush(time_file);

            sleep_time = reply.interval;
            usleep(sleep_time);
        }
        else    break;
    }
}

void error_msg(void){
    fprintf(stderr, "%s\n", whatsMyError());
    exit(0);
}