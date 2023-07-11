#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

/////////////////////////
// Graphics Primitives //
/////////////////////////

// A wrapper for a window, and a pixel buffer for rendering
// Drawing should be done to the canvas surface.
typedef struct Window {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* canvas;
	SDL_Texture* canvas_texture;
	// A filename stub for renderign to a file, set this to save to a file
	char* target;
	int frame;
	int w, h;
} Window;

// Open a window an create a Window stuct
Window window_open(int w, int h);

// Create a Window struct exclusivly for rendering to a file
Window window_open_file(char* target);

// (re)prepare a buffer for rendering, does nothing if it is already initalized at the same resolution
void renderer_setup(Window* window, int w, int h);

// Show the graphics draw in the pixel buffer to the screen
void window_present(Window* window);

void set_pixel(Window* window, int x, int y, int r, int g, int b);

// Placeholder input handling function
void do_input(Window* window);

