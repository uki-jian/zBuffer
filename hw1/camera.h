#pragma once

#include "super.h"
#include "mesh.h"

#define MAXPOS -1000000
#define MINPOS  1000000

using namespace std;
using namespace glm;

class Camera {
public:
	static void model2screen(Mesh& mesh, int width, int height, frame_counter& fc, mat4& trans_mat);
};

