#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <simpl.h>
#include <ncurses.h>
#include "message.h"
#include <time.h>

typedef struct node{
    ACTION act;     
    int humanId; 
    struct node* next;
} queue;

queue *head = NULL, *tail = NULL;
char *fromWhom = NULL, *fromWhom2 = NULL;
char *couriers[2];
MESSAGE msg, reply, msg2, reply2;
int end = 0;
int end_num = 0;
int courier_num = 0;
int tmp_keyboard_num = 0;
int humanId;

void play_game(void);
void error_msg(void);
void push(int);
void pop(void);
void isend(void);
void check_and_reply(void);
FILE* banma;

int main(void){ 
    couriers[0] = NULL;
    couriers[1] = NULL;
    banma = fopen("input_file", "w"); 
    if (name_attach("Input_Admin", NULL) == -1){
        fprintf(stderr, "Attach name failed!\n");
        exit(0);
    }
    play_game();
    if (name_detach() == -1){
        fprintf(stderr, "Detach name failed\n");
        exit(0);
    }
    fclose(banma);
    return 0;
}

void play_game(void){
    while(!end){
        if (Receive(&fromWhom, &msg, sizeof(msg)) == -1) error_msg(); 
        // fprintf(banma, "input_admin receives message %d\n", msg.type);
        // fflush(banma);
        switch (msg.type){
            case REGISTER_COURIER:
                if (courier_num == 2){
                    reply.type = FAIL;
                    isend();
                }
                else{
                    reply.type = INIT;
                    courier_num++;
                    isend();
                }
                break;
            case COURIER_READY:
                reply.type = REGISTER_HUMAN;
                isend();
                break;
            case INIT:
                reply.type = HUMAN_READY;
                reply.humanId = msg.humanId;
                isend();
                break;
            case FAIL:
                break;
            case START:
                reply.type = HUMAN_MOVE;
                reply.act = NOACTION;
                reply.humanId = msg.humanId;
                // reply.humanId = 0;
                isend(); 
                break;
            case UPDATE:
                // fprintf(banma, "receives update from %s %d\n", fromWhom, msg.humanId);
                // fflush(banma);
                couriers[msg.humanId] = fromWhom; 
                check_and_reply();   
                break;
            case END:
                reply.type = OKAY;
                // end_num++;
                isend();
                // if (end_num == 2)   
                end = 1;
                break;
            case REGISTER_KEYBOARD: //register tmp_keyboard after couriers
                if (tmp_keyboard_num){
                    reply.type = FAIL;
                    isend();
                }
                else{
                    reply.type = INIT;
                    tmp_keyboard_num ++;
                    isend();
                }
                break;
            case KEYBOARD_READY: 
                reply.type = START;
                isend();
                break;
            case KEYBOARD_INPUT:
                reply.type = OKAY;
                push(msg.key);
                isend();                   
                break;
            default: break;
        }
    }
}

// check if the action valid. if yes, return, if no, wait for message from keyboard
void pop(){     // update reply and pop
    // while (head == NULL){
    //     if(Receive(&fromWhom2, &msg2, sizeof(msg2)) == -1) error_msg();        
    //     if (msg2.type == KEYBOARD_INPUT){                                           
    //         push(msg2.key);
    //         reply2.type = OKAY;
    //         if (Reply(fromWhom2, &reply2, sizeof(MESSAGE)) == -1)
    //             exit(0);
    //     }        
    // }    
    // if (head == NULL)   return;
    queue* tmp = head; head = head->next; free(tmp);  
}

// change tmp_key to action
void push(int tmp_key){
    ACTION action;
    int humanId;
    switch (tmp_key)
    {
        case 259: action = MOVENORTH; humanId = 1; break;
        case 258: action = MOVESOUTH; humanId = 1; break;
        case 260: action = MOVEWEST; humanId = 1; break;
        case 261: action = MOVEEAST; humanId = 1; break;
        case 106: action = PLACELANCER; humanId = 1; break;
        case 107: action = PLACEHOPLITE; humanId = 1; break;
        case 117: action = PLACEMINE; humanId = 1; break;
        case 105: action = PLACEWALL; humanId = 1; break;
        case 119: action = MOVENORTH; humanId = 0; break;
        case 115: action = MOVESOUTH; humanId = 0; break;
        case 97: action = MOVEWEST; humanId = 0; break;
        case 100: action = MOVEEAST; humanId = 0; break;
        case 102: action = PLACELANCER; humanId = 0; break;
        case 103: action = PLACEHOPLITE; humanId = 0; break;
        case 114: action = PLACEMINE; humanId = 0; break;
        case 116: action = PLACEWALL; humanId = 0; break;
        default: break;
    }

    if (head == NULL){
        head = (queue*)malloc(sizeof(queue));
        tail = head;
        head->next = NULL;
        head->act = action;
        head->humanId = humanId;
    }
    else{
        tail->next = (queue*)malloc(sizeof(queue));
        tail->next->act = action;
        tail->next->humanId = humanId;
        tail = tail->next;
        tail->next = NULL;
    }
    // fprintf(banma, "finish push %d\n", tail->act);
    // fflush(banma);
    check_and_reply();
}

void check_and_reply(void){
    if (head != NULL && couriers[head->humanId] != NULL){
        // fprintf(banma, "1. reply %d act is %d\n", head->humanId, head->act);
        // fflush(banma);
        reply.type = HUMAN_MOVE;
        reply.humanId = head->humanId;  
        reply.act = head->act;
        if (Reply(couriers[head->humanId], &reply, sizeof(MESSAGE)) == -1) error_msg();
        couriers[head->humanId] = NULL;
        pop();
    }
    if (head != NULL && couriers[head->humanId] != NULL){
        // fprintf(banma, "2. reply %d\n", head->humanId);
        // fflush(banma);
        reply.type = HUMAN_MOVE;
        reply.humanId = head->humanId;  
        reply.act = head->act;
        if (Reply(couriers[head->humanId], &reply, sizeof(MESSAGE)) == -1) error_msg();
        couriers[head->humanId] = NULL;
        pop();
    }
}

void isend(void){
    if (Reply(fromWhom, &reply, sizeof(MESSAGE)) == -1)
        error_msg();
}

void error_msg(void){
    fprintf(stderr, "%s\n", whatsMyError());
    exit(0);
}