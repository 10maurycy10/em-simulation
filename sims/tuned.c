#include "../maxwell.h"
#include "../render.h"

#define SUBSTEPS 30

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
	Window window = window_open(1000, 1000);
	
	World w = empty_world(100, 100, 0.1);
	w.color_scale = 1000;
	
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

        // Object
        int center_y = w.h/2;
        int center_x = w.w/2;
        int wire = 1000;

        // Horizontal wires
        for (int dx = -10; dx <= 10; dx++) {
                w.field_conductivity[center_x-dx][center_y-10] = wire;
                w.field_conductivity[center_x-dx][center_y+10] = wire;
        }

        // Vertical wires
        for (int dy = -10; dy <= 10; dy++) {
                w.field_conductivity[center_x-10][center_y-dy] = wire;
                w.field_conductivity[center_x+10][center_y-dy] = wire;
        }

        // Capacitor gap
        w.field_conductivity[center_x+10][center_y-0] = 0;

        // Capacitor plates
        for (int dx = 7; dx <= 13; dx++) {
                w.field_conductivity[center_x+dx][center_y+1] = wire;
                w.field_conductivity[center_x+dx][center_y-1] = wire;
        }

        int exitation_x = center_x+10;
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
		if (t < 0.5) w.field_j[exitation_x][exitation_y].y = 1;
		
		renderer_setup(&window, w.w, w.h);
	
		SDL_LockSurface(window.canvas);
		for (int x = 0; x < w.w; x++) {
			for (int y = 0; y < w.h; y++) {
				RGB c = colorgrade_m(&w, x,y);
				set_pixel(&window, x, y, clamp_to_rgb(c.r), clamp_to_rgb(c.g), clamp_to_rgb(c.b));
			}
		}

		SDL_UnlockSurface(window.canvas);
		
		window_present(&window);
	}

	free_world(w);

	return 0;
}
