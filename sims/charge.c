#include "../maxwell.h"
#include "../render.h"

#define SUBSTEPS 10

#define COLOR_SCALE 500*4

/////////////
// UI code //
/////////////


int main(int argc, char** argv) {
	// Open a window
	Window window = window_open(1000, 600);
	
	World w = empty_world(100, 100, 0.1);
	w.color_scale = 2000;
	
	// Absorbing bounries	
	for (int y = 0; y < w.h; y++) {
		for (int dx = 0; dx < 6; dx++) {
			w.conductivity[dx][y] = 6;
			w.conductivity[w.w-dx-1][y] = 6;
		}
	}

	for (int x = 0; x < w.w; x++) {
		for (int dy = 0; dy < 6; dy++) {
			w.conductivity[x][dy] = 6;
			w.conductivity[x][w.h-dy-1] = 6;
		}
	}
	
	float t = 0;	
	float dt = 1.0/60;
	while (1) {
		// Handle inputs
		do_input(&window);	
		
		for (int i = 0; i < SUBSTEPS; i++) {
			simulate_em(&w, dt/SUBSTEPS);
			t += dt/SUBSTEPS;
		}

		// Add current to exite em waves
		if (t < 1) for (int y = 45; y <= 55; y++) w.field_j[50][y].y = 0.1;
		
		renderer_setup(&window, w.w, w.h);
	
		SDL_LockSurface(window.canvas);
		for (int x = 0; x < w.w; x++) {
			for (int y = 0; y < w.h; y++) {
				RGB c = colorgrade_em(&w, x,y);
				set_pixel(&window, x, y, clamp_to_rgb(c.r), clamp_to_rgb(c.g), clamp_to_rgb(c.b));
			}
		}

		SDL_UnlockSurface(window.canvas);
		
		window_present(&window);
	}

	free_world(w);

	return 0;
}
