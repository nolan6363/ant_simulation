#include "frontend_renderer.h"
#include "constants.h"
#include "simulation.h"
#include "utils.h"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

SDL_FPoint toScreen(Coord2D pointCell) {
    float cellScreenSize = (float)SCREEN_W / (float)SIM_W;
    SDL_FPoint pointScreen = {
        pointCell.x * cellScreenSize + cellScreenSize / 2.0,
        pointCell.y * cellScreenSize + cellScreenSize / 2.0};

    return pointScreen;
}

Coord2D toSim(SDL_FPoint pointScreen) {
    float cellScreenSize = (float)SCREEN_W / (float)SIM_W;
    Coord2D simCell = {pointScreen.x / cellScreenSize,
                       pointScreen.y / cellScreenSize};

    if (simCell.x < 0 || simCell.x >= SIM_W || simCell.y < 0 ||
        simCell.y >= SIM_H) {
        return (Coord2D){0, 0};
    }

    return simCell;
}

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
                                     SDL_WINDOW_FULLSCREEN, &frontend->window,
                                     &frontend->renderer)) {
        printf("Couldn't create window/renderer: %s", SDL_GetError());
        return false;
    }
    SDL_SetRenderLogicalPresentation(frontend->renderer, SCREEN_W, SCREEN_H,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    if (!TTF_Init()) {
        printf("Unable to initialize SDL_ttf: %s\n", SDL_GetError());
        return false;
    }
    frontend->font = TTF_OpenFont(DEBUG_FONT_PATH, DEBUG_FONT_SIZE);
    if (!frontend->font) {
        printf("Unable to open %s (%s), using embedded font\n",
               DEBUG_FONT_PATH, SDL_GetError());
        frontend->font = TTF_OpenFontIO(
            SDL_IOFromConstMem(tiny_ttf, tiny_ttf_len), true, 12.0f);
    }
    if (!frontend->font) {
        printf("Unable to open font : %s\n", SDL_GetError());
        return false;
    }

    if (!initStats(&frontend->frameStats) ||
        !initStats(&frontend->tickStats)) {
        printf("Unable to allocate stats\n");
        return false;
    }

    frontend->renderAnt = true;
    frontend->renderPhA = true;
    frontend->renderPhB = true;
    frontend->headless = false;
    frontend->headlessSurface = NULL;

    frontend->prevFrameTime = SDL_GetTicksNS();
    return true;
}

// pas de fenêtre ni de GPU : le renderer logiciel dessine directement dans une
// surface en mémoire, ce qui permet de réutiliser toutes les fonctions drawXxx
bool initHeadlessRenderer(Frontend *frontend) {
    if (!SDL_Init(0)) {
        printf("Unable to initialize SDL : %s\n", SDL_GetError());
        return false;
    }

    frontend->window = NULL;
    frontend->font = NULL;
    frontend->headless = true;

    frontend->headlessSurface =
        SDL_CreateSurface(SCREEN_W, SCREEN_H, SDL_PIXELFORMAT_XRGB8888);
    if (!frontend->headlessSurface) {
        printf("Unable to create surface : %s\n", SDL_GetError());
        return false;
    }

    frontend->renderer = SDL_CreateSoftwareRenderer(frontend->headlessSurface);
    if (!frontend->renderer) {
        printf("Unable to create software renderer : %s\n", SDL_GetError());
        return false;
    }

    if (!initStats(&frontend->frameStats) ||
        !initStats(&frontend->tickStats)) {
        printf("Unable to allocate stats\n");
        return false;
    }

    frontend->renderAnt = true;
    frontend->renderPhA = true;
    frontend->renderPhB = true;

    frontend->prevFrameTime = SDL_GetTicksNS();
    return true;
}

bool saveFrame(Frontend *frontend, const Sim sim, const char *path) {
    drawFrame(frontend, sim);
    if (!SDL_SaveBMP(frontend->headlessSurface, path)) {
        printf("Unable to save %s : %s\n", path, SDL_GetError());
        return false;
    }
    return true;
}

bool destroyFrontendRenderer(Frontend *frontend) {
    destroyStats(&frontend->frameStats);
    destroyStats(&frontend->tickStats);

    if (frontend->font)
        TTF_CloseFont(frontend->font);
    TTF_Quit();

    SDL_DestroyRenderer(frontend->renderer);
    if (frontend->window)
        SDL_DestroyWindow(frontend->window);
    if (frontend->headlessSurface)
        SDL_DestroySurface(frontend->headlessSurface);

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

void renderCircle(Frontend *frontend, SDL_FPoint center, float radius,
                  SDL_FColor color) {
    // générer les points du cercle
    int resolution = M_PI * sqrt(radius);

    SDL_Vertex *circleVA =
        (SDL_Vertex *)malloc((resolution + 1) * sizeof(SDL_Vertex));
    int *triangleIndexArray = (int *)malloc(resolution * 3 * sizeof(int));

    float theta = 0;
    circleVA[0].position = center;
    circleVA[0].color = color;
    for (int i = 1; i < resolution + 1; i++) {
        circleVA[i].position.x = center.x + radius * cos(theta);
        circleVA[i].position.y = center.y + radius * sin(theta);
        circleVA[i].color = color;
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

void drawGrid(Frontend *frontend, const Sim sim) {
    SDL_Color gridColor = {.r = 30, .g = 30, .b = 30, .a = 255};
    SDL_SetRenderDrawColor(frontend->renderer, gridColor.r, gridColor.g,
                           gridColor.b, gridColor.a);

    for (int x = 0; x < SIM_W; x++) {
        SDL_RenderLine(frontend->renderer, ((float)SCREEN_W / (float)SIM_W) * x,
                       0.0, ((float)SCREEN_W / (float)SIM_W) * x, SCREEN_H);
    }
    for (int y = 0; y < SIM_H; y++) {
        SDL_RenderLine(frontend->renderer, 0.0,
                       ((float)SCREEN_H / (float)SIM_H) * y, SCREEN_W,
                       ((float)SCREEN_H / (float)SIM_H) * y);
    }
}

void drawPheromones(Frontend *frontend, const Sim sim) {
    float cellScreenSize = (float)SCREEN_W / (float)SIM_W;
    for (int y = 0; y < SIM_H; y++) {
        for (int x = 0; x < SIM_W; x++) {
            if ((sim.simGrid[y][x].pheromoneA > 0.05 && frontend->renderPhA) ||
                (sim.simGrid[y][x].pheromoneB > 0.05 && frontend->renderPhB)) {
                SDL_FColor pheromoneAColor = {
                    .r = frontend->renderPhA * sim.simGrid[y][x].pheromoneA,
                    .g = 0.5 * sim.simGrid[y][x].pheromoneA *
                             frontend->renderPhA +
                         0.5 * sim.simGrid[y][x].pheromoneB *
                             frontend->renderPhB,
                    .b = frontend->renderPhB * sim.simGrid[y][x].pheromoneB,
                    .a = 1.0};
                SDL_FPoint pheromoneCenter = toScreen((Coord2D){x, y});
                renderCircle(frontend, pheromoneCenter, cellScreenSize / 2,
                             pheromoneAColor);
            }
        }
    }
}

void drawColony(Frontend *frontend, const Sim sim) {
    float cellScreenSize = (float)SCREEN_W / (float)SIM_W;
    SDL_FPoint colonyCenter = toScreen(sim.colony);
    SDL_FColor colonyColor = {.r = 1.0, .g = 0, .b = 0, .a = 1.0};
    renderCircle(frontend, colonyCenter, COLONY_RADIUS * cellScreenSize,
                 colonyColor);
}

void drawFood(Frontend *frontend, const Sim sim) {
    float cellScreenSize = (float)SCREEN_W / (float)SIM_W;
    for (int y = 0; y < SIM_H; y++) {
        for (int x = 0; x < SIM_W; x++) {
            if (sim.simGrid[y][x].food > 0.0) {
                SDL_FColor foodColor = {.r = sim.simGrid[y][x].food,
                                        .g = sim.simGrid[y][x].food,
                                        .b = 0,
                                        .a = 1.0};
                SDL_FPoint foodCenter = toScreen((Coord2D){x, y});
                renderCircle(frontend, foodCenter, cellScreenSize / 2,
                             foodColor);
            }
        }
    }
}

void drawAnts(Frontend *frontend, const Sim sim) {
    float cellScreenSize = (float)SCREEN_W / (float)SIM_W;
    for (int i = 0; i < N_ANTS; i++) {
        float antColAngle = ((2 * M_PI) / N_ANTS) * i;
        SDL_FColor antColor = {
            .r = sin(antColAngle),
            .g = 0.5 + +0.5 * sin(antColAngle + M_PI * 2.0 / 3.0),
            .b = 0.5 + 0.5 * sin(antColAngle + M_PI * 4.0 / 3.0),
            .a = 1.0};
        SDL_FPoint antCenter = toScreen(sim.ants[i].pos);
        renderCircle(frontend, antCenter, cellScreenSize / 2, antColor);
    }
}

void drawDebug(Frontend *frontend, const Sim sim) {
    float cellScreenSize = (float)SCREEN_W / (float)SIM_W;
    SDL_FColor debugColor = {.r = 1.0, .g = 0.0, .b = 1.0, .a = 1.0};
    for (int y = 0; y < SIM_H; y++) {
        for (int x = 0; x < SIM_W; x++) {
            if (sim.simGrid[y][x].debug_flag == true) {
                SDL_FPoint debugCenter = toScreen((Coord2D){x, y});
                renderCircle(frontend, debugCenter, cellScreenSize / 3,
                             debugColor);
            }
        }
    }
}

void drawWall(Frontend *frontend, const Sim sim) {
    float cellScreenSize = (float)SCREEN_W / (float)SIM_W;
    SDL_FColor debugColor = {.r = 0.6, .g = 0.6, .b = 0.6, .a = 1.0};
    for (int y = 0; y < SIM_H; y++) {
        for (int x = 0; x < SIM_W; x++) {
            if (sim.simGrid[y][x].wall == true) {
                SDL_FPoint debugCenter = toScreen((Coord2D){x, y});
                renderCircle(frontend, debugCenter, cellScreenSize / 3,
                             debugColor);
            }
        }
    }
}

void drawFrameSync(Frontend *frontend, const Sim sim) {
    SDL_Time nextFrameDelay = max(
        (1e9 / TARGET_FPS) - (SDL_GetTicksNS() - frontend->prevFrameTime), 0);
    // SDL_Time nextFrameDelay = 0;
    SDL_DelayNS(nextFrameDelay);
    frontend->frameTime = SDL_GetTicksNS() - frontend->prevFrameTime;
    frontend->prevFrameTime = SDL_GetTicksNS();
    pushStat(&frontend->frameStats, frontend->frameTime / 1e6);
    drawFrame(frontend, sim);
}

void drawFrame(Frontend *frontend, const Sim sim) {
    float cellScreenSize = (float)SCREEN_W / (float)SIM_W;

    SDL_Color clearColor = {.r = 0, .g = 0, .b = 0, .a = 255};
    SDL_SetRenderDrawColor(frontend->renderer, clearColor.r, clearColor.g,
                           clearColor.b, clearColor.a);
    SDL_RenderClear(frontend->renderer);

    drawGrid(frontend, sim);
    drawPheromones(frontend, sim);
    drawColony(frontend, sim);
    drawFood(frontend, sim);
    if (frontend->renderAnt)
        drawAnts(frontend, sim);
    // en headless on veut seulement ce que l'utilisateur verrait, sans debug
    if (!frontend->headless)
        drawDebug(frontend, sim);
    drawWall(frontend, sim);

    if (!frontend->headless)
        drawStats(frontend);

    // en headless, Present vide la file de commandes vers la surface
    SDL_RenderPresent(frontend->renderer);
}

void drawStats(Frontend *frontend) {
    StatsSummary frameSummary = getWindowStats(&frontend->frameStats);
    StatsSummary tickSummary = getWindowStats(&frontend->tickStats);

    char text[5][128] = {0};
    snprintf(text[0], sizeof(text[0]), "Frame time : %7.3f ms   FPS : %4d",
             frontend->frameTime / 1e6,
             (int)(1000.0 / (frontend->frameTime / 1e6)));
    snprintf(text[1], sizeof(text[1]),
             "  avg %7.3f  P95 %7.3f  P99 %7.3f  max %7.3f", frameSummary.mean,
             frameSummary.p95, frameSummary.p99, frameSummary.max);
    snprintf(text[2], sizeof(text[2]), "Tick time  : %7.3f ms   TPS : %4d",
             frontend->tickTime / 1e6,
             (int)(1000.0 / (frontend->tickTime / 1e6)));
    snprintf(text[3], sizeof(text[3]),
             "  avg %7.3f  P95 %7.3f  P99 %7.3f  max %7.3f", tickSummary.mean,
             tickSummary.p95, tickSummary.p99, tickSummary.max);
    snprintf(text[4], sizeof(text[4]), "Window : %zu/%d samples",
             tickSummary.count, STATS_WINDOW_SIZE);

    int nLines = 5;
    int lineSkip = TTF_GetFontLineSkip(frontend->font);
    float margin = 10;
    float padding = 8;

    // fond semi-transparent pour rester lisible au-dessus des fourmis
    int maxWidth = 0;
    for (int i = 0; i < nLines; i++) {
        int w, h;
        if (TTF_GetStringSize(frontend->font, text[i], 0, &w, &h) &&
            w > maxWidth)
            maxWidth = w;
    }
    SDL_BlendMode prevBlendMode;
    SDL_GetRenderDrawBlendMode(frontend->renderer, &prevBlendMode);
    SDL_SetRenderDrawBlendMode(frontend->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(frontend->renderer, 0, 0, 0, 180);
    SDL_FRect background = {margin, margin, maxWidth + 2 * padding,
                            nLines * lineSkip + 2 * padding};
    SDL_RenderFillRect(frontend->renderer, &background);
    SDL_SetRenderDrawBlendMode(frontend->renderer, prevBlendMode);

    for (int i = 0; i < nLines; i++) {
        drawText(frontend, text[i], (SDL_Color){255, 255, 255, 255},
                 (SDL_FPoint){margin + padding,
                              margin + padding + i * lineSkip});
    }
}

void drawText(Frontend *frontend, char *text, SDL_Color textColor,
              SDL_FPoint pos) {
    SDL_Surface *textSurface;
    SDL_Texture *textTexture = NULL;
    textSurface = TTF_RenderText_Blended(frontend->font, text, 0, textColor);

    if (textSurface) {
        textTexture =
            SDL_CreateTextureFromSurface(frontend->renderer, textSurface);
        SDL_DestroySurface(textSurface);
    }
    if (!textTexture) {
        printf("Unable to create text : %s\n", SDL_GetError());
        exit(-1);
    }

    SDL_FRect dst;
    SDL_GetTextureSize(textTexture, &dst.w, &dst.h);
    dst.x = pos.x;
    dst.y = pos.y;

    SDL_SetRenderDrawColor(frontend->renderer, textColor.r, textColor.g,
                           textColor.b, textColor.a);
    SDL_RenderTexture(frontend->renderer, textTexture, NULL, &dst);
    SDL_DestroyTexture(textTexture);
}

void handleEvents(Frontend *frontend, Sim *sim) {
    SDL_Event event = {0};
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
            sim->running = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            switch (event.key.key) {
            case SDLK_ESCAPE:
            case SDLK_Q:
                sim->running = false;
                break;
            case SDLK_SPACE:
                sim->paused = !sim->paused;
                break;
            case SDLK_1:
                frontend->renderAnt = !frontend->renderAnt;
                break;
            case SDLK_2:
                frontend->renderPhA = !frontend->renderPhA;
                break;
            case SDLK_3:
                frontend->renderPhB = !frontend->renderPhB;
                break;
            default:
                continue;
            }
            break;
        case SDL_EVENT_MOUSE_MOTION:
            if (event.motion.state & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK)) {
                SDL_ConvertEventToRenderCoordinates(frontend->renderer, &event);
                Coord2D simCell =
                    toSim((SDL_FPoint){event.motion.x, event.motion.y});
                Coord2D scanStart = {max(0, simCell.x - DEFAULT_BRUSH_SIZE),
                                     max(0, simCell.y - DEFAULT_BRUSH_SIZE)};
                Coord2D scanStop = {min(SIM_W, simCell.x + DEFAULT_BRUSH_SIZE),
                                    min(SIM_H, simCell.y + DEFAULT_BRUSH_SIZE)};
                for (int x = scanStart.x; x < scanStop.x; x++) {
                    for (int y = scanStart.y; y < scanStop.y; y++) {
                        if (isInCircle((Coord2D){x, y}, simCell,
                                       DEFAULT_BRUSH_SIZE)) {
                            if (event.motion.state & SDL_BUTTON_LMASK)
                                sim->simGrid[y][x].wall = true;
                            else if (event.motion.state & SDL_BUTTON_RMASK)
                                sim->simGrid[y][x].wall = false;
                            sim->simGrid[y][x].pheromoneA = 0;
                            sim->simGrid[y][x].pheromoneB = 0;
                        }
                    }
                }
            }
            break;
        default:
            continue;
        }
    }
}
