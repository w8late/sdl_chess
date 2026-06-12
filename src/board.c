/* these are the worst pieces of code I've ever written
 * but it works and it's fast enough so who cares */

#include <math.h>
#include "board.h"
#include "common.h"

int in_range(int i, int b1, int b2);

/* ==== board methods ====*/

/* render all pieces on the board */
void renderBoard(SDL_Renderer *render) {
    int i;

    /* render pieces and tiles beneath them */
    for (i = 0; i < 64; i++) {
        SDL_FRect rect;
        int x, y;
        i2c(i, x, y);

        rect.x = x * CELL_SIZE;
        rect.y = y * CELL_SIZE;
        rect.w = CELL_SIZE;
        rect.h = CELL_SIZE;

        SDL_SetRenderDrawColor(render,128,90,64,SDL_ALPHA_OPAQUE);
        if (((x+1) % 2 && y % 2) || (x % 2 && (y+1) % 2))  SDL_RenderFillRect(render, &rect);  /* alternating tiles */
        if (B[i].type != PIECE_NONE) SDL_RenderTexture(render,sprites[B[i].color][B[i].type],NULL,&rect); /* pieces */
    }
}

int movesOutOfCheck(int prev_cell, int cell, struct chessPiece piece) {
    /* save board state */
    struct chessPiece board[64];
    memcpy(board, B, sizeof(struct chessPiece) * 64);
    B[cell] = piece;
    B[prev_cell].type = PIECE_NONE;

    if (!inCheck(state.checked_cell, state.checked_color)) { 
        /* reset board to save after checking */
        memcpy(B, board, sizeof(struct chessPiece) * 64);
        return 1; 
    } else {
        memcpy(B, board, sizeof(struct chessPiece) * 64);
        return 0; 
    };
}

/* check if the piece (king) has been mated */
int inMate(int cell) {
    const int king_moves[] = {-9, -8, -7, -1, 1, 7, 8, 9};
    int c, i, cdist, ydist;
    int valid_moves = 0;

    /* loop through all other moves and count the total possible moves */
    for (i = 0; i < 8; i++) {
        c = cell + king_moves[i];
        if (c >= 64 || c < 0) continue;

        cdist = abs(cell-c); 
        ydist = abs((int)(c/8) - (int)(cell/8)); 
        if (cdist == 1 && ydist == 0 && validMove(cell, c, B[cell])) valid_moves++;
        else if ((cdist == 7 || cdist == 8 || cdist == 9) && ydist == 1 && validMove(cell, c, B[cell])) valid_moves++;    
    }

    /* the king has been mated if there are no valid moves */
    return valid_moves == 0;
}

/* check if the piece (king) is in check */
int inCheck(int cell, int color) {
    int c, i;
    int cdist, ydist;
    const int knight_moves[] = {-17, -15, -10, -6, 6, 10, 15, 17}; /* cells the knight could move to, from this one */
    const int king_moves[] = {-9, -8, -7, -1, 1, 7, 8, 9}; /* cells the king could move to, from this one */
    
    /* rooks & queens (big hack)*/
    for (i = 1; (c = cell + i) < (int)(cell/8)*8+8; i++) {
        if (B[c].type != PIECE_NONE) {
            if ((B[c].type == PIECE_ROOK || B[c].type == PIECE_QUEEN) && B[c].color != color) return 1;
            break;
        }
    }
    for (i = 1; (c = cell - i) >= (int)(cell/8)*8; i++) {
        if (B[c].type != PIECE_NONE) {
            if ((B[c].type == PIECE_ROOK || B[c].type == PIECE_QUEEN) && B[c].color != color) return 1;
            break;
        }
    }
    for (i = 1; (c = cell + i * 8) < cell % 8 + 56; i++) {
        if (B[c].type != PIECE_NONE) {
            if ((B[c].type == PIECE_ROOK || B[c].type == PIECE_QUEEN) && B[c].color != color) return 1;
            break;
        }
    }
    for (i = 1; (c = cell - i * 8) > cell % 8; i++) {
        if (B[c].type != PIECE_NONE) {
            if ((B[c].type == PIECE_ROOK || B[c].type == PIECE_QUEEN) && B[c].color != color) return 1;
            break;
        }
    }

    /* bishops & queens (big hack pt. 2) */
    for (i = 1; (c = cell + i * 9) < 64; i++) {
        if (B[c].type != PIECE_NONE) {
            if ((B[c].type == PIECE_BISHOP || B[c].type == PIECE_QUEEN) && B[c].color != color) return 1;
            break;
        }
    }
    for (i = 1; (c = cell - i * 9) > 0; i++) {
        if (B[c].type != PIECE_NONE) {
            if ((B[c].type == PIECE_BISHOP || B[c].type == PIECE_QUEEN) && B[c].color != color) return 1;
            break;
        }
    }

    for (i = 1; (c = cell + i * 7) < 64; i++) {
        if (B[c].type != PIECE_NONE) {
            if ((B[c].type == PIECE_BISHOP || B[c].type == PIECE_QUEEN) && B[c].color != color) return 1;
            break;
        }
    }
    for (i = 1; (c = cell - i * 7) > 0; i++) {
        if (B[c].type != PIECE_NONE) {
            if ((B[c].type == PIECE_BISHOP || B[c].type == PIECE_QUEEN) && B[c].color != color) return 1;
            break;
        }
    }
    
    /* pawns */
    if (color == player_color) { /* pawns aren't symmetrical */
        for (c = cell - 7; c >= cell-  9; c--) {
            if (c >= 64 || c < 0) continue;
            if (c == cell - 8) continue;

            if (B[c].type == PIECE_PAWN && B[c].color != color) return 1;
        }     
    } else {
         for (c = cell + 7; c <= cell+9; c++) {
            if (c >= 64 || c < 0) continue;
            if (c == cell + 8) continue;

            if (B[c].type == PIECE_PAWN && B[c].color != color) return 1;
         } 
    }
    
    /* knights */
    for (i = 0; i < 8; i++) {
        c = cell + knight_moves[i];
        if (c >= 64 || c < 0) continue;

        if (B[c].type == PIECE_KNIGHT && B[c].color != color) {
            cdist = abs(cell-c); 
            ydist = abs((int)(c/8) - (int)(cell/8)); 
            if ((cdist == 6 || cdist == 10) &&  ydist == 1) return 1;
            else if ((cdist == 15 || cdist == 17) && ydist == 2) return 1;
        }
    }

    /* the other king */
    for (i = 0; i < 8; i++) {
        c = cell + king_moves[i];
        if (c >= 64 || c < 0) continue;

        if (B[c].type == PIECE_KING && B[c].color != color) {
            cdist = abs(cell-c); 
            ydist = abs((int)(c/8) - (int)(cell/8)); 
            if (cdist == 1 && ydist == 0) return 1;
            else if ((cdist == 7 || cdist == 8 || cdist == 9) && ydist == 1) return 1;    
        }
    }
    
    return 0;
}

/* returns 1 if the move from previous cell to the current cell is valid, otherwise 0 */
int validMove(int prev_cell, int cell, struct chessPiece piece)  {
    int x, y, px, py;
    int cdist, ydist;
    float slope;
    /* first off, can't capture a piece of your own color */
    if (B[cell].type != PIECE_NONE && B[cell].color == piece.color) return 0;
    /* also can't capture a king */
    if (B[cell].type == PIECE_KING) return 0; 
    /* don't bother if piece hasn't moved either */
    if (prev_cell == cell) return 0;
    /* cant move it's not your turn! */
    if (piece.color != state.color_turn) return 0; 
    cdist = abs(cell-prev_cell); /* distance between cells, counting along each row */
    ydist = abs((int)(prev_cell/8) - (int)(cell/8)); /* distance along the y-axis between cells */
    /* handle movement patterns of each piece */
    switch (piece.type) {
    case PIECE_KING:
        /* you can't move the king *into* check */
        if (((cdist == 1 && ydist == 0) ||
        ((cdist == 7 || cdist == 8 || cdist == 9) && ydist == 1)) && !inCheck(cell, piece.color)) return 1;    
        else return 0;
        break;
    case PIECE_KNIGHT:
        if ((cdist == 6 || cdist == 10) &&  ydist == 1) return 1;
        else if ((cdist == 15 || cdist == 17) && ydist == 2) return 1;
        else return 0;
        break;
    case PIECE_PAWN:
        if (piece.color == player_color) {
            if (cell + 8 == prev_cell && B[cell].type == PIECE_NONE ) return 1; /* move up a square */
            else if (cell + 16 == prev_cell && (int)(prev_cell / 8) == 6 && B[cell].type == PIECE_NONE && B[cell+8].type == PIECE_NONE) return 1; /* is the pawn's first move from 2nd rank */
            else if ((prev_cell - cell == 7 || prev_cell - cell == 9) && B[cell].type != PIECE_NONE) return 1; /* capturing piece on the diagonal */
            else return 0;
        } else {
            if (prev_cell + 8 == cell && B[cell].type == PIECE_NONE) return 1; /* move down a square */
            else if (prev_cell + 16 == cell && (int)(prev_cell/8) == 1 && B[cell].type == PIECE_NONE && B[cell-8].type == PIECE_NONE) return 1; /* is the pawn's first move from 7th rank */
            else if ((cell - prev_cell == 7 || cell - prev_cell == 9) && B[cell].type != PIECE_NONE) return 1;
            else return 0;
        }
        break;
    case PIECE_ROOK:
        if (abs(cell - prev_cell) % 8 == 0) {
            return in_range(cell, prev_cell, findPieceAtAngle(cell, prev_cell, 8));
        } else if ((int)(prev_cell/8) == (int)(cell/8)) {
            return in_range(cell, prev_cell, findPieceAtAngle(cell, prev_cell, 1));
        } else return 0;
        break;
    case PIECE_BISHOP:
        i2c(cell, x, y);
        i2c(prev_cell, px, py);
        if (px == x) return 0;
        slope = ((float)(py-y)/(px-x));
        if (slope == 1 || slope == -1) {
            return in_range(cell, prev_cell, findPieceAtAngle(cell, prev_cell, (slope>0)?9:7));
        } else return 0;
        break;
    /* the queen is basically the bishop and rook in one, biggest mess yet*/
    case PIECE_QUEEN:
        i2c(cell, x, y);
        i2c(prev_cell, px, py);
        slope = ((float)(py-y)/(px-x));
        if (abs(cell - prev_cell) % 8 == 0) {
            return in_range(cell, prev_cell, findPieceAtAngle(cell, prev_cell, 8));
        } else if ((int)(prev_cell/8) == (int)(cell/8)) {
            return in_range(cell, prev_cell, findPieceAtAngle(cell, prev_cell, 1));
        } else if (slope == 1 || slope == -1) {
            return in_range(cell, prev_cell, findPieceAtAngle(cell, prev_cell, (slope>0)?9:7));
        } else return 0;
        break;
    default: return 0;
    }   
}

/* returns 1 if i is in range of b1 -> b2 (inclusive), else 0 */
int in_range(int i, int b1, int b2) {
    return (i >= min(b1, b2) && i <= max(b1,b2));
}

/* returns ... */
int findPieceAtAngle(int c1, int c2, int angle) {
    int i, c = c1, m = max(c1,c2);

    for (i = min(c1,c2); i < m; i += angle) 
        if (B[i].type != PIECE_NONE) c = i;
    return c;
}
