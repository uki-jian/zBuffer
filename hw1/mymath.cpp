#include "mymath.h"

Parameter computePlaneParameter(vec3 &p1, vec3 &p2, vec3 &p3) {
	float A = (p2.y - p1.y) * (p3.z - p1.z) - (p2.z - p1.z) * (p3.y - p1.y);
	float B = (p3.x - p1.x) * (p2.z - p1.z) - (p2.x - p1.x) * (p3.z - p1.z);
	float C = (p2.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) * (p2.y - p1.y);
	float D = -(A * p1.x + B * p1.y + C * p1.z);

	float norm = sqrt(A*A + B * B + C * C);
	A /= norm;
	B /= norm;
	C /= norm;
	D /= norm;
	Parameter parameter = { A, B, C, D };
	return parameter;
}

float useXYcomputeZ(Parameter P, float x, float y) {
	//Ax+By+Cz+D=0
	float z;
	z = -(P.A * x + P.B * y + P.D) / P.C;
	return z;
}
