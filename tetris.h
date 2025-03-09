#pragma once

#include <SDL3/SDL.h>

#define MAX_SIZE 4

enum PieceType {
	I_PIECE,
	J_PIECE,
	L_PIECE,
	O_PIECE,
	S_PIECE,
	Z_PIECE,
	T_PIECE
};

struct tetromino {
	size_t size;
	SDL_Color color;
	char cells[MAX_SIZE * MAX_SIZE];
};

struct field {
	int width;
	int height;
	int cellSize;

	int dropTime;
	int lastDrop;

	int pieceX;
	int pieceY;

	struct tetromino currentPiece;
	SDL_Color buffer[100];
	SDL_Color cells[];
};

struct field *createField(size_t width, size_t height, size_t cellSize, unsigned int dropTime);
void renderField(SDL_Renderer *renderer, struct field *f);
void destroyField(struct field *f);

void spawnPiece(struct field *f, enum PieceType p);
void updateField(struct field *f);
bool movePiece(struct field *f, int x, int y);
bool rotatePiece(struct field *f, bool clockwise);
void hardDrop(struct field *f);