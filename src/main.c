#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define TARGET_FPS 60

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

Uint32 timerCallback(void *userdata, SDL_TimerID timerID, Uint32 interval) {
    SDL_Event newFrameEvent;
    SDL_zero(newFrameEvent);
    newFrameEvent.type = SDL_EVENT_USER;
    newFrameEvent.user.code = 2;
    newFrameEvent.user.data1 = NULL;
    newFrameEvent.user.data2 = NULL;
    SDL_PushEvent(&newFrameEvent);

    return interval;
}

int main(int argc, char *argv[]) {

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Unable to initialize SDL : %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Ant Simulation", 640, 480,
                                     SDL_WINDOW_RESIZABLE, &window,
                                     &renderer)) {
        printf("Couldn't create window/renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, 640, 480,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_TimerID timer = SDL_AddTimer(1000 / TARGET_FPS, timerCallback, NULL);

    bool running = true;
    while (running) {
        SDL_Event event = {0};
        SDL_WaitEvent(&event);
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        } else if (event.type == SDL_EVENT_USER) {
            if (event.user.code == 2) { // timer event
                const double now =
                    ((double)SDL_GetTicks()) / 1000.0; // time in seconds
                SDL_SetRenderDrawColorFloat(
                    renderer, 0.5 + 0.5 * sin(now),
                    0.5 + 0.5 * sin(now + M_PI * 2.0 / 3.0),
                    0.5 + 0.5 * sin(now + M_PI * 4.0 / 3.0), 1.0f);
                SDL_RenderClear(renderer);
                SDL_RenderPresent(renderer);
            }
        }
    }

    SDL_RemoveTimer(timer);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);

    SDL_Quit();

    return EXIT_SUCCESS;
}
