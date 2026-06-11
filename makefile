CC = clang -std=c89 -Wall -Wextra -Og -g
FLAGS = -I ./include -llib/SDL3 -llib/SDL3_image -llib/SDL3_mixer -llib/SDL3_ttf
BINARY = chess
ARGS = -pWHITE -t5

all:
	$(CC) src/main.c src/board.c src/init.c $(FLAGS) -o bin/$(BINARY).exe
run:
	bin/$(BINARY) $(ARGS)