#include "math.h"
#include <math.h>

v2 v2_add(v2 a, v2 b) {
	return (v2) {.x = a.x + b.x, .y = a.y + b.y};
}

v2 v2_sub(v2 a, v2 b) {
	return (v2) {.x = a.x - b.x, .y = a.y - b.y};
}

v2 v2_mul(v2 a, v2 b) {
	return (v2) {.x = a.x * b.x, .y = a.y * b.y};
}

v2 v2_mul_scaler(v2 a, float s) {
	return (v2) {.x = a.x * s, .y = a.y * s};
}

v2 v2_div_scaler(v2 a, float s) {
	return v2_mul_scaler(a, 1/s);
}

float v2_length(v2 a) {
	return sqrt(a.x * a.x + a.y * a.y);
}

v2 v2_normalize(v2 a) {
	return v2_div_scaler(a, v2_length(a));
}
