#include "random.h"

float randf(float min, float max) {
	return ((float)rand() / RAND_MAX) * (max - min) + min;
}

glm::vec3 randvec(float min, float max) {
	return(glm::vec3(randf(min, max), randf(min, max), randf(min, max)));
}


void bound(float& value, float min, float max) {
	if (value < min) value = min;
	if (value > max) value = max;
}


void wrap(float& value, float min, float max) {
	float w = max - min;
	while (value < min) value += w;
	while (value > max) value -= w;
}