#include "render.h"
#include <assert.h>
#include <SDL2/SDL_image.h>

// Open a window an create a Window stuct
Window window_open(int w, int h) {
	//int windowFlags = SDL_WINDOW_RESIZABLE;
	int windowFlags = 0;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_Window* window = SDL_CreateWindow("3d", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, windowFlags);

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
		.frame = 0,
		.h = 480,
		.renderer = renderer,
		.canvas = NULL,
		.canvas_texture = NULL,
	};
}

Window window_open_file(char* target) {
	return (Window) {
		.window = NULL,
		.canvas = 0,
		.w = 0,
		.h = 0,
		.frame = 0,
		.renderer = NULL,
		.canvas_texture = NULL,
		.target = target
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
	
	window->canvas = SDL_CreateRGBSurface(0, w, h, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
	assert(window->canvas);
	
	if (window->renderer) {
		window->canvas_texture = SDL_CreateTexture(window->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, w, h);
		assert(window->canvas_texture);
	}
}

// Show the graphics draw in the pixel buffer to the screen
void window_present(Window* window) {
	if (window->renderer) {
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

	if (window->target) {
		// TODO support large filenames
		char filename[256];
		snprintf(filename, 256, "%s%d.png", window->target, window->frame);
		printf("Saving frame to %s\n", filename);
		IMG_SavePNG(window->canvas, filename);
	}

	window->frame++;
}

void set_pixel(Window* window, int x, int y, int r, int g, int b) {
	assert(x >= 0 && x < window->w);
	assert(y >= 0 && y < window->h);
	uint32_t* pixels = (uint32_t*)window->canvas->pixels;
	pixels[x + y * window->w] = r << 24 | g << 16 | b << 8 | 0xff;
}

// Placeholder function to poll SDL events and exit on an quit reqest
void do_input(Window* window) {
        // Check if the user wants to close the window
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
                switch (event.type) {
                        case SDL_QUIT:
                                exit(0);
                                break;
                }
        }

        uint8_t* keyboard = SDL_GetKeyboardState(NULL);
}


