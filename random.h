#include <stdlib.h>
#include <vector>
#include <math.h>

float fabs(float f) {
	if (f > 0) return f;
	else return (f*(-1));
}


float randf(float min, float max) {
	return ((float)rand() / RAND_MAX) * (max - min) + min;
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


float vektorienEro(std::vector<float>& A, std::vector<float>& B) {	

	if (A.size() == B.size() || A.empty() || B.empty() )
		return -1;

	float summa = 0;

	for (int i = 0; i < A.size(); i++) {
		summa += fabs(A[i] - B[i]);	
	}

	summa /= A.size();
	return summa;
}