#ifndef INCLUDEsrcgame_renderergame_renderer
#define INCLUDEsrcgame_renderergame_renderer

#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>

typedef struct {
    bool running;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Time prevFrameTime;
} Game;

Uint32 FPSTimerCallback(void *userdata, SDL_TimerID timerID, Uint32 interval);
bool initGameRenderer(Game *game);
bool destroyGameRenderer(Game *game);
void gameDrawRGB(Game *game);
void drawGameStateSync(Game *game);
void checkEvents(Game *game);

#endif // INCLUDEsrcgame_renderergame_renderer.h_
