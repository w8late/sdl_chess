/* chess pieces from https://unknuffig.itch.io/2d-chess-pices */
/* sound effects from https://jdsherbert.itch.io/tabletop-games-sfx-pack */

#include <string.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include "init.h"
#include "board.h"
#include "common.h"

/* global variables are a bit messy, but oh well */
struct chessPiece B[64];
int player_color;
SDL_Texture *sprites[2][6];
MIX_Track *sounds[2]; /* 0: move, 1: impact */
struct gameState state = {0};

int parseArgumentsForColor(char *arg);

int main(int argc, char **argv) {  
    state.color_turn = COLOR_WHITE;
    SDL_Window *window;
    SDL_Renderer *render;
    MIX_Mixer *mixer;
    SDL_Event event;
    float mx,my;
    int i, j; /* loop indices */
    struct chessPiece held_piece = {PIECE_NONE, COLOR_WHITE};
    int picked = 0; /* held piece has been picked */
    int cell, prev_cell;
    SDL_Surface *wicon;

    LOG_IF_ERR(SDL_Init(SDL_INIT_VIDEO), return 1)
    LOG_IF_ERR(MIX_Init(), return 1)
    LOG_IF_ERR(mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL), return 1)
    initAudio(mixer);
    LOG_IF_ERR(SDL_CreateWindowAndRenderer("CHESS", WIDTH,HEIGHT, 0, &window,&render), return 1)
    initSprites(render);
    initBoard(player_color); 
    SDL_SetRenderLogicalPresentation(render, WIDTH,HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    
    /* load chess piece sprites */
    if (argc < 2) return 1;
    player_color = parseArgumentsForColor(argv[1]);
     
    wicon = IMG_Load("./assets/chess_win_icon.png");
    SDL_SetWindowIcon(window, wicon);
 
    while (1) {
        SDL_FRect rect;
        rect.w = WIDTH/8;
        rect.h = HEIGHT/8;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                goto quit;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:     
                SDL_GetMouseState(&mx,&my);
                prev_cell = cell = p2i(mx,my);
                if (!picked  && B[cell].type != PIECE_NONE) {
                    held_piece = B[cell];
                    B[cell].type = PIECE_NONE;
                    picked = 1;
                    MIX_PlayTrack(sounds[0], 0);
                }
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                SDL_GetMouseState(&mx,&my);
                cell = p2i(mx,my);
                if (picked && validMove(prev_cell, cell, held_piece)) {
                    B[cell] = held_piece;
                    MIX_PlayTrack(sounds[1], 0);
                    state.color_turn = 1 - state.color_turn; /* clever way of switching turns */
                } else B[prev_cell] = held_piece;     
                held_piece.type = PIECE_NONE;
                picked = 0;
                break;
            }
        }

        for (i = 0; i < 64; i++)
            if (B[i].type == PIECE_KING && inCheck(cell, B[i].color)) {
                state.check = 1;
                state.checked_color = B[i].color;
                state.checked_cell = i;
            }
        SDL_GetMouseState(&mx,&my);
        SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(render, 234,210,168, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(render);
        renderBoard(render);
        if (picked) { 
            SDL_SetRenderDrawColor(render, 44,44,44, 128);  
            for (i = 0; i < 64; i++) {   
                int x,y;
                i2c(i, x, y);
                rect.x = x * rect.w;
                rect.y = y * rect.h;      
                if (validMove(prev_cell, i, held_piece)) SDL_RenderFillRect(render, &rect);
            }
            rect.x = mx - rect.w/2;
            rect.y = my - rect.h/2;
            SDL_RenderTexture(render,sprites[held_piece.color][held_piece.type],NULL,&rect);   
        }
        SDL_RenderPresent(render);
    }
quit:
    for (i = 0; i < 2; i++) 
        for (j = 0; j < 6; j++) 
            SDL_DestroyTexture(sprites[i][j]);
    
    SDL_DestroySurface(wicon);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(render);
    MIX_Quit();
    SDL_Quit();
    return 0;
}

/* returns NULL on failure, otherwise returns static string. */
int parseArgumentsForColor(char *arg) {
    if (strncmp("-p", arg, 2)) {
        SDL_Log("invalid argument");
        exit(1);
    } else {
        char *v = arg + 2; /* argument value */
        if (!strcmp("BLACK",v)) return COLOR_BLACK;
        else if (!strcmp("WHITE",v)) return COLOR_WHITE;
        else exit(1);
    }
    return 42; /* won't be reached anyway */
}
