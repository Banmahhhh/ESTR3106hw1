#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <simpl.h>
#include <ncurses.h>
#include <time.h>
#include "message.h"
#include <string.h>


typedef struct input{
    MESSAGE reply;
    struct input* next;
} queue;

queue *head, *tail;
MESSAGE msg, reply, reply2;
int timer_num = 0, courier_num = 0, human_num = 0;
int end = 0;
ARENA arena;
char *fromWhom = NULL, *courier = NULL;
FILE *zebra;

// '/O\': 201 index 201;    {: index;      }: index;    
// &/: index 202;   \&: 202 index;      @]: index 203;   [@: 203 index
int banma[22][103];       // the board, store all units
int left_bound[2] = {0,50};
int right_bound[2] = {49, 99};
int mine_num[2] = {0,0};    // the number of mines of two forces
int unit_num[2] = {0,0};    // the number of units
int lancer_l_bound[2] = {1, 51};
int lancer_r_bound[2] = {48, 100}; // to replace a lancer, the marker should be within this bound

void registration(void);
void play_game(void);
void g_send(void);
void error_msg(void);
void init(void);
void push(void);
void pop(void);
void game_end(void);
void move_l_h(int melee_damage, int base_damage, int type_num, UNIT_TYPE type);     // move lancer and hoptile when timer sends message
void create_unit(int index, int x, int y, FORCE humanId, UNIT_TYPE type, int health, int reduce_resource);

int main(void){
    zebra = fopen("zebra", "w");
    if ((name_attach("Game_Admin", NULL)) == -1){
        fprintf(stderr, "Attach name failed!\n");
        exit(0);
    }
    init();  
    registration();
    fprintf(zebra, "registration finish\n");
    fflush(zebra);
    srand((unsigned int)(time(NULL)));
    play_game();
    game_end();
    if (name_detach() == -1){
        fprintf(stderr, "Detach name failed\n");
        exit(0);
    }
    fclose(zebra);
    return 0;
}

void registration(void){
    char* courier2 = NULL, *human[2] = {NULL, NULL};

    while(timer_num != 3 || courier_num != 1 || human_num != 2){
        if (Receive(&fromWhom, &msg, sizeof(msg)) == -1)    error_msg();
        fprintf(zebra, "registration receive message, receive message %s, %d\n", fromWhom, msg.type);
        fflush(zebra);
        ;
        switch (msg.type){
            case REGISTER_TIMER:
                if (timer_num == 3){
                    reply.type = FAIL;
                    g_send();
                }
                else{
                    reply.type = INIT;
                    reply.timer_type = timer_num;
                    timer_num++;
                    g_send();                    
                }
                break;
            case TIMER_READY:
                if (msg.timer_type == LANCER_TIMER)    reply.interval = LANCER_INTERVAL;
                else if (msg.timer_type == HOPLITE_TIMER)   reply.interval = HOPLITE_INTERVAL;
                else if (msg.timer_type == MINE_TIMER)   reply.interval = MINE_INTERVAL;
                reply.type = SLEEP;
                reply.timer_type = msg.timer_type;
                g_send();
                break;
            case REGISTER_HUMAN:
                if (human_num == 2){
                    reply.type = FAIL;
                    g_send();
                }
                else{
                    reply.type = INIT;
                    reply.humanId = fromWhom[7]-'0'; //human_tmp++;
                    fprintf(zebra, "human id is %d\n", reply.humanId);
                    fflush(zebra);
                    g_send();
                }
                break;
            case HUMAN_READY:
                // reply.type = START;     //game start!
                human[human_num] = fromWhom;
                human_num++;    // human registration finish
                // g_send();
                break;
            case REGISTER_COURIER:      // game_admin and courier 2
                if (courier_num == 1){
                    reply.type = FAIL;
                    g_send();
                }
                else{
                    reply.type = INIT;
                    courier_num++;
                    g_send();
                }
                break;
            case COURIER_READY:     // Before start, display
                courier2 = fromWhom;
                // g_send();
                break;
            default:
                break;
        }
    }
    reply.type = DISPLAY_ARENA; reply.arena = arena;
    if(Reply(courier2, &reply, sizeof(MESSAGE)) == -1)   error_msg();

    reply.type = START; 
    reply.humanId = 0;
    if(Reply(human[0], &reply, sizeof(MESSAGE)) == -1)   error_msg();
    reply.humanId = 1;
    if(Reply(human[1], &reply, sizeof(MESSAGE)) == -1)   error_msg();
}

void play_game(void){
    push();  
    while(arena.players[0].health > 0 && arena.players[1].health > 0){
        if (Receive(&fromWhom, &msg, sizeof(MESSAGE)) == -1)  error_msg();
        fprintf(zebra, "in play_game receive message %s, %d\n", fromWhom, msg.type);
        fflush(zebra);
        switch (msg.type){
            case TIMER_READY:
                if (msg.timer_type == LANCER_TIMER)    reply.interval = LANCER_INTERVAL;
                else if (msg.timer_type == HOPLITE_TIMER)   reply.interval = HOPLITE_INTERVAL;
                else if (msg.timer_type == MINE_TIMER)   reply.interval = MINE_INTERVAL;
                reply.type = SLEEP;
                reply.timer_type = msg.timer_type;
                g_send();

                switch (msg.timer_type){
                    case LANCER_TIMER:
                        move_l_h(LANCER_MD, LANCER_BD, 202, LANCER);
                        break;
                    case HOPLITE_TIMER:
                        move_l_h(HOPLITE_MD, LANCER_BD, 203, HOPLITE);
                        break;
                    case MINE_TIMER:
                        arena.players[0].resource += (10*mine_num[0]);
                        arena.players[1].resource += (10*mine_num[1]);
                        break;
                    default:
                        break;
                }
                push();
                // fprintf(zebra, "receive timer and update\n");
                // fflush(zebra);
                break;
            case HUMAN_MOVE:  ;  // update tail->reply.arena, and push
                fprintf(zebra, "in play_game receive human_move %s, %d\n", fromWhom, msg.act);
                fflush(zebra);
                int humanId = msg.humanId;
                // fprintf(zebra, "receive human move, id is %d\n", humanId);
                // fflush(zebra);
                ACTION act = msg.act;
                reply.type = UPDATE; 
                reply.humanId = msg.humanId;  
                g_send();

                int x = arena.players[humanId].pos.x;     // not from (0,0)
                int y = arena.players[humanId].pos.y;
                int i;               
                // according to the act, update the global arena, and tail->reply->arena = new arena
                switch (act){
                    case NOACTION:
                        break;
                    case MOVEEAST:
                        if (x < right_bound[humanId]) 
                            arena.players[humanId].pos.x ++;
                        break;
                    case MOVEWEST:
                        if (x > left_bound[humanId]) 
                            arena.players[humanId].pos.x --;
                        break;
                    case MOVENORTH:
                        if (y > 0) 
                            arena.players[humanId].pos.y --;
                        break;
                    case MOVESOUTH:
                        if (y < 19) 
                            arena.players[humanId].pos.y ++;
                        break;
                    case PLACELANCER: 
                        if (arena.players[humanId].unit_no >= MAX_UNIT || arena.players[humanId].resource < 20)  break; 
                        int u;      // store the empty slot of units array
                        if (x<=lancer_r_bound[humanId] && x>=lancer_l_bound[humanId] && banma[y][x] == -1 && banma[y][x-2*humanId+1] == -1){    // no overlap
                            for (i=0; i<2*MAX_UNIT; i++){   // find the empty entry in arena.units
                                if (arena.units[i].active == 0) {
                                u=i;    break; }
                            }
                            banma[y][x] = u;   banma[y][x-2*humanId+1] = 202;
                            create_unit(u, x, y, humanId, LANCER, 50, 20);
                            arena.players[humanId].unit_no++;
                        }
                        break;
                    case PLACEHOPLITE:
                        if (arena.players[humanId].unit_no >= MAX_UNIT || arena.players[humanId].resource < 30)  break;    
                        if (x<=lancer_r_bound[humanId] && x>=lancer_l_bound[humanId] && banma[y][x] == -1 && banma[y][x-2*humanId+1] == -1){    // no overlap
                            for (i=0; i<2*MAX_UNIT; i++){   // find the empty entry in arena.units
                                if (arena.units[i].active == 0) {
                                u=i;    break;}
                            }
                            banma[y][x] = u;   banma[y][x-2*humanId+1] = 203;
                            create_unit(u, x, y, humanId, HOPLITE, 100, 30);
                            arena.players[humanId].unit_no++;
                        }
                        break;
                    case PLACEMINE:
                        //remove mine
                        if (banma[y][x] != -1 && arena.units[banma[y][x]].unit_type == MINE){  // && banma[y][x+1] == 201 && banma[y][x-1] == 201){
                            arena.units[banma[y][x]].active = 0;
                            banma[y][x] = -1;
                            banma[y][x+1] = -1;
                            banma[y][x-1] = -1;
                            arena.players[humanId].resource += 100;
                            arena.players[humanId].unit_no--;
                            mine_num[humanId]--;
                            break;
                        }
                        // place mine
                        if (arena.players[humanId].unit_no >= MAX_UNIT || arena.players[humanId].resource < 200)  break;     
                        if (x<right_bound[humanId] && x>left_bound[humanId] && banma[y][x] == -1 && banma[y][x+1] == -1 && banma[y][x-1] == -1){
                            for (i=0; i<2*MAX_UNIT; i++){   // find the empty entry in arena.units
                                if (arena.units[i].active == 0) {
                                u=i;    break;}
                            }
                            banma[y][x] = u;   banma[y][x+1] = 201;     banma[y][x-1] = 201;
                            create_unit(u, x, y, humanId, MINE, 200, 200);
                            arena.players[humanId].unit_no++;
                            mine_num[humanId]++;
                        }                       
                        break;
                    case PLACEWALL:
                        //remove wall
                        if (banma[y][x] != -1 && arena.units[banma[y][x]].unit_type == WALL){
                            arena.units[banma[y][x]].active = 0;
                            banma[y][x] = -1;
                            arena.players[humanId].resource += 10;
                            arena.players[humanId].unit_no--;
                            break;
                        } 
                        // place mine
                        if (arena.players[humanId].unit_no >= MAX_UNIT || arena.players[humanId].resource < 20)  break;     
                        if (banma[y][x] == -1){
                            for (i=0; i<2*MAX_UNIT; i++){   // find the empty entry in arena.units
                                if (arena.units[i].active == 0){
                                u=i;    break; }
                            }
                            banma[y][x] = u;
                            create_unit(u, x, y, humanId, WALL, 100, 20);
                            arena.players[humanId].unit_no++;
                        }
                        break;
                    default:
                        break;
                }
                // tail->reply.arena = arena;
                push();
                break;
            case OKAY:
                if (head != NULL){
                    reply = head->reply;
                    pop();
                    g_send();
                } 
                else{
                    courier = fromWhom;     // reply when a key comes
                    // g_send();
                }  
            default:
                break;
        }
    }   
}

void move_l_h(int melee_damage, int base_damage, int type_num, UNIT_TYPE type){
    int i, j;
    for (i=0; i<=19; i++){  // scan red lancers, from right to left
        for (j=100; j>1; j--){
            if (banma[i][j] == type_num && arena.units[banma[i][j-1]].unit_type == type){
                if (j == 99){  // attack the opponent
                    arena.players[1].health -= (rand()%base_damage);
                    arena.units[banma[i][j-1]].active = 0;
                    banma[i][j] = -1;    banma[i][j-1] = -1;
                    arena.players[0].unit_no--;
                }
                else if (banma[i][j+1] == -1){  // move forward
                    arena.units[banma[i][j-1]].pos.x++;
                    banma[i][j+1] = type_num;    banma[i][j] = banma[i][j-1];    banma[i][j-1] = -1;
                }
                else if (arena.units[banma[i][j+1]].unit_type == WALL && arena.units[banma[i][j+1]].force == BLUE){ // blue wall
                    arena.units[banma[i][j+1]].highlight = (arena.units[banma[i][j+1]].highlight+1)%2;
                    arena.units[banma[i][j+1]].health -= (rand()%melee_damage);
                    if (arena.units[banma[i][j+1]].health <= 0){
                        arena.units[banma[i][j+1]].active = 0;
                        arena.players[1].unit_no--;
                        banma[i][j+1] = -1;
                        arena.units[banma[i][j-1]].highlight = 0;
                    }
                }
                else if (banma[i][j+1] == 201 && arena.units[banma[i][j+2]].force == BLUE){ // blue mine
                    arena.units[banma[i][j+2]].highlight = (arena.units[banma[i][j+2]].highlight+1)%2;
                    arena.units[banma[i][j+2]].health -= (rand()%melee_damage);
                    if (arena.units[banma[i][j+2]].health <= 0){
                        arena.units[banma[i][j+2]].active = 0;
                        arena.players[1].unit_no--;
                        banma[i][j+2] = -1; banma[i][j+1] = -1; banma[i][j+3] = -1;
                        mine_num[1]--;
                        arena.units[banma[i][j-1]].highlight = 0;
                    }
                }
                else if ((banma[i][j+1] == 202 || banma[i][j+1] == 203) && arena.units[banma[i][j+2]].force == BLUE){ // blue lancer or hoplite
                    arena.units[banma[i][j+2]].highlight = (arena.units[banma[i][j+2]].highlight+1)%2;
                    arena.units[banma[i][j+2]].health -= (rand()%melee_damage);
                    if (arena.units[banma[i][j+2]].health <= 0){
                        arena.units[banma[i][j+2]].active = 0;
                        arena.players[1].unit_no--;
                        banma[i][j+2] = -1; banma[i][j+1] = -1;
                        arena.units[banma[i][j-1]].highlight = 0;
                    }
                }
            }
        }
    }
    for (i=0; i<=19; i++){  // scan blue lancers, from left to right
        for (j=0; j<100; j++){
            if (banma[i][j] == type_num && arena.units[banma[i][j+1]].unit_type == type){
                if (j == 0){
                    arena.players[0].health -= (rand()%base_damage);
                    arena.units[banma[i][j+1]].active = 0;
                    banma[i][j] = -1;    banma[i][j+1] = -1;
                    arena.players[1].unit_no--;
                }
                else if (banma[i][j-1] == -1.){   // nothing forward, move
                    arena.units[banma[i][j+1]].pos.x--;
                    banma[i][j-1] = type_num;    banma[i][j] = banma[i][j+1];    banma[i][j+1] = -1; 
                }
                else if (arena.units[banma[i][j-1]].unit_type == WALL && arena.units[banma[i][j-1]].force == RED){ // red wall
                    arena.units[banma[i][j-1]].highlight = (arena.units[banma[i][j-1]].highlight+1)%2;
                    arena.units[banma[i][j-1]].health -= (rand()%melee_damage);
                    if (arena.units[banma[i][j-1]].health <= 0){
                        arena.units[banma[i][j-1]].active = 0;
                        arena.players[0].unit_no--;
                        banma[i][j-1] = -1;
                        arena.units[banma[i][j+1]].highlight = 0;
                    }
                }
                else if (banma[i][j-1] == 201 && arena.units[banma[i][j-2]].force == RED){ // red mine
                    arena.units[banma[i][j-2]].highlight = (arena.units[banma[i][j-2]].highlight+1)%2;
                    arena.units[banma[i][j-2]].health -= (rand()%melee_damage);
                    if (arena.units[banma[i][j-2]].health <= 0){
                        arena.units[banma[i][j-2]].active = 0;
                        arena.players[0].unit_no--;
                        banma[i][j-2] = -1; banma[i][j-1] = -1; banma[i][j-3] = -1;
                        mine_num[0]--;
                        arena.units[banma[i][j+1]].highlight = 0;
                    }
                }
                else if ((banma[i][j-1] == 202 || banma[i][j-1] == 203) && arena.units[banma[i][j-2]].force == RED){ // red lancer or hoplite
                    arena.units[banma[i][j-2]].highlight = (arena.units[banma[i][j-2]].highlight+1)%2;
                    arena.units[banma[i][j-2]].health -= (rand()%melee_damage);
                    if (arena.units[banma[i][j-2]].health <= 0){
                        arena.units[banma[i][j-2]].active = 0;
                        arena.players[0].unit_no--;
                        banma[i][j-2] = -1; banma[i][j-1] = -1;
                        arena.units[banma[i][j+1]].highlight = 0;
                    }
                }
            }
        }
    }
}

void game_end(void){
    int i;
    reply.type = END;
    reply.arena = arena;
    if (arena.players[0].health > arena.players[1].health)  reply.humanId = 0;
    else    reply.humanId = 1;
    
    for (i=0; i<6; i++){
        if (Receive(&fromWhom, &msg, sizeof(msg)))  error_msg();       
        g_send();
    }
}

void g_send(void){
    if (Reply(fromWhom, &reply, sizeof(MESSAGE)) == -1)
        error_msg();
}

void error_msg(void){
    fprintf(stderr, "%s\n", whatsMyError());
    exit(0);
}

void push(void){
    if (head == NULL){
        head = (queue*)malloc(sizeof(queue));
        head->next = NULL;
        head->reply.arena = arena;
        head->reply.type = DISPLAY_ARENA;
        tail = head;
    }
    else{
        tail->next = (queue*)malloc(sizeof(queue));
        // tail->next->reply = tail->reply;
        tail = tail->next;
        tail->next = NULL; // pass value to next one
        tail->reply.arena = arena;
        tail->reply.type = DISPLAY_ARENA;
    }

    // reply to courier2
    if (courier != NULL){
        reply2 = head->reply;
        pop();
        if (Reply(courier, &reply2, sizeof(MESSAGE)) == -1)
            error_msg();
        courier = NULL;
    }
}

void pop(void){
    if (head == NULL) return;
    queue* tmp = head;
    head = head->next;
    free(tmp);
}

void init(void){
    // arena = (ARENA)malloc(sizeof(ARENA));
    arena.players[0].pos.x = 25;
    arena.players[0].pos.y = 9;
    arena.players[0].force = RED;
    arena.players[0].health = 1000;
    arena.players[0].resource = 200;
    arena.players[0].unit_no = 0;

    arena.players[1].pos.x = 75;
    arena.players[1].pos.y = 9;
    arena.players[1].force = BLUE;
    arena.players[1].health = 1000;
    arena.players[1].resource = 200;
    arena.players[1].unit_no = 0;

    int i;
    for (i=0; i<2*MAX_UNIT; i++){
        arena.units[i].active = 0;
        arena.units[i].highlight = 0;
    }

    for (i=0; i<22; i++){   // initialize banma with -1
        memset(banma[i], -1, 102*sizeof(int));
    }
}

void create_unit(int index, int x, int y, FORCE humanId, UNIT_TYPE type, int health, int reduce_resource){
    arena.units[index].active = 1;
    arena.units[index].highlight = 0;
    arena.units[index].pos.x = x;
    arena.units[index].pos.y = y;
    arena.units[index].force = humanId;
    arena.units[index].unit_type = type;
    arena.units[index].health = health;
    arena.players[humanId].resource -= reduce_resource;
}
                            