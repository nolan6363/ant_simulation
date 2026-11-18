#ifndef INCLUDEsrcgame_renderergame_renderer
#define INCLUDEsrcgame_renderergame_renderer

#include "simulation.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    SDL_Time prevFrameTime;
    SDL_Time frameTime;
    SDL_Time tickTime;
    bool renderAnt, renderPhA, renderPhB;
} Frontend;

extern unsigned char tiny_ttf[];
extern unsigned int tiny_ttf_len;

Uint32 FPSTimerCallback(void *userdata, SDL_TimerID timerID, Uint32 interval);
bool initFrontendRenderer(Frontend *frontend);
bool destroyFrontendRenderer(Frontend *frontend);
void frameDrawRGB(Frontend *frontend);
void renderCircle(Frontend *frontend, SDL_FPoint center, float radius,
                  SDL_FColor color);
void drawGrid(Frontend *frontend, const Sim sim);
void drawFrameSync(Frontend *frontend, const Sim sim);
void drawFrame(Frontend *frontend, const Sim sim);
void handleEvents(Frontend *frontend, Sim *sim);
void drawGrid(Frontend *frontend, const Sim sim);
void drawPheromones(Frontend *frontend, const Sim sim);
void drawColony(Frontend *frontend, const Sim sim);
void drawFood(Frontend *frontend, const Sim sim);
void drawAnts(Frontend *frontend, const Sim sim);
void drawDebug(Frontend *frontend, const Sim sim);
void drawText(Frontend *frontend, char *text, SDL_Color textColor,
              SDL_FPoint pos);

#endif // INCLUDEsrcgame_renderergame_renderer.h_
