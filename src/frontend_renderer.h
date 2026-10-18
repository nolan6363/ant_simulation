#ifndef INCLUDEsrcgame_renderergame_renderer
#define INCLUDEsrcgame_renderergame_renderer

#include "simulation.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>

typedef struct {
    bool running;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Time prevFrameTime;
} Frontend;

Uint32 FPSTimerCallback(void *userdata, SDL_TimerID timerID, Uint32 interval);
bool initFrontendRenderer(Frontend *frontend);
bool destroyFrontendRenderer(Frontend *frontend);
void frameDrawRGB(Frontend *frontend);
void renderCircle(Frontend *frontend, SDL_FPoint center, float radius);
void drawGrid(Frontend *frontend, const Sim sim);
void drawFrameSync(Frontend *frontend, const Sim sim);
void drawFrame(Frontend *frontend, const Sim sim);
void checkEvents(Frontend *frontend);

#endif // INCLUDEsrcgame_renderergame_renderer.h_
