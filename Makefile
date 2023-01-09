CFLAGS=-std=c99 -g
LFLAGS=-lSDL2

OUT=editor
SRC=main.c     \
	buffer.c   \
	editor.c   \
	file.c     \
	font.c     \
	input.c    \
	row.c      \
	syntax.c   \
	terminal.c \
	window.c

${OUT}: ${SRC}
	${CC} ${CFLAGS} ${LFLAGS} ${SRC} -o ${OUT}
