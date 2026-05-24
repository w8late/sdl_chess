#pragma once
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

/* helper */
#define LOG_IF_ERR(expr, stat)  \
    do {                        \
        if (!(expr)) {            \
            SDL_Log("%s", SDL_GetError());  \
            stat;               \
        }                       \
    } while(0);  

/* global defines */
#define WIDTH 1024
#define HEIGHT 1024
#define COLOR_WHITE 0
#define COLOR_BLACK 1

enum PIECE_TYPE {
    PIECE_PAWN,
    PIECE_ROOK,
    PIECE_KNIGHT,
    PIECE_BISHOP,
    PIECE_QUEEN,
    PIECE_KING,
    PIECE_NONE
};

struct chessPiece {
    int type;
    int color;
};

struct gameState {
    int check;
    int checked_color;
    int checked_cell;
    int checkmate;
    int winner;
    int color_turn;
};

/* coordinate to board index 
 * where (1,1) == 0 and (8,8) == 63 */
#define c2i(x, y) (((y)-1) * 8 + ((x)-1))

/* board index to coordinate
 * where 0 == (0,0) and 63 == (7,7)
   ox and oy are out parameters */
#define i2c(i, ox, oy) \
    do {\
        ox = i % 8;\
        oy = (int)(i / 8);\
    } while(0)

/* screen point to board index 
 * where (0,0) == 0 and (WIDTH-1, HEIGHT-1) == 63 */
#define p2i(x, y) (c2i((int)(8.*(x)/WIDTH)+1, (int)(8.*(y)/HEIGHT)+1))

/* global variables are a bit messy, but oh well */
extern struct chessPiece B[64]; /* could have been one big uint64_t with bitwise operations for cell type and color */
extern int player_color;
extern SDL_Texture *sprites[2][6];
extern MIX_Track *sounds[2]; /* 0: move, 1: impact */
extern struct gameState state;