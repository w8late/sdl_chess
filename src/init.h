#pragma once
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

void initAudio(MIX_Mixer *mx);
void initSprites(SDL_Renderer *render);
void initBoard(int color);