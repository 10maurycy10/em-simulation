// 2d simulation of maxwells equations
#include "maxwell.h"
#include <stdlib.h>

World empty_world(int width, int h, float spacing) {
	World w;

	w.w = width;
	w.h = h;
	w.spacing = spacing;

	w.color_scale = 1000;

	// Fields
	w.field_e = malloc(sizeof(*w.field_e) * width);
	w.field_b = malloc(sizeof(*w.field_b) * width);
	w.field_e_next = malloc(sizeof(*w.field_e) * width);
	w.field_b_next = malloc(sizeof(*w.field_b) * width);
	w.field_j = malloc(sizeof(*w.field_j) * width);

	// Materials
	w.conductivity = malloc(sizeof(*w.conductivity) * width);
	w.permittivity = malloc(sizeof(*w.permittivity) * width);
	w.permeability = malloc(sizeof(*w.permeability) * width);
	for (int x = 0; x < width; x++) {
		// Fields
		w.field_e[x] = malloc(sizeof(**w.field_e) * h);
		w.field_b[x] = malloc(sizeof(**w.field_b) * h);
		w.field_e_next[x] = malloc(sizeof(**w.field_e) * h);
		w.field_b_next[x] = malloc(sizeof(**w.field_b) * h);
		w.field_j[x] = malloc(sizeof(**w.field_j) * h);
		// Materials
		w.conductivity[x] = malloc(sizeof(**w.conductivity) * h);
		w.permeability[x] = malloc(sizeof(**w.permeability) * h);
		w.permittivity[x] = malloc(sizeof(**w.permittivity) * h);

		for (int y = 0; y < h; y++) {
			// Fields
			w.field_e[x][y] = (v2) {0, 0}; 
			w.field_j[x][y] = (v2) {0, 0};
			w.field_b[x][y] = 0;
			// Materials
			w.conductivity[x][y] = 0;
			w.permittivity[x][y] = 1;
			w.permeability[x][y] = 1;
		}
	}
	return w;
}

void free_room(World w) {
	for (int x = 0; x < w.w; x++) {
		free(w.field_e[x]);
		free(w.field_e_next[x]);
		free(w.field_j[x]);
		free(w.field_b[x]);
		free(w.field_b_next[x]);
		free(w.conductivity[x]);
		free(w.permeability[x]);
		free(w.permittivity[x]);
	}
	free(w.field_e);
	free(w.field_e_next);
	free(w.field_j);
	free(w.field_b);
	free(w.field_b_next);
	free(w.conductivity);
	free(w.permittivity);
	free(w.permeability);
}

// Simple boundry conditions, E and B fields are always zero outside of simulation
v2 get_e_field(World* w, int x, int y) {
	if (x >= 0 && x < w->w && y >= 0 && y < w->h) {
		return w->field_e[x][y];
	} else {
		return (v2) {.x = 0, .y = 0};
	}
}

float get_b_field(World* w, int x, int y) {
	if (x >= 0 && x < w->w && y >= 0 && y < w->h) {
		return w->field_b[x][y];
	} else {
		return 0;
	}
}
// Curl(B) = permiability * ( J + permitivity * dE/dt)
// Curl(B) / permiability = J + permitiviity * dE/dt
// (Curl(B) / permiability) - J = permitiviity * dE/dt
// ((Curl(B) / permiability) - J)/permitivity = dE/dt

// Wire the next field state to the next buffer at the given location
void calculate_ex_at(World* w, float dt, int x, int y) {
	v2 J = w->field_j[x][y];

	float dB_dy = (get_b_field(w, x, y) - get_b_field(w, x, y - 1)) / w->spacing;

	float dEx_dt = (dB_dy/w->permeability[x][y] - J.x)/w->permittivity[x][y];
	
	w->field_e_next[x][y].x = w->field_e[x][y].x + dEx_dt * dt;
}
void calculate_ey_at(World* w, float dt, int x, int y) {
	v2 J = w->field_j[x][y];

	float dB_dx = (get_b_field(w, x, y) - get_b_field(w, x - 1, y)) / w->spacing;

	float dEy_dt = (-dB_dx/w->permeability[x][y] - J.y)/w->permittivity[x][y];
	
	w->field_e_next[x][y].y = w->field_e[x][y].y + dEy_dt * dt;
}

// Calculate the value of the b field at the next timestep
void calculate_b_at(World* w, float dt, int x, int y) {
	float dEx_dy = (get_e_field(w, x, y + 1).x - get_e_field(w, x, y).x) / w->spacing;
	float dEy_dx = (get_e_field(w, x + 1, y).y - get_e_field(w, x, y).y) / w->spacing;
	
	float dB_dt = (+dEx_dy - dEy_dx);
	
	w->field_b_next[x][y] = w->field_b[x][y] + dB_dt * dt;
}

void simulate_em(World* w, float dt) {
	for (int x = 0; x < w->w; x++) {
		for (int y = 0; y < w->h; y++) {
			// FIXME This breaks when fully syncronus, why?
			calculate_ex_at(w, dt, x, y);
			calculate_ey_at(w, dt, x, y);
			w->field_e[x][y] = w->field_e_next[x][y];
			calculate_b_at(w, dt, x, y);
			w->field_b[x][y] = w->field_b_next[x][y];
		}
	}
	// Update the current field with the newly computed one
	for (int x = 0; x < w->w; x++) {
		for (int y = 0; y < w->h; y++) {
			// FIXME This breaks when fully syncronus, why?
			//w->field_e[x][y] = w->field_e_next[x][y];
			//w->field_b[x][y] = w->field_b_next[x][y];
		}
	}
	// Ohms law, this is at the end so that other code can modify currents during a step
	for (int x = 0; x < w->w; x++) {
		for (int y = 0; y < w->h; y++) {
			w->field_j[x][y] = v2_mul_scaler(w->field_e[x][y], w->conductivity[x][y]);
		}
	}
}

float sigmoid(float x) {
	return tanh(x);
}

RGB colorgrade_em(World* w, int x, int y) {
	// A simple colorgrading that shows the electric and magnetic fields
	return (RGB) {
		.r = (sigmoid(w->field_e[x][y].x*w->color_scale) + 1) * 128,
		.g = (sigmoid(w->field_e[x][y].y*w->color_scale) + 1) * 128,
		.b = (sigmoid(w->field_b[x][y]*w->color_scale) + 1) * 128
	};
}

RGB colorgrade_m(World* w, int x, int y) {
	float total = v2_length(w->field_j[x][y]);
	// Shows the magnetic feild
	return (RGB) {
		.r = sigmoid(w->field_b[x][y] * 10 * w->color_scale) * 255,
		.g = sigmoid(total * w->color_scale) * 255,
		.b = sigmoid(-w->field_b[x][y] * 10 * w->color_scale) * 255
	};
}

RGB colorgrade_total_current(World* w, int x, int y) {
	// Shows the current (in conductors) and charge gradients
	float total = v2_length(w->field_j[x][y]);

	float Ey = get_e_field(w, x, y).y/w->spacing;
	float Ex = get_e_field(w, x, y).x/w->spacing;

	return (RGB) {
		.r =  sigmoid(total * w->color_scale) * 255,
		.g =  fabs(sigmoid(Ey/2) * w->color_scale) * 255,
		.b =  fabs(sigmoid(Ex/2) * w->color_scale) * 255,
	};
}

RGB colorgrade_total_em(World* w, int x, int y) {
	float e = v2_length(w->field_e[x][y]);
	float b = fabs(w->field_b[x][y]);
	e = sigmoid(e * w->color_scale);
	b = sigmoid(b * w->color_scale);

	return (RGB) {
		.r =  b * 255,
		.g =  e * 255,
		.b =  0,
	};
}

// Clamp to alowable range for 8 bit colors
int clamp_to_rgb(int c) {
	if (c < 0) return 0;
	if (c > 255) return 255;
	return c;
}

