#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <simpl.h>
#include "message.h"

typedef struct input
{
    MESSAGE reply;
    struct input* next; 
} queue;

queue *head, *tail;
char *fromWhom = NULL, *fromWhom2 = NULL;
MESSAGE msg, reply, msg2, reply2;
int winner;
int end = 0;
char *painter = NULL;

void display_game(void);
void psend(void);
void push_msg(ARENA);
void pop_msg(void);
void error_msg(void);
FILE* display_file;

int main(void)
{
    display_file = fopen("display_file", "w");
    char name[] = "Display_Admin";
    if (name_attach(name, NULL) == -1){
        fprintf(stderr, "Cannot attach name!\n");
        exit(0);
    }
    display_game();

    if (name_detach() == -1){
        fprintf(stderr, "Cannot, detach name!\n");
        exit(0);
    }
    fclose(display_file);
    return 0;
}

void display_game(void){
    while (!end){
        if (Receive(&fromWhom, &msg, sizeof(msg)) == -1)    error_msg();
        switch (msg.type){
            case DISPLAY_ARENA:
                fprintf(display_file, "receive display_arena\n");
                fflush(display_file);
                reply.type = OKAY;
                psend();
                push_msg(msg.arena);
                break;
            case PAINTER_READY:
                pop_msg();
                psend();
                break;
            case END:
                end = 1;
                winner = msg.humanId; //TODO
                push_msg(msg.arena);
                reply.type = OKAY;
                psend();
                break;
            default: break;
        }
    }
}

void psend(){
    if (Reply(fromWhom, &reply, sizeof(MESSAGE)) == -1){
        fprintf(stderr, "%s\n", whatsMyError());
        exit(0);
    }
}

void push_msg(ARENA arena){
    if (head == NULL){
        head = (queue*)malloc(sizeof(queue));
        head->next = NULL;
        head->reply.arena = arena;
        head->reply.type = PAINT;
        tail = head;
    }
    else{
        tail->next = (queue*)malloc(sizeof(queue));
        tail = tail->next;
        tail->next = NULL;
        tail->reply.arena = arena;
        tail->reply.type = PAINT;
    }
}

void pop_msg(){
    while(head == NULL){
        if (Receive(&fromWhom2, &msg2, sizeof(msg2)) == -1) error_msg();
        if (msg2.type == DISPLAY_ARENA){
            push_msg(msg2.arena);
            reply2.type = OKAY;
            if(Reply(fromWhom2, &reply2,sizeof(MESSAGE)) == -1)     error_msg();
        }
    }
    reply = head->reply;
    queue* tmp = head;
    head = head->next;
    free(tmp);
}

void error_msg(void){
    fprintf(stderr, "%s\n", whatsMyError());
    exit(0);
}
