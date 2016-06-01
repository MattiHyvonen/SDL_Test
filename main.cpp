#include <SDL2/SDL.h>
#include <thread>
#include <iostream>

#include "random.h"
#include "dClock.h"
#include "libnnInterface.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

bool quit = false;

struct mouseState{
	float x;
	float y;
	int x_px;
	int y_px;
	mouseState() : x(0), y(0), x_px(0), y_px(0) {}
} mouse;


struct settings{
	int w;
	int h;
	settings() : w(640), h(480) {}
} S;


SDL_Rect makeRect(int x_, int y_, int w_, int h_) {
	SDL_Rect result;
	result.x = x_;
	result.y = y_;
	result.w = w_;
	result.h = h_;
	return result;
}


struct rectT{
	const int MAX_SIZE = 400;
	
	float x;
	float y;
	float size;
	rectT() : x(0.5), y(0.5), size(0.5) {}

	SDL_Rect get() {
		return makeRect(
			S.w * x - (MAX_SIZE / 2)*size,
			S.h * y - (MAX_SIZE / 2)*size,
			MAX_SIZE * size,
			MAX_SIZE * size
			);
	}
} rect;


struct cursorT{
	float size = 10;
	
	SDL_Rect get() {
		return makeRect(
			mouse.x_px - size / 2,
			mouse.y_px - size / 2,
			size, size
			);			
	}
} cursor;


void handleEvent(SDL_Event e) {
	
	const float XY_MUUTOS = 1.0f / 50;
	const float SIZE_MUUTOS = 1.0f / 50;
	
	if (e.type == SDL_QUIT) {
		quit = true;
	}

	else if (e.type == SDL_MOUSEMOTION) {
		int mousex_px, mousey_px;
		
		SDL_GetMouseState(&(mouse.x_px) , &(mouse.y_px) );

		mouse.x = mouse.x_px / S.w;
		mouse.y = mouse.y_px / S.h;
	}


	else if (e.type == SDL_KEYDOWN) {
		switch (e.key.keysym.sym) {
		case SDLK_LEFT:
			rect.x -= XY_MUUTOS;
			break;
		case SDLK_RIGHT:
			rect.x += XY_MUUTOS;
			break;
		case SDLK_UP:
			rect.y -= XY_MUUTOS;
			break;
		case SDLK_DOWN:
			rect.y += XY_MUUTOS;
			break;
		case SDLK_KP_PLUS:
			rect.size += SIZE_MUUTOS;
			break;
		case SDLK_KP_MINUS:
			rect.size -= SIZE_MUUTOS; 
			break;
		}
	
		bound(rect.x, 0, 1);
		bound(rect.y, 0, 1);
		bound(rect.size, SIZE_MUUTOS, 1);

	}
		

}


void initialize() {
	
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("events", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, S.w, S.h, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, 0);

	nnInterface::Init();
}


void draw() {
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 200, 40, 0, 255);
	SDL_RenderFillRect(renderer, &(rect.get()));

	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 20);
	SDL_RenderFillRect(renderer, &(cursor.get()));

	SDL_RenderPresent(renderer);
}



int main(int argc, char* argv[]) {

	initialize();
	
	dClock t;

	std::thread nnThread(nnInterface::StartRoutine);

	while (!quit) {
		
		t.reset();

		//get event
		SDL_Event e;
		while (SDL_PollEvent(&e))
			handleEvent(e);
		
		//draw
		draw();

		//wait to 30 fps
		t.delay(30);
		
	}

	std::cout << "exiting...\n";

	nnInterface::Close();
	nnThread.join();
	std::cout << "neural net closed\n";

	SDL_Quit();
	std::cout << "SDL closed\n";

	return 0;
}