// You don't need to change this file, or understand anything in it.

#include "display.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct display {
    bool testing;
    char *file;
    int width, height;
    SDL_Window *window;
    SDL_Renderer *renderer;
    char *actual;
    char **expected;
};

// Fail, including the SDL error message if any, and stop the program.
static void fail(char *s1, char *s2) {
    fprintf(stderr, "%s %s %s\n", s1, s2, SDL_GetError());
    SDL_Quit();
    exit(1);
}

// Check an actual call against the next expected call.
static void check(display *d) {
    bool ended = d->expected[0] == NULL;
    if (!ended && strcmp(d->actual, d->expected[0]) == 0) {
        d->expected = d->expected + 1;
    } else {
        fprintf(stderr, "Failure in %s\n", d->file);
        if (ended) fail("Unexpected extra call", d->actual);
        fprintf(stderr, "Bad call  %s\n", d->actual);
        fail("Expecting", d->expected[0]);
    }
}

// Create a dummy display for testing.
static display *testDisplay(char *file, char **expected) {
    display *d = malloc(sizeof(display));
    d->testing = true;
    d->file = file;
    d->expected = expected;
    d->actual = malloc(100);
    return d;
}

display *newDisplay(char *file, int width, int height, char **test) {
    if (test != NULL) return testDisplay(file, test);
    display *d = malloc(sizeof(display));
    d->testing = false;
    d->file = file;
    d->width = width;
    d->height = height;
    int result = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
    if (result < 0) fail("Bad SDL", "");
    int x = SDL_WINDOWPOS_UNDEFINED, y = SDL_WINDOWPOS_UNDEFINED;
    d->window = SDL_CreateWindow("SKETCH", x, y, width, height, 0);
    if (d->window == NULL) fail("Bad window", "");
    SDL_Surface *surface = SDL_GetWindowSurface(d->window);
    d->renderer = SDL_CreateSoftwareRenderer(surface);
    if (d->renderer == NULL) fail("Bad renderer", "");
    SDL_SetRenderDrawColor(d->renderer, 255, 255, 255, 255);
    SDL_RenderClear(d->renderer);
    SDL_UpdateWindowSurface(d->window);
    SDL_Delay(1);
    return d;
}

void line(display *d, int x0, int y0, int x1, int y1) {
    if (d->testing) {
        sprintf(d->actual, "line(d,%d,%d,%d,%d)", x0, y0, x1, y1);
        check(d);
        return;
    }
    SDL_SetRenderDrawColor(d->renderer, 0, 0, 0, 255);
    int rc = SDL_RenderDrawLine(d->renderer, x0, y0, x1, y1);
    if (rc < 0) fail("Bad line", "");
    SDL_UpdateWindowSurface(d->window);
}

void cline(display *d, int x0, int y0, int x1, int y1, int rgba) {
    if (d->testing) {
        sprintf(d->actual, "cline(d,%d,%d,%d,%d,%d)", x0, y0, x1, y1, rgba);
        check(d);
        return;
    }
    int r = (rgba >> 24) & 0xFF;
    int g = (rgba >> 16) & 0xFF;
    int b = (rgba >> 8) & 0xFF;
    int a = rgba & 0xFF;
    int rc = SDL_SetRenderDrawColor(d->renderer, r, g, b, a);
    if (rc < 0) fail("Bad set colour", "");
    rc = SDL_RenderDrawLine(d->renderer, x0, y0, x1, y1);
    if (rc < 0) fail("Bad cline", "");
    SDL_UpdateWindowSurface(d->window);
}

void clear(display *d) {
    if (d->testing) {
        sprintf(d->actual, "clear(d)");
        check(d);
        return;
    }
    SDL_SetRenderDrawColor(d->renderer, 255, 255, 255, 255);
    SDL_RenderClear(d->renderer);
    SDL_UpdateWindowSurface(d->window);
}

void pause(display *d, int ms) {
    if (d->testing) {
        sprintf(d->actual, "pause(d,%d)", ms);
        check(d);
        return;
    }
    if (ms > 0) SDL_Delay(ms);
}

char key(display *d) {
    if (d->testing) {
        sprintf(d->actual, "key(d)");
        check(d);
        return 0;
    }
    SDL_Event event_structure;
    SDL_Event *event = &event_structure;
    while (true) {
        int r = SDL_WaitEvent(event);
        if (r == 0) fail("Bad event", "");
        if (event->type == SDL_QUIT) {
            SDL_Quit();
            exit(0);
        }
        else if (event->type == SDL_KEYUP) {
            int sym = event->key.keysym.sym;
            return (char)sym;
        }
    }
}

void end(display *d) {
    if (d->testing) {
        bool ended = d->expected[0] == NULL;
        if (! ended) {
            fprintf(stderr, "Failure in sketching: %s\n", d->file);
            fail("Expecting further call", d->expected[0]);
        }
        else printf("Sketch %s OK\n", d->file);
    } else {
        SDL_Delay(5000);
        SDL_Quit();
        exit(0);
    }
}
