#pragma once
#include <SDL3/SDL.h>

struct chessPiece;
int validMove(int prev_cell, int cell, struct chessPiece piece);
int findPieceAtAngle(int c1, int c2, int angle);
void renderBoard(SDL_Renderer *render);
int inCheck(int cell, int color);
int movesOutOfCheck(int prev_cell, int cell, struct chessPiece piece);
        