#include <SDL3_image/SDL_image.h>
#include "init.h"
#include "common.h"

void initAudio(MIX_Mixer *mx) {
    MIX_Audio *a[2];

    a[0] = MIX_LoadAudio(mx,"./assets/piece_move.ogg",false);
    a[1] = MIX_LoadAudio(mx,"./assets/piece_impact.ogg",false);
    sounds[0] = MIX_CreateTrack(mx);
    sounds[1] = MIX_CreateTrack(mx);
    MIX_SetTrackAudio(sounds[0], a[0]);
    MIX_SetTrackAudio(sounds[1], a[1]);
}

void initSprites(SDL_Renderer *render) {
    sprites[COLOR_WHITE][PIECE_PAWN] = IMG_LoadTexture(render,"./assets/w_Pawn.png");
    sprites[COLOR_WHITE][PIECE_ROOK] = IMG_LoadTexture(render,"./assets/w_Rook.png");
    sprites[COLOR_WHITE][PIECE_KNIGHT] = IMG_LoadTexture(render,"./assets/w_Knight.png");
    sprites[COLOR_WHITE][PIECE_BISHOP] = IMG_LoadTexture(render,"./assets/w_Bishop.png");
    sprites[COLOR_WHITE][PIECE_QUEEN] = IMG_LoadTexture(render,"./assets/w_Queen.png");
    sprites[COLOR_WHITE][PIECE_KING] = IMG_LoadTexture(render,"./assets/w_King.png");
    sprites[COLOR_BLACK][PIECE_PAWN] = IMG_LoadTexture(render,"./assets/b_Pawn.png");
    sprites[COLOR_BLACK][PIECE_ROOK] = IMG_LoadTexture(render,"./assets/b_Rook.png");
    sprites[COLOR_BLACK][PIECE_KNIGHT] = IMG_LoadTexture(render,"./assets/b_Knight.png");
    sprites[COLOR_BLACK][PIECE_BISHOP] = IMG_LoadTexture(render,"./assets/b_Bishop.png");
    sprites[COLOR_BLACK][PIECE_QUEEN] = IMG_LoadTexture(render,"./assets/b_Queen.png");
    sprites[COLOR_BLACK][PIECE_KING] = IMG_LoadTexture(render,"./assets/b_King.png");
}


/* initializes board. Given color is the player's color: relevant pieces are placed at the bottom. */
void initBoard(int player_color) {
    int opposite_color = player_color == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE;
    int i;
    for (i = 0; i < 64; i++)
        B[i].type = PIECE_NONE;

    /* init pieces 
     * pawns */
    for (i = 1; i <= 8; i++) {
        B[c2i(i,2)].type = B[c2i(i,7)].type = PIECE_PAWN;
        B[c2i(i,2)].color = opposite_color;
        B[c2i(i,7)].color = player_color;
    }

    /* rooks */
    B[c2i(1,1)].type = B[c2i(8,1)].type = B[c2i(1,8)].type = B[c2i(8,8)].type = PIECE_ROOK; 
    B[c2i(1,1)].color = B[c2i(8,1)].color = opposite_color;
    B[c2i(1,8)].color = B[c2i(8,8)].color = player_color;

    /* knights */
    B[c2i(2,1)].type = B[c2i(7,1)].type = B[c2i(2,8)].type = B[c2i(7,8)].type = PIECE_KNIGHT; 
    B[c2i(2,1)].color = B[c2i(7,1)].color = opposite_color;
    B[c2i(2,8)].color = B[c2i(7,8)].color = player_color;

    /* bishops */
    B[c2i(3,1)].type = B[c2i(6,1)].type = B[c2i(3,8)].type = B[c2i(6,8)].type = PIECE_BISHOP; 
    B[c2i(3,1)].color = B[c2i(6,1)].color = opposite_color;
    B[c2i(3,8)].color = B[c2i(6,8)].color = player_color;

    /* queens & kings 
     * queens are on their own color; kings are on the opposite */
    switch (player_color) {
    case COLOR_WHITE:
        B[c2i(4,1)].type = B[c2i(4,8)].type = PIECE_QUEEN;
        B[c2i(5,1)].type = B[c2i(5,8)].type = PIECE_KING;
        break;
    case COLOR_BLACK:
        B[c2i(5,1)].type = B[c2i(5,8)].type = PIECE_QUEEN;
        B[c2i(4,1)].type = B[c2i(4,8)].type = PIECE_KING;
        break;
    }
    B[c2i(4,1)].color = B[c2i(5,1)].color = opposite_color; 
    B[c2i(4,8)].color = B[c2i(5,8)].color =  player_color; 
}

