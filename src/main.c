/* chess pieces from https://unknuffig.itch.io/2d-chess-pices */
/* sound effects from https://jdsherbert.itch.io/tabletop-games-sfx-pack */

#include <string.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "init.h"
#include "board.h"
#include "common.h"

/* global variables are a bit messy, but oh well */
struct chessPiece B[64]; /* the board: each tile has a color and type, or nothing at all */
int player_color;
SDL_Texture *sprites[2][6]; /* sprites for each color */
MIX_Track *sounds[2]; /* 0: move, 1: impact */
struct gameState state = {0};
TTF_Font *global_font;

int parseArgumentForColor(char *arg);
int parseArgumentForTime(char *arg);
void renderTimer(SDL_Renderer *render, SDL_FRect *rect, struct timer_data t);
int windowInFocus(SDL_Window *win);
unsigned int decrementTimer(void *userdata, SDL_TimerID id, unsigned int interval);

int main(int argc, char **argv) {  
    state.color_turn = COLOR_WHITE;
    SDL_Window *window, *twindow;
    SDL_Renderer *render, *trender;
    MIX_Mixer *mixer;
    SDL_Event event;
    float mx,my; /* mouse coords */
    int i, j; /* loop indices */
    struct chessPiece held_piece = {PIECE_NONE, COLOR_WHITE};
    int picked = 0; /* held piece has been picked */
    int cell = 0, prev_cell = 0;
    SDL_Surface *wicon;
    SDL_FRect timer_rect;
    struct timer_data black_timer = {10*60, COLOR_BLACK}, white_timer = {10*60, COLOR_WHITE};
    timer_rect.w = 100;
    timer_rect.h = 32;

    LOG_IF_ERR(SDL_Init(SDL_INIT_VIDEO), return 1)
    LOG_IF_ERR(TTF_Init(), return 1)
    global_font = TTF_OpenFont("./assets/arialbd.ttf", 24);;
    LOG_IF_ERR(MIX_Init(), return 1)

    LOG_IF_ERR(mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL), return 1)
    initAudio(mixer);
    LOG_IF_ERR(SDL_CreateWindowAndRenderer("CHESS", WIDTH,HEIGHT, 0, &window,&render), return 1)
    LOG_IF_ERR(SDL_CreateWindowAndRenderer("CHESS UI", 320,240, 0, &twindow,&trender), return 1)
    initSprites(render);
    initBoard(player_color); 
    SDL_SetRenderLogicalPresentation(render, WIDTH,HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_SetRenderLogicalPresentation(trender, 320,240, SDL_LOGICAL_PRESENTATION_LETTERBOX);
   
    if (argc < 2) return 1;
    player_color = parseArgumentForColor(argv[1]);

    if (argc > 2) 
        white_timer.timer = black_timer.timer = parseArgumentForTime(argv[2])*60;
    else  /* default timer */
        white_timer.timer = black_timer.timer = 10*60;

    
     
    wicon = IMG_Load("./assets/chess_win_icon.png");
    SDL_SetWindowIcon(window, wicon);

    SDL_AddTimer(1000, decrementTimer, &black_timer);
    SDL_AddTimer(1000, decrementTimer, &white_timer);
 
    while (1) {
        /* we're reusing this single rect: silly */
        SDL_FRect rect;
        rect.w = CELL_SIZE;
        rect.h = CELL_SIZE;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                goto quit;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN: 
                if (!windowInFocus(window)) continue;    
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
                if (!windowInFocus(window)) continue;  
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

        state.check = 0;
        for (i = 0; i < 64; i++)
            if (B[i].type == PIECE_KING && inCheck(i, B[i].color)) {
                state.check = 1;
                state.checked_color = B[i].color;
                state.checked_cell = i;
            }
        SDL_GetMouseState(&mx,&my);
        SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(render, 234,210,168, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(render);
    
        renderBoard(render);

        /* render all possible moves for the picked piece as grey tiles */
        if (picked && windowInFocus(window)) { 
            SDL_SetRenderDrawColor(render, 44,44,44, 128);  
            for (i = 0; i < 64; i++) {   
                int x,y;
                i2c(i, x, y);
                rect.x = x * rect.w;
                rect.y = y * rect.h;  
                
                //if (validMove(prev_cell, i, held_piece))  SDL_RenderFillRect(render, &rect); 
                
                if (validMove(prev_cell, i, held_piece)) { 
                    if (state.check && movesOutOfCheck(cell, i, held_piece)) {
                        SDL_RenderFillRect(render, &rect);
                    } else if (!state.check) {
                        SDL_RenderFillRect(render, &rect);
                    }
                }
            }
            /* render the picked piece under the mouse cursor */
            rect.x = mx - rect.w/2;
            rect.y = my - rect.h/2;
            SDL_RenderTexture(render,sprites[held_piece.color][held_piece.type],NULL,&rect);   
        }

      

        /* render timers */
        SDL_SetRenderDrawColor(trender, 22,22,22, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(trender);
        timer_rect.y =  8;
        timer_rect.x = 16;
        SDL_SetRenderDrawColor(trender, 234,210,168, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(trender, &timer_rect);
        renderTimer(trender, &timer_rect, black_timer);
       
        timer_rect.x = 320 - timer_rect.w - 16;
        SDL_SetRenderDrawColor(trender,128,90,64,SDL_ALPHA_OPAQUE); 
        SDL_RenderFillRect(trender, &timer_rect);
        renderTimer(trender, &timer_rect, white_timer);
        
        SDL_RenderPresent(render);
        SDL_RenderPresent(trender);
    }
quit:
    for (i = 0; i < 2; i++) 
        for (j = 0; j < 6; j++) 
            SDL_DestroyTexture(sprites[i][j]);
    
    SDL_DestroySurface(wicon);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(twindow);
    SDL_DestroyRenderer(trender);
    MIX_Quit();
    SDL_Quit();
    return 0;
}

/* render time in MM:SS format */
void renderTimer(SDL_Renderer *render, SDL_FRect *rect, struct timer_data t) {
    SDL_Color color;
    SDL_Surface *surf;
    SDL_Texture *txtr;
    color.r = 255;
    color.g = 255;
    color.b = 255;

    char buf[6];
    SDL_snprintf(buf, 6, "%.2d:%.2d", (int)(t.timer/60), t.timer % 60);
    surf = TTF_RenderText_Solid(global_font, buf, 5, color);
    txtr = SDL_CreateTextureFromSurface(render, surf);
    SDL_RenderTexture(render, txtr, NULL, rect);

    SDL_DestroyTexture(txtr);
    SDL_DestroySurface(surf);
}

/* returns color on success, otherwise just exits (lol) */
int parseArgumentForColor(char *arg) {
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

/* returns timer value on success */
int parseArgumentForTime(char *arg) {
    if (strncmp("-t", arg, 2)) {
        SDL_Log("invalid argument");
        exit(1);
    } else {
        char *v = arg + 2; /* argument value */
        return strtol(v, NULL, 10);
    }
    return 42; /* won't be reached anyway */
}

int windowInFocus(SDL_Window *win) {
    int flags = SDL_GetWindowFlags(win);
    return flags & SDL_WINDOW_INPUT_FOCUS;
}

unsigned int decrementTimer(void *userdata, SDL_TimerID _id, unsigned int _interval) {
    struct timer_data *t = userdata;

    if (t->color != state.color_turn && t->timer > 0) t->timer--;
    return 1000;
}
