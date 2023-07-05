#include "render.h"
#include "math.h"

#define ELEMENTS_H 64
#define ELEMENTS_W 128

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

// Buffer for conductivity data
float feild_conductivity[ELEMENTS_W][ELEMENTS_H];

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
	// This is at the end so that other code can modify the current
	for (int x = 0; x < ELEMENTS_W; x++) {
		for (int y = 0; y < ELEMENTS_H; y++) {
			feild_j[x][y] = v2_mul_scaler(feild_e[x][y], feild_conductivity[x][y]);
		}
	}
}

///////////////
// Rendering //
///////////////

typedef struct RGB {
	int r,g,b;
} RGB;


float color_scale = 50;

RGB colorgrade_em(int x, int y) {
	// A simple colorgrading that shows the electric and magnetic fields
	return (RGB) {
		.r = (feild_e[x][y].x*color_scale + 1) * 128,
		.g = (feild_e[x][y].y*color_scale + 1) * 128,
		.b = (feild_b[x][y]  *color_scale + 1) * 128
	};
}

RGB colorgrade_m(int x, int y) {
	// Shows the magnetic feild
	return (RGB) {
		.r =  feild_b[x][y] * color_scale * 255,
		.g = 0,
		.b = -feild_b[x][y] * color_scale * 255
	};
}

RGB colorgrade_total_current(int x, int y) {
	// Shows the current (in conductors) and charge gradients
	float total = v2_length(feild_j[x][y]);

	float dEy_dy = (get_e_feild(x, y + 1).y - get_e_feild(x, y).y)/ELEMENTS_PITCH;
	float dEx_dx = (get_e_feild(x + 1, y).x - get_e_feild(x, y).x)/ELEMENTS_PITCH;

	return (RGB) {
		.r =  total * color_scale * 255,
		.g =  dEy_dy * color_scale * 255,
		.b =  dEx_dx * color_scale * 255,
	};
}

RGB colorgrade_total_em(int x, int y) {
	float e = v2_length(feild_e[x][y]);
	float b = fabs(feild_b[x][y]);
	e = sqrt(e);
	b = sqrt(b);

	return (RGB) {
		.r =  b * color_scale * 255,
		.g =  e * color_scale * 255,
		.b =  0,
	};
}

#define COLORGRADE colorgrade_total_em

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
			feild_conductivity[x][y] = 0;
		}
	}
	
	for (int y = 0; y < ELEMENTS_H; y++) {
		feild_conductivity[32][y] = 100;
		feild_conductivity[31][y] = 100;
	}
	
	feild_conductivity[32][24] = 0;
	feild_conductivity[32][25] = 0;
	feild_conductivity[31][24] = 0;
	feild_conductivity[31][25] = 0;
	
	feild_conductivity[32][43] = 0;
	feild_conductivity[32][42] = 0;
	feild_conductivity[31][43] = 0;
	feild_conductivity[31][42] = 0;

	float t = 0;	
	float dt = 1.0/60;
	while (1) {
		// Handle inputs
		do_input();	
		
		simulate_em(dt/3);
		simulate_em(dt/3);
		simulate_em(dt/3);
		t += dt;
	
		feild_conductivity[16][31] = 100;
		feild_conductivity[16][32] = 100;
		feild_conductivity[16][33] = 100;
		if ((int)(t * 5) % 2) {
			feild_j[16][32].y = 1;
		} else {
			feild_j[16][32].y = -1;
		}
		
		renderer_setup(&window, ELEMENTS_W, ELEMENTS_H);
	
		SDL_FillRect(window.canvas, NULL, 0xff00ffff);

		SDL_LockSurface(window.canvas);
		for (int x = 0; x < ELEMENTS_W; x++) {
			for (int y = 0; y < ELEMENTS_H; y++) {
				RGB c = COLORGRADE(x,y);
				set_pixel(&window, x, y, clamp_to_rgb(c.r), clamp_to_rgb(c.g), clamp_to_rgb(c.b));
			}
		}

		SDL_UnlockSurface(window.canvas);
		
		window_present(&window);
	}

	return 0;
}
