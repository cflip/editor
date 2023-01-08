CFLAGS=-std=c99

OUT=editor
SRC=main.c     \
	buffer.c   \
	editor.c   \
	file.c     \
	input.c    \
	row.c      \
	syntax.c   \
	terminal.c \

${OUT}: ${SRC}
	${CC} ${CFLAGS} ${SRC} -o ${OUT}
