#include "iostream"
#include "set"
#include "vector"
#include "string"
#include "random"
#include "fstream"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"

// Constants
const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 900;
const char* WINDOW_TITLE = "!!! RUN AWAY !!!";

// Colors
SDL_Color White = { 255, 255, 255 }, Black = { 0, 0, 0 };

const int DEFAULT_STEP = 5;
const int DEFAULT_SWORD_LENGTH = 70;
const double DEFAULT_ANGLE_STEP = 0.03, PI = 3.141593;

using namespace std;

void logErrorAndExit(const char* msg, const char* error)
    {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "%s: %s", msg, error);
        SDL_Quit();
    }

SDL_Window* initSDL(int SCREEN_WIDTH, int SCREEN_HEIGHT, const char* WINDOW_TITLE)
    {
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) { logErrorAndExit("SDL_Init", SDL_GetError()); }

        SDL_Window* window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        
        if (window == nullptr) { logErrorAndExit("CreateWindow", SDL_GetError()); }

        if (!IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG)) { logErrorAndExit( "SDL_image error:", IMG_GetError()); }

        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) { logErrorAndExit("SDL_mixer Error: %s\n", Mix_GetError()); }

        if (TTF_Init() == -1) { logErrorAndExit("SDL_ttf could not initialize! SDL_ttf Error: ", TTF_GetError()); }

        return window;
    }

SDL_Renderer* createRenderer(SDL_Window* window)
    {
        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        if (renderer == nullptr) logErrorAndExit("CreateRenderer", SDL_GetError());

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

        return renderer;
    }

void quitSDL(SDL_Window* window, SDL_Renderer* renderer)
    {
        IMG_Quit();
        Mix_Quit();
        TTF_Quit();

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

void waitUntilKeyPressed()
    {
        SDL_Event e;
        while (true) {
            if ((SDL_PollEvent(&e) != 0) && ((e.type == SDL_KEYDOWN) || (e.type == SDL_QUIT))) { return; }
            SDL_Delay(100);
        }
    }

void clrscr(SDL_Renderer* renderer)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

SDL_Texture* loadTexture(const char* filename, SDL_Renderer* renderer)
    {
        SDL_Texture* texture = IMG_LoadTexture(renderer, filename);
        if (texture == NULL) { SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "Load texture %s", IMG_GetError()); }

        return texture;
    }

Mix_Chunk* loadSound(const char* path) 
    {
        Mix_Chunk* gChunk = Mix_LoadWAV(path);
        if (gChunk == nullptr) { SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "Could not load sound! SDL_mixer Error: %s", Mix_GetError()); }
    
        return gChunk;
    }

void playSound(Mix_Chunk* gChunk) 
    {
        if (gChunk != nullptr) { Mix_PlayChannel(-1, gChunk, 0); }
    }

Mix_Music* loadMusic(const char* path)
    {
        Mix_Music* gMusic = Mix_LoadMUS(path);
        if (gMusic == nullptr) { SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "Could not load music! SDL_mixer Error: %s", Mix_GetError()); }

        return gMusic;
    }

void playMusic(Mix_Music* gMusic)
    {
        if (gMusic == nullptr) { return; }

        if (Mix_PlayingMusic() == 0) { Mix_PlayMusic(gMusic, -1); }
        else if (Mix_PausedMusic() == 1) { Mix_ResumeMusic(); }
    }

TTF_Font* loadFont(const char* path, int size)
    {
        TTF_Font* gFont = TTF_OpenFont(path, size);
        if (gFont == nullptr) { SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "Load font %s", TTF_GetError()); }
    
        return gFont;
    }



std::mt19937_64 gen(_getpid());

int randLL(int LIM1, int LIM2)
{
    std::uniform_int_distribution<int> distrib(LIM1, LIM2);

    return distrib(gen);
}

double randLD(double LIM1, double LIM2, int DECIMAL)
{
    std::uniform_real_distribution<long double> distrib(LIM1, LIM2);

    double P = 1.0;
    for (int i = 1; i <= DECIMAL; i++) { P *= 10.0; }

    return round(distrib(gen) * P) / P;
}