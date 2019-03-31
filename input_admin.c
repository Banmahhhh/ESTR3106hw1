#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <simpl.h>
#include <ncurses.h>
#include "message.h"

typedef struct node{
    ACTION act;
    int humanId;
    struct node* next;
} queue;

queue *head, *tail;
char *fromWhom = NULL, *fromWhom2 = NULL;
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

int main(void){ 
    if (name_attach("Input_Admin", NULL) == -1){
        fprintf(stderr, "Attach name failed!\n");
        exit(0);
    }
    play_game();
    if (name_detach() == -1){
        fprintf(stderr, "Detach name failed\n");
        exit(0);
    }
    return 0;
}

void play_game(void){
    while(!end){
        if (Receive(&fromWhom, &msg, sizeof(msg)) == -1) error_msg();
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
                reply.humanId = reply.humanId;
                isend();
                break;
            case UPDATE:
                reply.type = HUMAN_MOVE;
                reply.humanId = head->humanId;  
                reply.act = head->act;
                pop();           
                break;
            case END:
                reply.type = OKAY;
                end_num++;
                isend();
                if (end_num == 2)   end = 1;
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
            default:
                break;
        }
    }
}

// check if the action valid. if yes, return, if no, wait for message
void pop(){
    while (head == NULL){
        if(Receive(&fromWhom2, &msg2, sizeof(MESSAGE)) == -1){
            error_msg();
        }
        if (msg2.type == KEYBOARD_INPUT){
            push(msg2.key);
            reply2.type = OKAY;
            if (Reply(fromWhom2, &reply2, sizeof(MESSAGE)) == -1)
                exit(0);
        }        
    }    
    head = head->next;
}

// change tmp_key to action
void push(int tmp_key){
    ACTION action;
    FORCE humanID;
    switch (tmp_key)
    {
        case 259: action = MOVENORTH; humanId = BLUE; break;
        case 258: action = MOVESOUTH; humanId = BLUE; break;
        case 260: action = MOVEWEST; humanId = BLUE; break;
        case 261: action = MOVEEAST; humanId = BLUE; break;
        case 106: action = PLACELANCER; humanId = BLUE; break;
        case 107: action = PLACEHOPLITE; humanId = BLUE; break;
        case 117: action = PLACEMINE; humanId = BLUE; break;
        case 105: action = PLACEWALL; humanId = BLUE; break;
        case 119: action = MOVENORTH; humanId = RED; break;
        case 115: action = MOVESOUTH; humanId = RED; break;
        case 97: action = MOVEWEST; humanId = RED; break;
        case 100: action = MOVEEAST; humanId = RED; break;
        case 102: action = PLACELANCER; humanId = RED; break;
        case 103: action = PLACEHOPLITE; humanId = RED; break;
        case 114: action = PLACEMINE; humanId = RED; break;
        case 116: action = PLACEWALL; humanId = BLUE; break;
        default: break;
    }

    if (head == NULL){
        head = (queue*)malloc(sizeof(queue));
        tail = head;
        head->next = NULL;
        head->act = action;
        head->humanId = humanID;
    }
    else{
        tail->next = (queue*)malloc(sizeof(queue));
        tail->next->act = action;
        tail->next->humanId = humanID;
        tail = tail->next;
        tail->next = NULL;
    }
}

void isend(void){
    if (Reply(fromWhom, &reply, sizeof(MESSAGE)) == -1)
        exit(0);
}

void error_msg(void){
    fprintf(stderr, "%s\n", whatsMyError());
    exit(0);
}