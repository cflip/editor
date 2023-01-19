CFLAGS=-std=c99 -g -Wall
LFLAGS=-lSDL2 -lz

OUT=editor
OBJS=main.o     \
     buffer.o   \
     editor.o   \
     error.o    \
     file.o     \
     font.o     \
     input.o    \
     line.o     \
     syntax.o   \
     window.o

.PHONY: all clean

all: $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

${OUT}: $(OBJS)
	$(CC) $(LFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) $(OUT)
