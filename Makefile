CFLAGS=-std=c99 -g -Wall
LFLAGS=-lSDL2

OUT=editor
OBJS=main.o     \
     buffer.o   \
     editor.o   \
     file.o     \
     font.o     \
     input.o    \
     row.o      \
     syntax.o   \
     terminal.o \
     window.o

.PHONY: all clean

all: $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

${OUT}: $(OBJS)
	$(CC) $(LFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) $(OUT)
