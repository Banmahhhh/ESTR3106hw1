/* -------------------------
* ESTR 3106 - Assignment 1
* Name : LI Yunxiang
* Student ID : 1155092144
* Email : yxli7@link.cuhk.edu.hk
**
Failure/Success
* -----------------------*/
#ifndef __MESSAGE_H
#define __MESSAGE_H
// version: 0.3 

/* Maximum number of keys to read in from the keyboard at once */
#define MAX_WIDTH	   100
#define MAX_HEIGHT	   20
#define MAX_UNIT	   100
#define MAX_RESOURCE   100000

/* Time interval in microseconds */
#define LANCER_INTERVAL 200000 
#define HOPLITE_INTERVAL 300000
#define MINE_INTERVAL 1000000

/* Maximum Damage caused by robots */
#define LANCER_MD 15
#define HOPLITE_MD 10
#define LANCER_BD 15
#define HOPLITE_BD 20


/* Message Token */
typedef enum { 
	INIT, FAIL, END, OKAY,	// 3
	REGISTER_TIMER, TIMER_READY, SLEEP,	// 6
	REGISTER_COURIER, COURIER_READY, DISPLAY_ARENA, // 9
	REGISTER_KEYBOARD, KEYBOARD_READY, // 11
	REGISTER_HUMAN, HUMAN_READY, HUMAN_MOVE, UPDATE, // 15
	PAINTER_READY, PAINT, KEYBOARD_INPUT, START // 19
} MESSAGE_TYPE;

/* Type of Units */ 
typedef enum { 
	MINE=1, HOPLITE=2, LANCER=3, WALL=4, 
} UNIT_TYPE;

/* Type of Timers */
typedef enum {
	LANCER_TIMER = 0, HOPLITE_TIMER=1, MINE_TIMER=2
} TIMER_TYPE;

/* Force */ 
typedef enum {
	RED=0, BLUE=1
} FORCE;

typedef enum {
	NOACTION, MOVEEAST, MOVESOUTH, MOVEWEST, MOVENORTH, PLACELANCER, PLACEHOPLITE, PLACEMINE, PLACEWALL
} ACTION;

/* coordinate */ 
typedef struct {
	int x,y; 
} COORDINATE;

/* player */
typedef struct {
	COORDINATE pos; /* position of focus */ 
	FORCE force; 	/* blue or red force */
	int health; 	/* health value of the base */ 
	int resource; 	/* number of resources */
	int unit_no;    /* number of units */ 
} PLAYER;

/* Unit */ 
typedef struct {
	int active; /* whether this is an active unit */
	int highlight; /* whether this unit should be highlighted */
	COORDINATE pos; 
	FORCE force; 
	UNIT_TYPE unit_type;
	int health; 
} UNIT; 

/* Arena */ 
typedef struct {
	UNIT units[2*MAX_UNIT];	/* unit array */  
	PLAYER players[2];		/* 0: red player, 1: blue player */ 
} ARENA;

/* Message Format */
typedef struct { 
	MESSAGE_TYPE type;  	/* Message Type */
	TIMER_TYPE timer_type;	/* Timer Type */ 
	int interval;       	/* Sleep Interval */
	ACTION act; 			/* action taken */
	ARENA arena;			/* current game state */
	int key; 				/* key pressed */
	int humanId;        	/* Courier ID*/
} MESSAGE;

#endif
