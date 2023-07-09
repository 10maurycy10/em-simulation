#include "../maxwell.h"
#include "../render.h"

#define SUBSTEPS 10

#define PI 3.14

// If this is one the induced freqency will be completly different then then tuned circuit
#define DETUNE 0

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
	w.color_scale = 100;
	
	// Absorbing bounries	
	for (int y = 0; y < w.h; y++) {
		for (int dx = 0; dx < 10; dx++) {
			w.conductivity[dx][y] = 6;
			w.conductivity[w.w-dx-1][y] = 6;
		}
	}

	for (int x = 0; x < w.w; x++) {
		for (int dy = 0; dy < 10; dy++) {
			w.conductivity[x][dy] = 6;
			w.conductivity[x][w.h-dy-1] = 6;
		}
	}

        // Object
        int center_y = w.h/2;
        int center_x = w.w/2;
        int wire = 1000;

	int width = 4, height = 4;

        // Horizontal wires
        for (int dx = -(width/2); dx <= (width/2); dx++) {
                w.conductivity[center_x-dx][center_y-(height/2)] = wire;
                w.conductivity[center_x-dx][center_y+(height/2)] = wire;
        }

        // Vertical wires
        for (int dy = -(height/2); dy <= (height/2); dy++) {
                w.conductivity[center_x-(width/2)][center_y-dy] = wire;
                w.conductivity[center_x+(width/2)][center_y-dy] = wire;
        }

        // Capacitor gap
        w.conductivity[center_x-(height/2)][center_y-0] = 0;

        int exitation_x = 10, exitation_y = center_y;

	float t = 0;	
	float dt = 1.0/60;
	while (1) {
		// Handle inputs
		do_input(&window);	
		
		for (int i = 0; i < SUBSTEPS; i++) {
			simulate_em(&w, dt/SUBSTEPS);
			t += dt/SUBSTEPS;
		}
	
		float freqency;
		if (DETUNE) {
			freqency = 9 / PI;
		} else {
			freqency = 6 / PI;
		}
		
		w.field_j[exitation_x][exitation_y].y = sin(freqency * t);
        
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
