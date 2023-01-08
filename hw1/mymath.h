#pragma once
#include "super.h"

using namespace glm;

//平面方程参数
struct Parameter {
	float A, B, C, D;
};

Parameter computePlaneParameter(vec3 &p1, vec3 &p2, vec3 &p3);
float useXYcomputeZ(Parameter P, float x, float y);
