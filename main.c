#include "tetris.h"
#include <SDL3/SDL.h>
#include <time.h>
#include <stdlib.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#define DROP_TIME 1000
#define LINE_CLEAR_TIME 500

struct field *playField = NULL;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

#define FIELD_WIDTH 10
#define FIELD_HEIGHT 20
#define CELL_SIZE 12
#define SCALE 4

#define WINDOW_WIDTH FIELD_WIDTH * CELL_SIZE * SCALE
#define WINDOW_HEIGHT FIELD_HEIGHT * CELL_SIZE * SCALE

/*
TODO (in rough order of priority)
 - 'random bag' generation
 - wall kicks
 - score
 - hold
 - piece preview
*/


void logError(const char *msg){
	SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", msg, SDL_GetError());
}


SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv){
	srand(time(0));
	if(!SDL_SetAppMetadata("SDL3 Tetris", NULL, "tetris.sdl")){
		logError("SDL_SetAppMetadata");
		return SDL_APP_FAILURE;
	}

	if(!SDL_Init(SDL_INIT_VIDEO)){
		logError("SDL_Init");
		return SDL_APP_FAILURE;
	}

	if(!SDL_CreateWindowAndRenderer("Tetris", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)){
		logError("SDL_CreateWindowAndRenderer");
		return SDL_APP_FAILURE;
	}

	SDL_SetRenderScale(renderer, SCALE, SCALE);

	playField = createField(FIELD_WIDTH, FIELD_HEIGHT, CELL_SIZE, DROP_TIME, LINE_CLEAR_TIME);

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate){
	updateField(playField);

	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(renderer);

	renderField(renderer, playField);

	SDL_RenderPresent(renderer);

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event){
	if(event->type == SDL_EVENT_QUIT){
		return SDL_APP_SUCCESS;
	}
	else if(event->type == SDL_EVENT_KEY_DOWN){
		switch(event->key.scancode){
		case SDL_SCANCODE_LEFT:
			shiftPiece(playField, -1);
			break;
		case SDL_SCANCODE_RIGHT:
			shiftPiece(playField, 1);
			break;
		case SDL_SCANCODE_UP:
			// Rotate piece clockwise
			rotatePiece(playField, true);
			break;
		case SDL_SCANCODE_DOWN:
			moveDown(playField);
			break;
		case SDL_SCANCODE_SPACE:
			hardDrop(playField);
			break;
		case SDL_SCANCODE_Z:
			// Rotate piece counter-clockwise
			rotatePiece(playField, false);
			break;
		}
	}

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result){
	destroyField(playField);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}
