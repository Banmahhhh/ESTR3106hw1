CC=gcc
OS=_THISIS_LINUX
VER=_SIMPLIPC
SIMPL_DIR=${SIMPL_HOME}/simplipc
INCL_DIR=${SIMPL_DIR}/include
CDFLAGS=-c -Wall -I ${INCL_DIR} -D${OS} -D${VER}
LDFLAGS=-L ${SIMPL_DIR}/lib -lsimpl 
LIBS=-lcurses
SOURCES =  game_admin timer courier display_admin painter input_admin keyboard
# SOURCES = display_admin game_admin cycle timer courier painter input_admin keyboard

all: ${SOURCES}

%.o: %.c
	${CC} ${CDFLAGS} $? -o $@

timer.o: message.h timer.c  
	${CC} ${CDFLAGS} timer.c -o timer.o

timer: timer.o
	${CC} timer.o -o timer ${LDFLAGS}


courier.o: message.h courier.c
	${CC} ${CDFLAGS} courier.c -o courier.o

courier: courier.o
	${CC} courier.o -o courier ${LDFLAGS}


keyboard.o: message.h keyboard.c
	${CC} ${CDFLAGS} keyboard.c -o keyboard.o 

keyboard: keyboard.o
	${CC} keyboard.o -o keyboard ${LDFLAGS}	${LIBS}

TODO:
painter.o: message.h painter.c
	${CC} ${CDFLAGS} painter.c -o painter.o 

painter: painter.o
	${CC} painter.o -o painter ${LDFLAGS} ${LIBS}


input_admin.o: message.h input_admin.c
	${CC} ${CDFLAGS} input_admin.c -o input_admin.o

input_admin: input_admin.o
	${CC} input_admin.o -o input_admin ${LDFLAGS}


game_admin.o: message.h game_admin.c
	${CC} ${CDFLAGS} game_admin.c -o game_admin.o

game_admin: game_admin.o
	${CC} game_admin.o -o game_admin ${LDFLAGS}


display_admin.o: message.h display_admin.c
	${CC} ${CDFLAGS} display_admin.c -o display_admin.o

display_admin: display_admin.o
	${CC} display_admin.o -o display_admin ${LDFLAGS}




clean:
	@rm -f *.o *~ ${SOURCES}

