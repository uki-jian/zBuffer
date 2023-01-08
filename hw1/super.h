#pragma once

//#include "include/glad/glad.h"
#include "include/GLFW/glfw3.h"
#include "include/glut/glut.h"
#include "include/glm/glm.hpp"
#include "include/glm/gtc/matrix_transform.hpp"

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <time.h>
#include <random>
#include <omp.h>

#define SCR_WIDTH 800
#define SCR_HEIGHT 800

#define FARTHEST 0
#define NEAREST 800


#define RANDER_COLOR

#define RANDOM_SEED 1



struct frame_counter {
	unsigned int frame_counts = 0;
	unsigned int buffer_size = 10;
	clock_t time_buffer[10];
	float fps;
};

extern glm::vec3 full_framebuffer[SCR_HEIGHT][SCR_WIDTH];

//#define __ROTATE

#define __DEEPMAP



