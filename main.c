#include "render.h"
#include "math.h"

#define ELEMENTS_H 200
#define ELEMENTS_W 200

#define ELEMENTS_PITCH 0.1

////////////////
// Simulation //
////////////////

// Sources:
//	https://physics.stackexchange.com/questions/32685/
//	https://physics.stackexchange.com/questions/104008/

// From David Z's answer on https://physics.stackexchange.com/questions/32685
// The magnetic feilds is a sclaer, and the electic is a 2d vector

// dE.y / dt = -dB / dx
// dE.x / dt = dB / dy
// dB / dt = dE.x / dy - dE.y / dx (this part is actualy the 2d curl of the electric feild :) )

// Electric and magnetic feilds
v2 feild_j[ELEMENTS_W][ELEMENTS_H];
v2 feild_e[ELEMENTS_W][ELEMENTS_H];
v2 feild_e_new[ELEMENTS_W][ELEMENTS_H];
float feild_b[ELEMENTS_W][ELEMENTS_H];
float feild_b_new[ELEMENTS_W][ELEMENTS_H];

// Simple boundry conditions, electic feild is always zero outside of simulation
v2 get_e_feild(int x, int y) {
	if (x >= 0 && x < ELEMENTS_W && y >= 0 && y < ELEMENTS_H) {
		return feild_e[x][y];
	} else {
		return (v2) {.x = 0, .y = 0};
	}
}

// Simple boundry conditions, magnetic feild is always zero outside of simulation
float get_b_feild(int x, int y) {
	if (x >= 0 && x < ELEMENTS_W && y >= 0 && y < ELEMENTS_H) {
		return feild_b[x][y];
	} else {
		return 0;
	}
}

// Discrete integration of a 2d generalization of maxwells equasions
void do_em_at(float dt, int x, int y) {
	v2 J = feild_j[x][y];

	// Compute all the necicary diriveratives
	// These are computed from the point of view of the other feild
	float dB_dx = (get_b_feild(x + 1, y) - get_b_feild(x - 1, y)) / ELEMENTS_PITCH / 2;
	float dB_dy = (get_b_feild(x, y + 1) - get_b_feild(x, y - 1)) / ELEMENTS_PITCH / 2;

	float dEx_dy = (get_e_feild(x, y + 1).x - get_e_feild(x, y - 1).x) / ELEMENTS_PITCH / 2;
	float dEy_dx = (get_e_feild(x + 1, y).y - get_e_feild(x - 1, y).y) / ELEMENTS_PITCH / 2;

	// Compute the diriverative of the electric and magnetic feilds
	v2 dE_dt = {.x = dB_dy - J.x, .y = -dB_dx - J.y};
	float dB_dt = dEx_dy - dEy_dx;

	// Apply it.
	feild_e[x][y] = v2_add(feild_e[x][y], v2_mul_scaler(dE_dt, dt));
	feild_b[x][y] = feild_b[x][y] + dB_dt * dt;
}

void do_em(float dt) {
	for (int x = 0; x < ELEMENTS_W; x++) {
		for (int y = 0; y < ELEMENTS_H; y++) {
			do_em_at(dt, x, y);
		}
	}
	for (int x = 0; x < ELEMENTS_W; x++) {
		for (int y = 0; y < ELEMENTS_H; y++) {
	//		feild_e[x][y] = feild_e_new[x][y];
	//		feild_b[x][y] = feild_b_new[x][y];
		}
	}
}

///////////////
// Rendering //
///////////////

v2 e_feild_to_color(v2 e) {
	return (v2) {
		.x = (e.x*5 + 1) * 128,
		.y = (e.y*5 + 1) * 128 
	};
}

int b_feild_to_color(float b) {
	return (b*5 + 1) * 128;
}

// Clamp to alowable range for 8 bit colors
int clamp_to_rgb(int c) {
	if (c < 0) return 0;
	if (c > 255) return 255;
	return c;
}

/////////////
// UI code //
/////////////

void do_input() {
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



int main(int argc, char** argv) {
	// Open a window,
	Window window = window_open();
	for (int x = 0; x < ELEMENTS_W; x++) {
		for (int y = 0; y < ELEMENTS_H; y++) {
			feild_e[x][y] = (v2) {.x = 0, .y = 0};
			feild_b[x][y] = 0.0;
			feild_j[x][y] = (v2) {.x = 0, .y = 0};
		}
	}


	float t = 0;	
	float dt = 1.0/60;
	while (1) {
		// Handle inputs
		do_input();	
		
		do_em(dt);
		t += dt;
	
		if ((int)(t * 1)%2 == 0) {
			feild_j[100][100].y = 5;
		} else {
			feild_j[100][100].y = -5;
		}
		
		renderer_setup(&window, ELEMENTS_H, ELEMENTS_W);
	
		SDL_FillRect(window.canvas, NULL, 0xff00ffff);

		SDL_LockSurface(window.canvas);
		// TODO Rendering code here
		for (int x = 0; x < ELEMENTS_W; x++) {
			for (int y = 0; y < ELEMENTS_H; y++) {
				v2 e = e_feild_to_color(feild_e[x][y]);
				float b = b_feild_to_color(feild_b[x][y]);
				set_pixel(&window, x, y, clamp_to_rgb(e.x), clamp_to_rgb(e.y), clamp_to_rgb(b));
			}
		}

		SDL_UnlockSurface(window.canvas);
		
		window_present(&window);
	}

	return 0;
}
