#include "frontend_renderer.h"
#include "constants.h"
#include "simulation.h"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
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

bool initFrontendRenderer(Frontend *frontend) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Unable to initialize SDL : %s", SDL_GetError());
        return false;
    }

    if (!SDL_CreateWindowAndRenderer("Ant Simulation", SCREEN_W, SCREEN_H,
                                     SDL_WINDOW_RESIZABLE, &frontend->window,
                                     &frontend->renderer)) {
        printf("Couldn't create window/renderer: %s", SDL_GetError());
        return false;
    }
    SDL_SetRenderLogicalPresentation(frontend->renderer, SCREEN_W, SCREEN_H,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    frontend->running = true;
    frontend->prevFrameTime = SDL_GetTicksNS();
    return true;
}

bool destroyFrontendRenderer(Frontend *frontend) {
    SDL_DestroyRenderer(frontend->renderer);
    SDL_DestroyWindow(frontend->window);

    SDL_Quit();

    return true;
}

void frameDrawRGB(Frontend *frontend) {
    const double now = ((double)SDL_GetTicks()) / 1000.0; // time in seconds
    SDL_SetRenderDrawColorFloat(frontend->renderer, 0.5 + 0.5 * sin(now),
                                0.5 + 0.5 * sin(now + M_PI * 2.0 / 3.0),
                                0.5 + 0.5 * sin(now + M_PI * 4.0 / 3.0), 1.0f);
    SDL_RenderClear(frontend->renderer);
    SDL_RenderPresent(frontend->renderer);
}

void renderCircle(Frontend *frontend, SDL_FPoint center, float radius) {
    // générer les points du cercle
    int resolution = M_PI * sqrt(radius);

    SDL_Vertex *circleVA =
        (SDL_Vertex *)malloc((resolution + 1) * sizeof(SDL_Vertex));
    int *triangleIndexArray = (int *)malloc(resolution * 3 * sizeof(int));

    float theta = 0;
    SDL_FColor circleColor = {.r = 1.0, .g = 0.0, .b = 0.0, .a = 1.0};
    circleVA[0].position = center;
    circleVA[0].color = circleColor;
    for (int i = 1; i < resolution + 1; i++) {
        circleVA[i].position.x = center.x + radius * cos(theta);
        circleVA[i].position.y = center.y + radius * sin(theta);
        circleVA[i].color = circleColor;
        theta += (2 * M_PI) / resolution;
    }
    for (int i = 0; i < resolution; i++) {
        triangleIndexArray[i * 3] = 0;
        triangleIndexArray[i * 3 + 1] = i + 1;
        triangleIndexArray[i * 3 + 2] = (i + 1) % resolution + 1;
    }

    if (!SDL_RenderGeometry(frontend->renderer, NULL, circleVA, resolution + 1,
                            triangleIndexArray, resolution * 3)) {
        printf("Erreur de rendu de cercle : %s\n", SDL_GetError());
        exit(-1);
    }

    free(circleVA);
    free(triangleIndexArray);
}

void drawFrameSync(Frontend *frontend, const Sim *sim) {
    SDL_Time nextFrameDelay =
        (1e9 / TARGET_FPS) - (SDL_GetTicksNS() - frontend->prevFrameTime);
    SDL_DelayNS(nextFrameDelay);
    frontend->prevFrameTime = SDL_GetTicksNS();
    drawFrame(frontend, sim);
}

void drawFrame(Frontend *frontend, const Sim *sim) {
    SDL_Color clearColor = {.r = 0, .g = 0, .b = 0, .a = 255};
    SDL_SetRenderDrawColor(frontend->renderer, clearColor.r, clearColor.g,
                           clearColor.b, clearColor.a);
    SDL_RenderClear(frontend->renderer);

    SDL_Color colonyColor = {.r = 255, .g = 0, .b = 0, .a = 255};
    SDL_SetRenderDrawColor(frontend->renderer, colonyColor.r, colonyColor.g,
                           colonyColor.b, colonyColor.a);

    SDL_FPoint circleCenter = {.x = 100.0, .y = 100.0};
    renderCircle(frontend, circleCenter, 50.0);

    SDL_RenderPresent(frontend->renderer);
}

void checkEvents(Frontend *frontend) {
    SDL_Event event = {0};
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            frontend->running = false;
        }
    }
}
