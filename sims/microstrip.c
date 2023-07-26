#include "../maxwell.h"
#include "../render.h"
#include <math.h>

#define SUBSTEPS 30

#define SHORTED 1

// Comment one to select colorscheme 
#define COLORGRADE colorgrade_m
//#define COLORGRADE colorgrade_em

// Uncomment to simply show the conductivity instead of the simulaton results
#define SHOW_CONDUCTIVITY 1

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
	//Window window = window_open(1000, 500);
	Window window = window_open_file("strip1/");
	
	World w = empty_world(1000, 500, 0.1);
	w.color_scale = 400;
	
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

        // Object
        int center_y = w.h/2;
        int center_x = w.w/2;
        int wire = 1000;

        // Horizontal wires
        for (int dx = -300; dx <= 300; dx++) {
                w.conductivity[center_x-dx][center_y-5] = wire;
                w.conductivity[center_x-dx][center_y+5] = wire;
        }

        // Vertical wires
        for (int dy = -5; dy <= 5; dy++) {
                w.conductivity[center_x-300][center_y-dy] = wire;
                #ifdef SHORTED
        	w.conductivity[center_x+300][center_y-dy] = wire;
		#endif
        }

        // Power source gap
        w.conductivity[center_x-300][center_y+0] = 0;
	
	int exitation_x = center_x-300;
        int exitation_y = center_y;

	float t = 0;	
	float dt = 1.0/60 * 3;
	while (1) {
		// Handle inputs
		do_input(&window);	
		
		for (int i = 0; i < SUBSTEPS; i++) {
			simulate_em(&w, dt/SUBSTEPS);
			t += dt/SUBSTEPS;
		}

		// Add current to exite em waves
		if (t > 0 && t < 3.14 && w.field_e[exitation_x][exitation_y].x > -0.1) {
			w.field_j[exitation_x][exitation_y].x = sin(t*2)*5;
		}
		
		renderer_setup(&window, w.w, w.h);
	
		SDL_LockSurface(window.canvas);
		for (int x = 0; x < w.w; x++) {
			for (int y = 0; y < w.h; y++) {
				RGB c = COLORGRADE(&w, x,y);
				#ifdef SHOW_CONDUCTIVITY
				c.r = (w.conductivity[x][y]*50);
				c.g = 0;
				c.b = 0;
				#endif
				set_pixel(&window, x, y, clamp_to_rgb(c.r), clamp_to_rgb(c.g), clamp_to_rgb(c.b));
			}
		}

		SDL_UnlockSurface(window.canvas);
		
		window_present(&window);
	}

	free_world(w);

	return 0;
}
