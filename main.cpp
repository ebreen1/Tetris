#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

SDL_Window *window;
SDL_Renderer *renderer;

unsigned int lastTime = 0, currentTime, elapsedTime;
constexpr unsigned int FPS = 60;
constexpr unsigned int frameTime = 1000 / FPS;

unsigned int lastMove = 0;
constexpr unsigned int moveDuration = 1000;

#define FIELD_WIDTH 10
#define FIELD_HEIGHT 20

// stores cell colors (default is black)
unsigned int field[FIELD_HEIGHT][FIELD_WIDTH];
constexpr unsigned int CELL_SIZE = 12;
constexpr unsigned int SCALE = 4;

constexpr unsigned int WINDOW_WIDTH = FIELD_WIDTH * CELL_SIZE * SCALE;
constexpr unsigned int WINDOW_HEIGHT = FIELD_HEIGHT * CELL_SIZE * SCALE;

void logError(const char *msg){
	SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", msg, SDL_GetError());
}

int tetrominoCells[4][2] = { {0, 0}, {0, 1}, {0, 2}, {0, 3} };


void drawCell(int x, int y, unsigned int color){
	unsigned char r = (color >> 24) & 0xFF;
	unsigned char g = (color >> 16) & 0xFF;
	unsigned char b = (color >> 8) & 0xFF;
	unsigned char a = color & 0xFF;

	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_FRect dest = { (float)(x * CELL_SIZE), (float)(y * CELL_SIZE), CELL_SIZE, CELL_SIZE };
	SDL_RenderFillRect(renderer, &dest);

	SDL_SetRenderDrawColor(renderer, r*0.8, g*0.8, b*0.8, a*0.8);
	SDL_RenderRect(renderer, &dest);
}

void drawField(){
	for(int y = 0; y < FIELD_HEIGHT; y++){
		for(int x = 0; x < FIELD_WIDTH; x++){
			drawCell(x, y, field[y][x]);
		}
	}
}

void drawTetromino(){
	SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
	for(int i = 0; i < 4; i++){
		int y = tetrominoCells[i][0];
		int x = tetrominoCells[i][1];

		drawCell(x, y, 0xFF00FFFF);
	}
}


void moveTetromino(int x, int y){
	for(int i = 0; i < 4; i++){
		tetrominoCells[i][0] += y;
		tetrominoCells[i][1] += x;
	}
}

void moveDown(){
	for(int i = 0; i < 4; i++){
		if(tetrominoCells[i][0] >= FIELD_HEIGHT - 1){
			return;
		}
	}
	moveTetromino(0, 1);
}


SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv){
	if(!SDL_SetAppMetadata("Test", NULL, "test.sdl")){
		logError("SDL_SetAppMetadata");
		return SDL_APP_FAILURE;
	}

	if(!SDL_Init(SDL_INIT_VIDEO)){
		logError("SDL_Init");
		return SDL_APP_FAILURE;
	}

	if(!SDL_CreateWindowAndRenderer("SDL3 Test", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)){
		logError("SDL_CreateWindowAndRenderer");
		return SDL_APP_FAILURE;
	}

	SDL_SetRenderScale(renderer, SCALE, SCALE);

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate){
	currentTime = SDL_GetTicks();
	elapsedTime = currentTime - lastTime;
	lastTime = currentTime;

	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(renderer);

	if(currentTime - lastMove >= moveDuration){
		moveDown();
		lastMove = currentTime;
	}

	drawField();
	drawTetromino();

	SDL_RenderPresent(renderer);

	if(elapsedTime < frameTime){
		SDL_Delay(frameTime - elapsedTime);
	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event){
	if(event->type == SDL_EVENT_QUIT){
		return SDL_APP_SUCCESS;
	}
	else if(event->type == SDL_EVENT_KEY_DOWN){
		switch(event->key.scancode){
		case SDL_SCANCODE_LEFT:
			moveTetromino(-1, 0);
			break;
		case SDL_SCANCODE_RIGHT:
			moveTetromino(1, 0);
			break;
		}
	}

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result){
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}
