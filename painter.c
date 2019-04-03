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
    if (name_attach("Painter", NULL) == -1){
        fprintf(stderr, "Name attach failed!\n");
        exit(0);
    }
    
    if ((admin = name_locate("Display_Admin")) == -1)    error_msg();

    init();
    play_game();
    
    if(name_detach() == -1){
        fprintf(stderr, "Name detach Failed!\n");
        exit(0);
    }
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
            paint();
            msg.type = PAINTER_READY;
            ptsend();
        }
        else{
            //TODO: change
            end = 1;
            int winner = reply.humanId;
            if (winner == 0)    mvprintw(0, 0, "red win!");
            else    mvprintw(0, 0, "blue win!");
            refresh();
            exit(0);
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
	// red
	attron(COLOR_PAIR(2));
	mvprintw(start_y-1+22, start_x+1, "Red $: %d/%d, health: %d, unit_no: %d/%d",reply.arena.players[0].resource,MAX_RESOURCE,reply.arena.players[0].health,reply.arena.players[0].unit_no,MAX_UNIT);
	attroff(COLOR_PAIR(2));

	// blue
	attron(COLOR_PAIR(3));
	mvprintw(start_y-1+22, start_x+52, "Blue $: %d/%d, health: %d, unit_no: %d/%d",reply.arena.players[1].resource,MAX_RESOURCE,reply.arena.players[1].health,reply.arena.players[1].unit_no,MAX_UNIT);
	attroff(COLOR_PAIR(3));

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
        wattron(win, COLOR_PAIR(2) | A_BLINK);
        mvwprintw(win, y, x, symbol);
        wattroff(win,COLOR_PAIR(2) | A_BLINK);
    }
}

void paint_blue(int y, int x, char* symbol, int highlight){
    if (highlight == 0){
        wattron(win, COLOR_PAIR(3));
        mvwprintw(win, y, x, symbol);
        wattroff(win,COLOR_PAIR(3));
    }
    else{
        wattron(win, COLOR_PAIR(3) | A_BLINK);
        mvwprintw(win, y, x, symbol);
        wattroff(win,COLOR_PAIR(3) | A_BLINK);
    }
}

void create_win(){
    attron(COLOR_PAIR(2));
    win = newwin(height, width, start_y, start_x);
    wborder(win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    refresh();
    // attroff(COLOR_PAIR(2)));

    int i, j;
    for(j = start_y - 3; j <= start_y + height + 3; j++)
		for(i = start_x - 3; i <= start_x + width + 3; i++)
			mvaddch(j, i, ' ');
    refresh();
    // attroff(COLOR_PAIR(2));

    // left
    mvaddch(start_x-1, start_x-1, '+');
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
    fprintf(stderr, "%s\n", whatsMyError());
    exit(0);
}