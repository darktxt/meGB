#include"SDL2/SDL.h"
#include<memory>
#include<emulator.h>

typedef unsigned char byte;

const double TimePerFrame = 1.0 / 60.0;
const unsigned int CyclesPerFrame = 70224;

struct SDLWindowDeleter
{
    void operator()(SDL_Window* window)
    {
        if (window != nullptr)
        {
            SDL_DestroyWindow(window);
        }
    }
};

struct SDLRendererDeleter
{
    void operator()(SDL_Renderer* renderer)
    {
        if (renderer != nullptr)
        {
            SDL_DestroyRenderer(renderer);
        }
    }
};

struct SDLTextureDeleter
{
    void operator()(SDL_Texture* texture)
    {
        if (texture != nullptr)
        {
            SDL_DestroyTexture(texture);
        }
    }
};

void Render(SDL_Renderer* pRenderer, SDL_Texture* pTexture, Emulator* emulator)
{
    // Clear window
    SDL_SetRenderDrawColor(pRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(pRenderer);

    byte* pPixels;
    int pitch = 0;
    SDL_LockTexture(pTexture, nullptr, (void**)&pPixels, &pitch);

    // Render Game
    byte* pData = emulator->GetCurrentFrame();
    memcpy(pPixels, pData, 160 * 144 * 4);

    SDL_UnlockTexture(pTexture);

    SDL_RenderCopy(pRenderer, pTexture, nullptr, nullptr);

    // Update window
    SDL_RenderPresent(pRenderer);
}


int main(int argc, char** argv){

    int winHeight = 144;
    int winWidth = 160;
    int winScale = 4;

    
    SDL_Event event;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        //Logger::LogError("SDL could not initialize! SDL error: '%s'", SDL_GetError());
        return false;
    }
    std::unique_ptr<SDL_Window, SDLWindowDeleter> spWin(
        SDL_CreateWindow(
            "GameLad",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            winWidth * winScale, // Original = 160
            winHeight * winScale, // Original = 144
            SDL_WINDOW_SHOWN));
    if (spWin == nullptr)
    {
        //Logger::LogError("Window could not be created! SDL error: '%s'", SDL_GetError());
        return false;
    }

    std::unique_ptr<SDL_Renderer, SDLRendererDeleter> spRenderer(
        SDL_CreateRenderer(spWin.get(), -1, SDL_RENDERER_ACCELERATED));
    if (spRenderer == nullptr)
    {
        //Logger::LogError("Renderer could not be created! SDL error: '%s'", SDL_GetError());
        return false;
    }

    std::unique_ptr<SDL_Texture, SDLTextureDeleter> spTexture(
        SDL_CreateTexture(spRenderer.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 160, 144));
    
    std::unique_ptr <Emulator> emulator = std::make_unique<Emulator>();
    while (true) {
        Render(spRenderer.get(), spTexture.get(), emulator.get());
    }

    spTexture.reset();
    spRenderer.reset();
    spWin.reset();
    SDL_Quit();
    return 0;
}