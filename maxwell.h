#include "math.h"
#include <math.h>

////////////////
// Simulation //
////////////////

// dE.y / dt = -dB / dx
// dE.x / dt = dB / dy
// dB / dt = dE.x / dy - dE.y / dx (this part is actualy the 2d curl of the electric feild :) )

// Stagered field representation:

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
typedef struct World {
	// Width hieght and spacing
	int w, h;
	float spacing;

	// Scale factor for built in color grading
	float color_scale;

	// Material data
	float** permittivity; 
	float** permeability;
	float** conductivity;

	// Buffer for current flowing through every part of the simulation
	v2** field_j;

	// Electric and magnetic feilds
	v2** field_e;
	float** field_b;

	// Buffer for writing the computed states for next frame
	float** field_b_next;
	v2** field_e_next;
} World;

World empty_world(int w, int h, float spacing);
void free_world(World w);

// Get the value of a field, with bounds checks
v2 get_e_field(World* world, int x, int y);
float get_b_field(World* world, int x, int y);

// Write the next state of the field at a point into the next buffer
void calculate_ex_at(World* world, float dt, int x, int y);
void calculate_ey_at(World* world, float dt, int x, int y);
void calculate_b_at(World* world, float dt, int x, int y);

// Step the simulation of the whole world
void simulate_em(World* world, float dt);

///////////////
// Rendering //
///////////////

typedef struct RGB {
	int r,g,b;
} RGB;


float sigmoid(float x);

RGB colorgrade_em(World* w, int x, int y);

RGB colorgrade_m(World* w, int x, int y);

RGB colorgrade_total_current(World* w, int x, int y);

RGB colorgrade_total_em(World* w, int x, int y);

int clamp_to_rgb(int c);
