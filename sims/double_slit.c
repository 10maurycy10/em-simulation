#include "../maxwell.h"
#include "../render.h"

#define SUBSTEPS 10

#define COLOR_SCALE 500*4

////////////////
// Simulation //
////////////////

// dE.y / dt = -dB / dx
// dE.x / dt = dB / dy
// dB / dt = dE.x / dy - dE.y / dx (this part is actualy the 2d curl of the electric feild :) )

/////////////
// UI code //
/////////////


int main(int argc, char** argv) {
	// Open a window
	Window window = window_open(1000, 600);
	
	World w = empty_world(100, 60, 0.1);
	w.color_scale = 2000;
	
	// Absorbing bounries	
	for (int y = 0; y < w.h; y++) {
		for (int dx = 0; dx < 6; dx++) {
			w.field_conductivity[dx][y] = 6;
			w.field_conductivity[w.w-dx-1][y] = 6;
		}
	}

	for (int x = 0; x < w.w; x++) {
		for (int dy = 0; dy < 6; dy++) {
			w.field_conductivity[x][dy] = 6;
			w.field_conductivity[x][w.h-dy-1] = 6;
		}
	}
	
	// Setup the double slits
	for (int y = 0; y < w.h; y++) {
		w.field_conductivity[32][y] = 500;
		w.field_conductivity[31][y] = 500;
	}
	
	// Add the slits themselves
	w.field_conductivity[32][24] = 0;
	w.field_conductivity[32][25] = 0;
	w.field_conductivity[31][24] = 0;
	w.field_conductivity[31][25] = 0;
	
	w.field_conductivity[32][43] = 0;
	w.field_conductivity[32][42] = 0;
	w.field_conductivity[31][43] = 0;
	w.field_conductivity[31][42] = 0;

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
		w.field_j[16][32].y = sin(t*10);
		
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
