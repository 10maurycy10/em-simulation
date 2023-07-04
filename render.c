#include "render.h"
#include <assert.h>

// Open a window an create a Window stuct
Window window_open() {
	int windowFlags = SDL_WINDOW_RESIZABLE;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_Window* window = SDL_CreateWindow("3d", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640*2, 480*2, windowFlags);

	if (!window) {
		printf("Failed to open window: %s\n", SDL_GetError());
		exit(1);
	}
	
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC); 
	
	if (!renderer) {
		printf("Failed to open renderer: %s\n", SDL_GetError());
		exit(1);
	};

	return (Window) {
		.window = window,
		.w = 640,
		.h = 480,
		.renderer = renderer,
		.canvas = NULL,
		.canvas_texture = NULL,
	};
}

// (re)prepare a buffer for rendering, does nothing if it is already initalized at the same resolution
void renderer_setup(Window* window, int w, int h) {
	// Do nothing if the current resolution is the same, and the struct is initalized.
	if (w == window->w && h == window->h && window->canvas && window->canvas_texture) return;

	window->w = w;
	window->h = h;

	if (window->canvas) SDL_FreeSurface(window->canvas);
	if (window->canvas_texture) SDL_DestroyTexture(window->canvas_texture);
	
	window->canvas = SDL_CreateRGBSurface(0, h, w, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
	assert(window->canvas);
	window->canvas_texture = SDL_CreateTexture(window->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, h, w);
	assert(window->canvas_texture);
}

// Show the graphics draw in the pixel buffer to the screen
void window_present(Window* window) {
	// Copy rendered graphics to the to the gpu
	void* texture_pixels;
	int texture_pitch;
	SDL_LockTexture(window->canvas_texture, NULL, &texture_pixels, &texture_pitch);
	memcpy(texture_pixels, window->canvas->pixels, window->canvas->pitch * window->canvas->h);
	SDL_UnlockTexture(window->canvas_texture);

	// Draw the texture onto the renderer 
	SDL_RenderCopy(window->renderer, window->canvas_texture, NULL, NULL);
	
	// Present the renderer
	SDL_RenderPresent(window->renderer);
}

void set_pixel(Window* window, int x, int y, int r, int g, int b) {
	assert(x >= 0 && x < window->w);
	assert(y >= 0 && y < window->h);
	uint32_t* pixels = (uint32_t*)window->canvas->pixels;
	pixels[x + y * window->w] = r << 24 | g << 16 | b << 8 | 0xff;
}

