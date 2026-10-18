#include "game_renderer.h"
#include "constants.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

Uint32 FPSTimerCallback(void *userdata, SDL_TimerID timerID, Uint32 interval) {
    SDL_Event newFrameEvent;
    SDL_zero(newFrameEvent);
    newFrameEvent.type = SDL_EVENT_USER;
    newFrameEvent.user.code = 2;
    newFrameEvent.user.data1 = NULL;
    newFrameEvent.user.data2 = NULL;
    SDL_PushEvent(&newFrameEvent);

    return interval;
}

bool initGameRenderer(Game *game) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Unable to initialize SDL : %s", SDL_GetError());
        return false;
    }

    if (!SDL_CreateWindowAndRenderer("Ant Simulation", 640, 480,
                                     SDL_WINDOW_RESIZABLE, &game->window,
                                     &game->renderer)) {
        printf("Couldn't create window/renderer: %s", SDL_GetError());
        return false;
    }
    SDL_SetRenderLogicalPresentation(game->renderer, 640, 480,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    game->running = true;
    game->prevFrameTime = SDL_GetTicksNS();
    return true;
}

bool destroyGameRenderer(Game *game) {
    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);

    SDL_Quit();

    return true;
}

void gameDrawRGB(Game *game) {
    const double now = ((double)SDL_GetTicks()) / 1000.0; // time in seconds
    SDL_SetRenderDrawColorFloat(game->renderer, 0.5 + 0.5 * sin(now),
                                0.5 + 0.5 * sin(now + M_PI * 2.0 / 3.0),
                                0.5 + 0.5 * sin(now + M_PI * 4.0 / 3.0), 1.0f);
    SDL_RenderClear(game->renderer);
    SDL_RenderPresent(game->renderer);
}

void drawGameStateSync(Game *game) {
    SDL_Time nextFrameDelay =
        (1e9 / TARGET_FPS) - (SDL_GetTicksNS() - game->prevFrameTime);
    SDL_DelayNS(nextFrameDelay);
    game->prevFrameTime = SDL_GetTicksNS();
    gameDrawRGB(game);
}

void checkEvents(Game *game) {
    SDL_Event event = {0};
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            game->running = false;
        }
    }
}
