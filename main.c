#include "render.h"
#include "math.h"

#define ELEMENTS_H 64
#define ELEMENTS_W 64

#define ELEMENTS_PITCH 0.1

////////////////
// Simulation //
////////////////

// dE.y / dt = -dB / dx
// dE.x / dt = dB / dy
// dB / dt = dE.x / dy - dE.y / dx (this part is actualy the 2d curl of the electric feild :) )

// Stagered feild representation:

//-------+--------+
//     Ex|      Ex|
//       |        |
// Ey  B | Ey   B |
//-------+--------|
//     Ex|      Ex|
//       |        |
// Ey  B | Ey   B |
//-------+--------+

// Each cell in the grid has a B field value and and 2 E field comonents.
// These values are stagered in space to allow computing the needed diriverates easly

// Buffer for current flowing through every part of the simulation
v2 feild_j[ELEMENTS_W][ELEMENTS_H];

// Electric and magnetic feilds
v2 feild_e[ELEMENTS_W][ELEMENTS_H];
float feild_b[ELEMENTS_W][ELEMENTS_H];

// Buffer for writing the computed states for next frame
float feild_b_next[ELEMENTS_W][ELEMENTS_H];
v2 feild_e_next[ELEMENTS_W][ELEMENTS_H];

// Simple boundry conditions, E and B fields are always zero outside of simulation
v2 get_e_feild(int x, int y) {
	if (x >= 0 && x < ELEMENTS_W && y >= 0 && y < ELEMENTS_H) {
		return feild_e[x][y];
	} else {
		return (v2) {.x = 0, .y = 0};
	}
}
float get_b_feild(int x, int y) {
	if (x >= 0 && x < ELEMENTS_W && y >= 0 && y < ELEMENTS_H) {
		return feild_b[x][y];
	} else {
		return 0;
	}
}

// Calculate the value of the e field at the next timestep
void calculate_ex_at(float dt, int x, int y) {
	v2 J = feild_j[x][y];

	float dB_dy = (get_b_feild(x, y) - get_b_feild(x, y - 1)) / ELEMENTS_PITCH;

	float dEx_dt = dB_dy - J.x;
	
	feild_e_next[x][y].x = feild_e[x][y].x + dEx_dt * dt;
}
void calculate_ey_at(float dt, int x, int y) {
	v2 J = feild_j[x][y];

	float dB_dx = (get_b_feild(x, y) - get_b_feild(x - 1, y)) / ELEMENTS_PITCH;

	float dEy_dt = -dB_dx - J.y;
	
	feild_e_next[x][y].y = feild_e[x][y].y + dEy_dt * dt;
}

// Calculate the value of the b field at the next timestep
void calculate_b_at(float dt, int x, int y) {
	float dEx_dy = (get_e_feild(x, y + 1).x - get_e_feild(x, y).x) / ELEMENTS_PITCH;
	float dEy_dx = (get_e_feild(x + 1, y).y - get_e_feild(x, y).y) / ELEMENTS_PITCH;
	
	float dB_dt = +dEx_dy - dEy_dx;
	
	feild_b_next[x][y] = feild_b[x][y] + dB_dt * dt;
}

void simulate_em(float dt) {
	for (int x = 0; x < ELEMENTS_W; x++) {
		for (int y = 0; y < ELEMENTS_H; y++) {
			// FIXME This breaks when fully syncronus, why?
			calculate_ex_at(dt, x, y);
			calculate_ey_at(dt, x, y);
			feild_e[x][y] = feild_e_next[x][y];
			calculate_b_at(dt, x, y);
			feild_b[x][y] = feild_b_next[x][y];
		}
	}
	// Update the current field with the newly computed one
	for (int x = 0; x < ELEMENTS_W; x++) {
		for (int y = 0; y < ELEMENTS_H; y++) {
			feild_e[x][y] = feild_e_next[x][y];
			feild_b[x][y] = feild_b_next[x][y];
		}
	}
}

///////////////
// Rendering //
///////////////

typedef struct RGB {
	int r,g,b;
} RGB;

v2 e_feild_to_color(v2 e) {
	return (v2) {
		.x = (e.x*10 + 1) * 128,
		.y = (e.y*10 + 1) * 128
	};
}

int b_feild_to_color(float b) {
	return (b*10 + 1) * 128;
}
/*
RGB colorgrade_em(int x, int y) {
	return (RGB) {
		.r = feild_e[],
		.g = ,
		.b = 
	}
}
*/
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

	feild_j[1][1].y = .1;

	float t = 0;	
	float dt = 1.0/60;
	while (1) {
		// Handle inputs
		do_input();	
		
		simulate_em(dt);
		t += dt;
	
		if ((int)(t * 5) % 2) {
			feild_j[32][32].y = 1;
		} else {
			feild_j[32][32].y = -1;
		}
		
		renderer_setup(&window, ELEMENTS_H, ELEMENTS_W);
	
		SDL_FillRect(window.canvas, NULL, 0xff00ffff);

		SDL_LockSurface(window.canvas);
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
