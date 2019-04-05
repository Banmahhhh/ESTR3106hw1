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
#include <simpl.h>
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include "message.h"

int admin;
MESSAGE msg, reply;
int end = 0;
int start_x, start_y;
int height, width;
WINDOW *win;
ARENA arena;
// FILE* banma;

void init(void);
void error_msg(void);
void ptsend(void);
void paint(void);
void play_game(void);
void registration(void);
void create_win();

void paint_red(int y, int x, char* symbol, int highlight);
void paint_blue(int y, int x, char* symbol, int highlight);

int main(void){
    // banma = fopen("painter_file", "w");
    if (name_attach("Painter", NULL) == -1){
        fprintf(stderr, "Name attach failed!\n");
        exit(0);
    }
    
    if ((admin = name_locate("Display_Admin")) == -1)    error_msg();

    init();
    play_game();
    
    // fprintf(banma, "painter end\n");
    // fflush(banma);
    if(name_detach() == -1){
        fprintf(stderr, "Name detach Failed!\n");
        exit(0);
    }
    // fclose(banma);
    return 0;
}

void init(void){
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    start_color();
    // curs_set(0);
    noecho();

    init_pair(1, COLOR_GREEN, COLOR_BLACK); // title
    init_pair(2, COLOR_RED, COLOR_YELLOW);  // left
    init_pair(3, COLOR_BLUE, COLOR_YELLOW); // right
    init_pair(4, COLOR_YELLOW, COLOR_YELLOW);

    height = MAX_HEIGHT;    width = MAX_WIDTH;
    start_x = (COLS - width)/2;     start_y = (LINES - height)/2;

    create_win();
}

void play_game(void){
    msg.type = PAINTER_READY;
    ptsend();

    while(!end){
        // msg.type = PAINTER_READY;
        // ptsend();
        if (reply.type == PAINT){
            // arena = reply.
            paint();
            msg.type = PAINTER_READY;
            ptsend();
        }
        else{
            
            end = 1;
            int winner = reply.humanId;
            if (winner == 0) {
                wattron(win,A_REVERSE|COLOR_PAIR(1));
                mvwprintw(win,18,width/2-8,"red win!");
                wattroff(win,A_REVERSE|COLOR_PAIR(1));
            }
            else{
                wattron(win,A_REVERSE|COLOR_PAIR(1));
                mvwprintw(win,18,width/2-8,"blue win!");
                wattroff(win,A_REVERSE|COLOR_PAIR(1));
            }
            refresh();
            wrefresh(win);
            sleep(2);
            // exit(0);
        }
    }
    endwin();
}

void paint(void){
    werase(win);
    // wrefresh(win);
    refresh();
    create_win();
    wrefresh(win);
    int i;
    // ARENA arena = reply.arena;
    for (i=0; i<2*MAX_UNIT; i++){
        if(reply.arena.units[i].active == 1){
            switch (reply.arena.units[i].unit_type){
                case LANCER:
                    if(reply.arena.units[i].force == RED){
                        paint_red(reply.arena.units[i].pos.y, reply.arena.units[i].pos.x, "&/", reply.arena.units[i].highlight);
                    }
                    else{
                        paint_blue(reply.arena.units[i].pos.y, reply.arena.units[i].pos.x-1, "\\&", reply.arena.units[i].highlight);
                    }
                    break;
                case HOPLITE:
                    if(reply.arena.units[i].force == RED){
                        paint_red(reply.arena.units[i].pos.y, reply.arena.units[i].pos.x, "@]", reply.arena.units[i].highlight);
                    }
                    else{
                        paint_blue(reply.arena.units[i].pos.y, reply.arena.units[i].pos.x-1, "[@", reply.arena.units[i].highlight);
                    }
                    break;
                case MINE:
                    if(reply.arena.units[i].force == RED){
                        paint_red(reply.arena.units[i].pos.y, reply.arena.units[i].pos.x-1, "/O\\", reply.arena.units[i].highlight);
                    }
                    else{
                        paint_blue(reply.arena.units[i].pos.y, reply.arena.units[i].pos.x-1, "/O\\", reply.arena.units[i].highlight);
                    }
                    break;
                case WALL:
                    if(reply.arena.units[i].force == RED){
                        paint_red(reply.arena.units[i].pos.y, reply.arena.units[i].pos.x, "{", reply.arena.units[i].highlight);
                    }
                    else{
                        paint_blue(reply.arena.units[i].pos.y, reply.arena.units[i].pos.x, "}", reply.arena.units[i].highlight);
                    }
                    break;
                default:    break;
            }
        }
    }
    
    // markers
    wattron(win,A_REVERSE|COLOR_PAIR(2));
	mvwprintw(win,reply.arena.players[0].pos.y,reply.arena.players[0].pos.x," ");
	wattroff(win,A_REVERSE|COLOR_PAIR(2));

    wattron(win,A_REVERSE|COLOR_PAIR(3));
	mvwprintw(win,reply.arena.players[1].pos.y,reply.arena.players[1].pos.x," ");
	wattroff(win,A_REVERSE|COLOR_PAIR(3));


    // bottom
    mvwprintw(win,19,99," ");

    mvprintw(start_y+21, start_x+1, "Red $: %d/100000, health: %4d, unit_no: %d/100", reply.arena.players[0].resource, reply.arena.players[0].health, reply.arena.players[0].unit_no);
    mvchgat(start_y+21, start_x+1, width / 2 + 1, A_NORMAL, 2, NULL);
    mvprintw(start_y+21, start_x+52, "Blue $: %d/100000, health: %4d, unit_no: %d/100", reply.arena.players[1].resource, reply.arena.players[1].health, reply.arena.players[1].unit_no);
    mvchgat(start_y+21, start_x+52, width / 2 + 1, A_NORMAL, 3, NULL);

	refresh();
	wrefresh(win);

}

void paint_red(int y, int x, char* symbol, int highlight){
    if (highlight == 0){
        wattron(win, COLOR_PAIR(2));
        mvwprintw(win, y, x, symbol);
        wattroff(win,COLOR_PAIR(2));
    }
    else{
        wattron(win, COLOR_PAIR(2) | A_BOLD);
        mvwprintw(win, y, x, symbol);
        wattroff(win,COLOR_PAIR(2) | A_BOLD);
    }
}

void paint_blue(int y, int x, char* symbol, int highlight){
    if (highlight == 0){
        wattron(win, COLOR_PAIR(3));
        mvwprintw(win, y, x, symbol);
        wattroff(win,COLOR_PAIR(3));
    }
    else{
        wattron(win, COLOR_PAIR(3) | A_BOLD);
        mvwprintw(win, y, x, symbol);
        wattroff(win,COLOR_PAIR(3) | A_BOLD);
    }
}

void create_win(){
    win = newwin(height, width, start_y, start_x);
    wborder(win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    refresh();

    attron(COLOR_PAIR(4));
    int i, j;
    for(j = start_y - 3; j <= start_y + height + 3; j++)
		for(i = start_x - 3; i <= start_x + width + 3; i++)
			mvaddch(j, i, ' ');
    refresh();
    attroff(COLOR_PAIR(4));

    // left
    attron(COLOR_PAIR(2));
    mvaddch(start_y-1, start_x-1, '+');
    mvaddch(start_y + height, start_x-1, '+');
	mvhline(start_y-1, start_x, '-', width/2);
	mvhline(start_y + height, start_x, '-', width/2);
	mvvline(start_y, start_x-1, '|', height);
    refresh();
    attroff(COLOR_PAIR(2));

    // right
    attron(COLOR_PAIR(3));
    mvaddch(start_y-1, start_x + width, '+');
	mvaddch(start_y + height, start_x + width, '+');
	mvhline(start_y-1, start_x-1 + width/2 + 1, '-', width/2);
	mvhline(start_y + height, start_x + width/2, '-', width/2);
	mvvline(start_y, start_x + width, '|', height);
    refresh();
    attroff(COLOR_PAIR(3));

    // title
    attron(COLOR_PAIR(1));
    mvaddstr(start_y - 2, start_x+38, "Tower Defense and Attack");
    refresh();
    attroff(COLOR_PAIR(1));

    // background
    wbkgd(win, COLOR_PAIR(2));
    wrefresh(win);
}

void ptsend(void){
    if (Send(admin, &msg, &reply, sizeof(msg), sizeof(reply)) == -1){
        error_msg();
    }
}

void error_msg(void){
    fprintf(stderr, "painter, %s\n", whatsMyError());
    exit(0);
}