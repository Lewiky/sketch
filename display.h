// You do not need to change this file.

// The display module provides graphics for the sketch program.
#include <stdbool.h>

// A display structure needs to be created by calling newDisplay, and then
// needs to be passed to each sketching function.
struct display;
typedef struct display display;

// Create a display object representing a plain white window of a given size.
// If test is not NULL, return a display object which does no graphics, but
// tests the calls made to this module.
display *newDisplay(char *title, int width, int height, char *test[]);

// Draw a line from (x0,y0) to (x1,y1) which is black by default.
void line(display *d, int x0, int y0, int x1, int y1);

// Draw a line of a given colour from (x0,y0) to (x1,y1).  The colour is in
// rgba format, i.e. the red value in the leftmost (most significant) byte,
// then green in the next byte, then blue, then opacity (normally 255).
void cline(display *d, int x0, int y0, int x1, int y1, int rgba);

// Pause for the given number of milliseconds.
void pause(display *d, int ms);

// Clear the display to white.
void clear(display *d);

// Wait for a key press.
char key(display *d);

// Hold the display for a few seconds, then shut down.
void end(display *d);
