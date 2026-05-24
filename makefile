CC = clang -std=c89 -Wall -Wextra -g
FLAGS = -I ./include -llib/SDL3 -llib/SDL3_image -llib/SDL3_mixer
BINARY = chess
ARGS = -pWHITE

all:
	$(CC) src/main.c src/board.c src/init.c $(FLAGS) -o bin/$(BINARY).exe
run:
	bin/$(BINARY) $(ARGS)