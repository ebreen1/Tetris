#pragma once

#include <SDL3/SDL.h>

#define MAX_SIZE 4

enum PieceType {
	EMPTY,
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
	char cells[MAX_SIZE * MAX_SIZE];
	enum PieceType type;
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
	enum PieceType buffer[100];
	enum PieceType cells[];
};

struct field *createField(size_t width, size_t height, size_t cellSize, unsigned int dropTime);
void renderField(SDL_Renderer *renderer, struct field *f);
void destroyField(struct field *f);

void spawnRandom(struct field *f);
void spawnPiece(struct field *f, enum PieceType p);
void updateField(struct field *f);
bool shiftPiece(struct field *f, int x);
bool moveDown(struct field *f);
bool rotatePiece(struct field *f, bool clockwise);
int hardDrop(struct field *f);