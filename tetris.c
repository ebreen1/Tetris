#include "tetris.h"
#include <string.h>
#include <stdlib.h>
#include <SDL3/SDL.h>

SDL_Color colors[] = {
	{ 0x00, 0x00, 0x00, 0x00 }, // Empty cell - black
	{ 0x00, 0xE6, 0xFE, 0xFF },	// I - light blue
	{ 0x18, 0x01, 0xFF, 0xFF },	// J - blue
	{ 0xFF, 0x73, 0x08, 0xFF }, // L - orange
	{ 0xFF, 0xFF, 0x00, 0xFF }, // O - yellow
	{ 0x00, 0xFF, 0x00, 0xFF }, // S - green
	{ 0xFF, 0x00, 0x08, 0xFF }, // Z - red
	{ 0xFF, 0x00, 0xFF, 0xFF }, // T - purple
	{ 0xFF, 0xFF, 0xFF, 0xFF }, // Line clear - white
};

const struct tetromino I = {
	size: 4,
	cells: {
		0, 0, 0, 0,
		1, 1, 1, 1,
		0, 0, 0, 0,
		0, 0, 0, 0
	}
};

const struct tetromino J = {
	size: 3,
	cells: {
		1, 0, 0,
		1, 1, 1,
		0, 0, 0
	}
};

const struct tetromino L = { 
	size: 3,
	cells: {
		0, 0, 1,
		1, 1, 1,
		0, 0, 0
	}
};

const struct tetromino O = { 
	size: 2,
	cells: {
		1, 1,
		1, 1
	}
};

const struct tetromino S = { 
	size: 3,
	cells: {
		0, 1, 1,
		1, 1, 0,
		0, 0, 0
	}
};

const struct tetromino Z = { 
	size: 3,
	cells: {
		1, 1, 0,
		0, 1, 1,
		0, 0, 0
	}
};

const struct tetromino T = { 
	size: 3,
	cells: {
		0, 1, 0,
		1, 1, 1,
		0, 0, 0
	}
};

int getLandingY(struct field *f);

static void renderCell(SDL_Renderer *renderer, int x, int y, int cellSize, SDL_Color color){
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

	SDL_FRect dest = { x * cellSize, y * cellSize, cellSize, cellSize };
	SDL_RenderFillRect(renderer, &dest);

	SDL_SetRenderDrawColor(renderer, color.r * 0.8, color.g * 0.8, color.b * 0.8, color.a * 0.8);

	SDL_RenderRect(renderer, &dest);
}

static void renderSilhouetteCell(SDL_Renderer *renderer, int x, int y, int cellSize, SDL_Color color){
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

	SDL_FRect dest = { x * cellSize, y * cellSize, cellSize, cellSize };

	SDL_SetRenderDrawColor(renderer, color.r * 0.6, color.g * 0.6, color.b * 0.6, color.a * 0.6);
	SDL_RenderFillRect(renderer, &dest);

	SDL_SetRenderDrawColor(renderer, color.r * 0.4, color.g * 0.4, color.b * 0.4, color.a * 0.6);
	SDL_RenderRect(renderer, &dest);
}

struct field *createField(size_t width, size_t height, size_t cellSize, unsigned int dropTime, int clearDuration){
	struct field *f = malloc(sizeof(*f) + sizeof(*f->cells) * width * height);

	f->width = width;
	f->height = height;
	f->cellSize = cellSize;
	f->dropTime = dropTime;
	f->lineClearDuration = clearDuration;

	f->score = 0;

	spawnRandom(f);

	memset(f->cells, 0, sizeof(*f->cells) * width * height);

	return f;
}

void renderField(SDL_Renderer *renderer, struct field *f){

	// Render the field itself (not including the currently active piece)
	for(int y = 0; y < f->height; y++){
		for(int x = 0; x < f->width; x++){
			renderCell(renderer, x, y, f->cellSize, colors[f->cells[y * f->width + x]]);
		}
	}

	//Render the silhouette of where the current piece will land
	int silhouetteY = getLandingY(f);
	for(int y = 0; y < f->currentPiece.size; y++){
		for(int x = 0; x < f->currentPiece.size; x++){
			if(f->currentPiece.cells[f->currentPiece.size * y + x]){
				renderSilhouetteCell(renderer, f->pieceX + x, silhouetteY + y, f->cellSize, colors[f->currentPiece.type]);
			}
		}
	}


	// Render the current piece
	for(int y = 0; y < f->currentPiece.size; y++){
		for(int x = 0; x < f->currentPiece.size; x++){
			if(f->currentPiece.cells[f->currentPiece.size * y + x]){
				renderCell(renderer, f->pieceX + x, f->pieceY + y, f->cellSize, colors[f->currentPiece.type]);
			}
		}
	}
}

void destroyField(struct field *f){
	free(f);
}

void checkLines(struct field *f){
	int linesCleared = 0;
	for(int y = 0; y < f->height; y++){
		bool lineFull = true;
		for(int x = 0; x < f->width; x++){
			if(f->cells[f->width*y + x] == EMPTY){
				lineFull = false;
				break;
			}
		}

		if(!lineFull){
			continue;
		}

		linesCleared++;

		// SDL_Log("Clearing line %d\n", y);
		// If the line is filled, clear it
		for(int x = 0; x < f->width; x++){
			f->cells[f->width * y + x] = LINE_CLEAR;
		}
	}

	if(linesCleared > 0){
		f->lineCleared = true;
		f->lastDrop += f->lineClearDuration;
		f->lineClearTime = SDL_GetTicks();

		// Points for clearing lines
		switch(linesCleared){
		case 1:
			f->score += 100;
			break;
		case 2:
			f->score += 300;
			break;
		case 3:
			f->score += 500;
			break;
		case 4:
			f->score += 800;
			break;
		}

		SDL_Log("Score: %d\n", f->score);
	}
}

// Clears lines marked with LINE_CLEAR
void clearLines(struct field *f){
	for(int y = 0; y < f->height; y++){
		// If a line was cleared, all of its cells would be LINE_CLEAR, so just check the first one
		if(f->cells[f->width * y] == LINE_CLEAR){
			// Set cells in cleared line to EMPTY
			for(int x = 0; x < f->width; x++){
				f->cells[f->width * y + x] = EMPTY;
			}

			for(int row = y-1; row >= -2; row--){
				for(int x = 0; x < f->width; x++){
					f->cells[ (row+1)*f->width + x ] = f->cells[ row*f->width + x ];
				}
			}
		}
	}
}


// Spawns a random piece
void spawnRandom(struct field *f){
	spawnPiece(f, rand() % 7 + 1);
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

	f->currentPiece.type = p;

	f->pieceX = (f->width - f->currentPiece.size) / 2;
	f->pieceY = -2;
	f->lastDrop = SDL_GetTicks();
}

void lockPiece(struct field *f){
	if(f->pieceY < -1){
		SDL_Log("Game Over\n");
		return;
	}
	
	for(int y = 0; y < f->currentPiece.size; y++){
		for(int x = 0; x < f->currentPiece.size; x++){
			if(f->currentPiece.cells[y * f->currentPiece.size + x]){
				int fieldX = x + f->pieceX;
				int fieldY = y + f->pieceY;
				f->cells[ fieldY * f->width + fieldX ] = f->currentPiece.type;
			}
		}
	}

	checkLines(f);

	spawnRandom(f);
}

void updateField(struct field *f){
	unsigned int currentTime = SDL_GetTicks();

	if(f->lineCleared){
		if(currentTime - f->lineClearTime > f->lineClearDuration){
			clearLines(f);
			f->lineCleared = false;
		}
		else{
			return;
		}
	}

	if(currentTime - f->lastDrop > f->dropTime){
		moveDown(f);
		f->lastDrop = currentTime;
	}
}

// Returns true if there will be a collision if the piece is moved by dx cells horizontally and dy cells vertically,
bool checkCollision(struct field *f, int dx, int dy){
	for(int y = 0; y < f->currentPiece.size; y++){
		for(int x = 0; x < f->currentPiece.size; x++){
			if(f->currentPiece.cells[y * f->currentPiece.size + x]){
				int fieldX = x + f->pieceX + dx;
				int fieldY = y + f->pieceY + dy;
				if(fieldY >= 0 && f->cells[ fieldY * f->width + fieldX ] != EMPTY){
					// Collision with a non-empty cell on the field
					return true;
				}
				if(fieldX < 0 ){
					// Collision with the left edge of the field
					return true;
				}
				if(fieldX >= f->width){
					// Collision with the right edge of the field
					return true;
				}
				if(fieldY >= f->height){
					//Collision with the bottom edge of the field
					return true;
				}
			}
		}
	}

	// No collision
	return false;
}

// Shifts the current piece left (negative x) or right (positive x) if possible
// Returns true if piece was successfully shifted, false if not
bool shiftPiece(struct field *f, int x){
	// If shifting the piece would cause a collision, don't shift it and return false
	if(checkCollision(f, x, 0)){
		return false;
	}

	// If there will not be a collision, shift the piece and return true
	f->pieceX += x;

	return true;
}


// Moves the current piece down if possible
// Returns true if piece was successfully shifted, false if not
bool moveDown(struct field *f){
	// Set lastDrop to current time to reset counter until next drop
	f->lastDrop = SDL_GetTicks();

	// If moving the piece down would cause a collision, lock it into the field, spawning a new piece, and return false
	if(checkCollision(f, 0, 1)){
		lockPiece(f);
		return false;
	}

	// If there will not be a collision, move the piece down and return true
	f->pieceY += 1;

	return true;
}

// Returns the y coordinate that the current piece will land if not shifted
// Used for showing the silhouette of where the piece will land
int getLandingY(struct field *f){
	int dy = 0;

	while(!checkCollision(f, 0, dy)){
		dy++;
	}

	// pieceY + dy is where collision occurs, so return last y value with no collision
	return f->pieceY + dy - 1;
}

// Drops the piece as far as it can go and locks it in
// Returns how many spaces down it moved
int hardDrop(struct field *f){
	int count = 0;
	// Move piece down until it can't anymore
	while(moveDown(f)){
		count++;
	}

	return count;
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

	if(checkCollision(f, 0, 0)){
		f->currentPiece = oldPiece;
	}
}