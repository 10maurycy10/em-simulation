// Vector type
typedef struct v2 {
	float x, y;
} v2;

v2 v2_add(v2 a, v2 b);

v2 v2_sub(v2 a, v2 b);

v2 v2_mul(v2 a, v2 b);

v2 v2_mul_scaler(v2 a, float s);

v2 v2_div_scaler(v2 a, float s);

float v2_length(v2 a);

v2 v2_normalize(v2 a);
