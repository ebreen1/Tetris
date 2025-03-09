#include "tetris.h"
#include <string.h>
#include <stdlib.h>
#include <SDL3/SDL.h>

const struct tetromino I = {
	size: 4,
	color: { 0, 0xE6, 0xFE, 0xFF },
	cells: {
		0, 0, 0, 0,
		1, 1, 1, 1,
		0, 0, 0, 0,
		0, 0, 0, 0
	}
};

const struct tetromino J = {
	size: 3,
	color: { 0x18, 0x01, 0xFF, 0xFF },
	cells: {
		1, 0, 0,
		1, 1, 1,
		0, 0, 0
	}
};

const struct tetromino L = { 
	size: 3,
	color: { 0xFF, 0x73, 0x08, 0xFF },
	cells: {
		0, 0, 1,
		1, 1, 1,
		0, 0, 0
	}
};

const struct tetromino O = { 
	size: 2,
	color: { 0xFF, 0xFF, 0x00, 0xFF },
	cells: {
		1, 1,
		1, 1
	}
};

const struct tetromino S = { 
	size: 3,
	color: { 0x00, 0xFF, 0x00, 0xFF },
	cells: {
		0, 1, 1,
		1, 1, 0,
		0, 0, 0
	}
};

const struct tetromino Z = { 
	size: 3,
	color: { 0xFF, 0x00, 0x08, 0xFF },
	cells: {
		1, 1, 0,
		0, 1, 1,
		0, 0, 0
	}
};

const struct tetromino T = { 
	size: 3,
	color: { 0xFF, 0x00, 0xFF, 0xFF },
	cells: {
		0, 1, 0,
		1, 1, 1,
		0, 0, 0
	}
};

static void renderCell(SDL_Renderer *renderer, int x, int y, int cellSize, SDL_Color color){
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

	SDL_FRect dest = { x * cellSize, y * cellSize, cellSize, cellSize };
	SDL_RenderFillRect(renderer, &dest);

	SDL_SetRenderDrawColor(renderer, color.r * 0.8, color.g * 0.8, color.b * 0.8, color.a * 0.8);

	SDL_RenderRect(renderer, &dest);
}

struct field *createField(size_t width, size_t height, size_t cellSize, unsigned int dropTime){
	struct field *f = malloc(sizeof(*f) + sizeof(*f->cells) * width * height);

	f->width = width;
	f->height = height;
	f->cellSize = cellSize;
	f->dropTime = dropTime;

	spawnPiece(f, L_PIECE);

	memset(f->cells, 0, sizeof(*f->cells) * width * height);

	return f;
}

void renderField(SDL_Renderer *renderer, struct field *f){
	for(int y = 0; y < f->height; y++){
		for(int x = 0; x < f->width; x++){
			renderCell(renderer, x, y, f->cellSize, f->cells[y * f->width + x]);
		}
	}

	for(int y = 0; y < f->currentPiece.size; y++){
		for(int x = 0; x < f->currentPiece.size; x++){
			if(f->currentPiece.cells[f->currentPiece.size * y + x]){
				renderCell(renderer, f->pieceX + x, f->pieceY + y, f->cellSize, f->currentPiece.color);
			}
		}
	}
}

void destroyField(struct field *f){
	free(f);
}

void checkLines(struct field *f){
	for(int y = 0; y < f->height; y++){
		bool lineFull = true;
		for(int x = 0; x < f->width; x++){
			if(!f->cells[f->width*y + x].a){
				lineFull = false;
				break;
			}
		}

		if(!lineFull){
			continue;
		}

		SDL_Log("Clearing line %d\n", y);
		// If the line is filled, clear it
		for(int x = 0; x < f->width; x++){
			f->cells[f->width * y + x].a = 0;
		}

		// The lines above the one cleared move down
		for(int row = y-1; row >= 0; row--){
			for(int x = 0; x < f->width; x++){
				f->cells[ (row+1)*f->width + x ] = f->cells[ row*f->width + x ];
			}
		}
	}
}


void spawnPiece(struct field *f, enum PieceType p){
	switch(p){
	case I_PIECE:
		f->currentPiece = I;
		break;
	case J_PIECE:
		f->currentPiece = J;
		break;
	case L_PIECE:
		f->currentPiece = L;
		break;
	case O_PIECE:
		f->currentPiece = O;
		break;
	case S_PIECE:
		f->currentPiece = S;
		break;
	case Z_PIECE:
		f->currentPiece = Z;
		break;
	case T_PIECE:
		f->currentPiece = T;
		break;
	}

	f->pieceX = 0;
	f->pieceY = -2;
	f->lastDrop = SDL_GetTicks();
}

void lockPiece(struct field *f){
	if(f->pieceY < -1){
		SDL_Log("Game Over\n");
		return;
	}
	SDL_Log("Locking piece\n");
	for(int y = 0; y < f->currentPiece.size; y++){
		for(int x = 0; x < f->currentPiece.size; x++){
			if(f->currentPiece.cells[y * f->currentPiece.size + x]){
				int fieldX = x + f->pieceX;
				int fieldY = y + f->pieceY;
				f->cells[ fieldY * f->width + fieldX ] = f->currentPiece.color;
			}
		}
	}

	checkLines(f);
}

void updateField(struct field *f){
	unsigned int currentTime = SDL_GetTicks();

	if(currentTime - f->lastDrop > f->dropTime){
		if(!movePiece(f, 0, 1)){
			lockPiece(f);
			spawnPiece(f, rand() % 7);
		}
		f->lastDrop = currentTime;
	}
}

bool checkCollision(struct field *f){
	for(int y = 0; y < f->currentPiece.size; y++){
		for(int x = 0; x < f->currentPiece.size; x++){
			if(f->currentPiece.cells[y * f->currentPiece.size + x]){
				int fieldX = x + f->pieceX;
				int fieldY = y + f->pieceY;
				if(fieldY >= 0 && f->cells[ fieldY * f->width + fieldX ].a){
					//SDL_Log("Collision with cell on field\n");
					return true;
				}
				if(fieldX < 0 ){
					//SDL_Log("Collision with left edge\n");
					return true;
				}
				if(fieldX >= f->width){
					//SDL_Log("Collision with right edge\n");
					return true;
				}
				if(fieldY >= f->height){
					//SDL_Log("Collision with bottom edge\n");
					//SDL_Log("field coords: %d, %d\n", fieldX, fieldY);
					return true;
				}
			}
		}
	}
	return false;
}

bool movePiece(struct field *f, int x, int y){
	f->pieceX += x;
	f->pieceY += y;

	if(checkCollision(f)){
		f->pieceX -= x;
		f->pieceY -= y;
		return false;
	}

	return true;
}

bool rotatePiece(struct field *f, bool clockwise){
	struct tetromino oldPiece = f->currentPiece;
	int size = f->currentPiece.size;

	for(int y = 0; y < size; y++){
		for(int x = 0; x < size; x++){
			if(clockwise){
				f->currentPiece.cells[ y * size + x ] = oldPiece.cells[ size*(size-x-1) + y ];
			}
			else{
				f->currentPiece.cells[ y * size + x ] = oldPiece.cells[ size*x + size-y-1 ];
			}
		}
	}

	if(checkCollision(f)){
		f->currentPiece = oldPiece;
	}
}