#ifndef RANDOM_H
#define RANDOM_H

#include <stdlib.h>
#include <vector>
#include <math.h>

float fabs(float f);
float randf(float min,float max);
void bound(float& value, float min, float max);
void wrap(float& value, float min, float max);
float vektorienEro(std::vector<float> A, std::vector<float> B);


#endif