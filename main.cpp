#include <SDL2/SDL.h>
#include <thread>
#include <iostream>
#include <vector>

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


enum moodiT{
	KOULUTETAAN, KATSELLAAN, MUOKATAAN, ASETA_TOIVE
} moodi;


void handleEvent(SDL_Event e) {
	
	const float XY_MUUTOS = 1.0f / 50;
	const float SIZE_MUUTOS = 1.0f / 50;
	
	if (e.type == SDL_QUIT) {
		quit = true;
	}

	else if (e.type == SDL_MOUSEMOTION) {
		int mousex_px, mousey_px;
		
		SDL_GetMouseState(&(mouse.x_px) , &(mouse.y_px) );

		mouse.x = (float)mouse.x_px / S.w;
		mouse.y = (float)mouse.y_px / S.h;
	}


	else if (e.type == SDL_KEYDOWN) {
		switch (e.key.keysym.sym) {
		
		//tallenna tilanne
		case SDLK_RETURN:
			if (moodi == MUOKATAAN) {
				moodi = ASETA_TOIVE;
				std::cout << "asetetaan toive\n";
			}
			break;

		//vaihda moodia
		case SDLK_TAB:
			if (moodi == MUOKATAAN) {
				moodi = KATSELLAAN;
				std::cout << "katsellaan\n";
			}
			else if (moodi == KATSELLAAN) {
				moodi = MUOKATAAN;
				std::cout << "muokataan\n";
			}
			break;		

		//muuta neliötä
		case SDLK_z:
			//arvonta
			rect.x = randf(0, 1);
			rect.y = randf(0, 1);
			rect.size = randf(0, 1);
			break;
		case SDLK_LEFT:
			if(moodi == MUOKATAAN) rect.x -= XY_MUUTOS;
			break;
		case SDLK_RIGHT:
			if (moodi == MUOKATAAN) rect.x += XY_MUUTOS;
			break;
		case SDLK_UP:
			if (moodi == MUOKATAAN) rect.y -= XY_MUUTOS;
			break;
		case SDLK_DOWN:
			if (moodi == MUOKATAAN) rect.y += XY_MUUTOS;
			break;
		case SDLK_PERIOD:
			if (moodi == MUOKATAAN) rect.size += SIZE_MUUTOS;
			break;
		case SDLK_COMMA:
			if (moodi == MUOKATAAN) rect.size -= SIZE_MUUTOS;
			break;
		}
	
		bound(rect.x, 0, 1);
		bound(rect.y, 0, 1);
		bound(rect.size, SIZE_MUUTOS, 1);

	}
		

}



void draw() {
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);

    SDL_Rect tmp = rect.get();
	SDL_SetRenderDrawColor(renderer, 200, 40, 0, 255);
	SDL_RenderFillRect(renderer, &(tmp));

    tmp = cursor.get();
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 20);
	SDL_RenderFillRect(renderer, &(tmp));

	SDL_RenderPresent(renderer);
}


void initialize() {

	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("events", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, S.w, S.h, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, 0);

	nnInterface::Init();
}


int main(int argc, char* argv[]) {

	initialize();
	
	dClock t;

	std::thread nnThread(nnInterface::StartRoutine);

	moodi = MUOKATAAN;

	while (!quit) {
		
		t.reset();

		//get event
		SDL_Event e;
		while (SDL_PollEvent(&e))
			handleEvent(e);
		
		std::vector<float> inputs(2);
		inputs[0] = mouse.x;
		inputs[1] = mouse.y;

		std::vector<float> outputs;
		std::vector <float> desiredOuts(3);

		desiredOuts[0] = rect.x;
		desiredOuts[1] = rect.y;
		desiredOuts[2] = rect.size;
		
		nnInterface::mtx.lock();
		
		if (moodi == KATSELLAAN) {
			
			//annetaan inputit nnetille
			nnInterface::SetInput(inputs);

			//haetaan vaste nnetiltä
			while (outputs.empty()) {
				outputs = nnInterface::GetOutput();
				//std::cout << "outputs oli tyhjä\n";
			}
			//std::cout << "tuli output\n";

            std::cout << "outputs: " << outputs[0] << " " << outputs[1] << " " << outputs[3] << "\n";
			
            //skaalaa
            for (int i = 0; i < outputs.size(); i++)
                outputs[i] = outputs[i] * 2 - 1;
            
			//laitetaan saadut arvot neliöön
			rect.x += outputs[0] * 0.05f;
			rect.y += outputs[1] * 0.05f;
			rect.size += outputs[2] * 0.05f;
            
            bound(rect.x,0,1);
            bound(rect.y,0,1);
            bound(rect.size,0.1,1);
            
            
            std::vector<float> nykyinenPaikka(3);
            nykyinenPaikka[0] = rect.x;
            nykyinenPaikka[1] = rect.y;
            nykyinenPaikka[2] = rect.size;
            
            nnInterface::LaskeDesiredOut(nykyinenPaikka);
            
		}
		
		if (moodi == ASETA_TOIVE) {
            nnInterface::SetInput(inputs);
            nnInterface::TeeTilanne(inputs, desiredOuts);
            moodi = MUOKATAAN;
		}

		if (moodi == MUOKATAAN) {//ks handle event
		}

		nnInterface::mtx.unlock();

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