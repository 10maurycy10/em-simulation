#include "../maxwell.h"
#include "../render.h"

#define SUBSTEPS 9
//#define IOR 1.1
#define IOR 1.33
//#define IOR 1.52
//#define IOR 2.417
//#define IOR 0.5

// IOR = sqrt( relperm * relpermit )
// relperm ~= 1 for most lense materials
// IOR = sqrt( relpermit )
// IOR^2 = repermit
// permit = IOR^2 * permit_of_free_space

// IOR Values:
//	1 = vacuum
//	1 ~= most gasses
// 	1.333 = water @ 20c
// 	1.31 = ice
//	1.52 = typical glass
//	1.77 = Saphire
//	2.15 = CZ
//	2.417 = diamnd

int main(int argc, char** argv) {
	// Open a window
	Window window = window_open(2000, 1500);
	
	World w = empty_world(720, 1280, 0.1);
	w.color_scale = 500;
	
	// Absorbing bounries	
	for (int y = 0; y < w.h; y++) {
		for (int dx = 0; dx < 6; dx++) {
		//	w.conductivity[dx][y] = 6;
			w.conductivity[w.w-dx-1][y] = 6;
		}
	}

	for (int x = 0; x < w.w; x++) {
		for (int dy = 0; dy < 6; dy++) {
			w.conductivity[x][dy] = 6;
			w.conductivity[x][w.h-dy-1] = 6;
		}
	}
	
	// Add circular lens
	for (int y = 0; y < w.h; y++) {
		for (int x = 0; x < w.w; x++) {
			if (v2_length((v2) {.x = x - w.w/3, .y = y - w.h/2}) < 40 ) {
				w.permittivity[x][y] *= IOR * IOR;
			}
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
		if (t < 1) {
			for (int y = 0; y < w.h; y++) {
				w.field_j[0][y].y = sin(t*5)/10;
			}
		}
		
		renderer_setup(&window, w.w, w.h);
	
		SDL_LockSurface(window.canvas);
		for (int x = 0; x < w.w; x++) {
			for (int y = 0; y < w.h; y++) {
				RGB c = colorgrade_em(&w, x,y);
				c.r *= w.permittivity[x][y];
				set_pixel(&window, x, y, clamp_to_rgb(c.r), clamp_to_rgb(c.g), clamp_to_rgb(c.b));
			}
		}

		SDL_UnlockSurface(window.canvas);
		
		window_present(&window);
	}

	free_world(w);

	return 0;
}
