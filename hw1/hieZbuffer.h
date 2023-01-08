#pragma once

#include "super.h"
#include "mesh.h"
#include "scanlineZbuffer.h"
#include "octree.h"

#define MINZ -1000000

using namespace std;
using namespace glm;

//每层在buffer节点：四叉树
struct zbuffer_node {

	int layer;

	int xl, xr, yl, yr; // [xl ,xr) [yl, yr)

	float minz;

	/*
	   0 1
	   2 3
	*/
	// up down left right
#define ul 0
#define ur 1
#define dl 2
#define dr 3
	zbuffer_node* sons[4];
	zbuffer_node* father;
};

class HieZbuffer{
public:
	int height, width;
	int depth{ 0 };
	int refused_cnt{ 0 };
	vector<vector<zbuffer_node*>> hiezbuffer;
	zbuffer_node* zbuffer_root;
	vec3 bg_color{0.0f, 0.0f, 0.0f};
	Octree* otp;

	HieZbuffer(int width, int height);
	~HieZbuffer();
	zbuffer_node* initZbufferNode(int layer, int xl, int xr, int yl, int yr, zbuffer_node* father);
	bool is_leaf_node(zbuffer_node* znp);
	bool ztest_a_node(zbuffer_node* znp, BoundingBox& bb);
	bool ztest_a_node(zbuffer_node* znp, OctreeNode* op, int faceIdx, BoundingBox& bb);
	void ztest(Mesh& mesh, bool octree_mode = false);
	void updateZbuffer(zbuffer_node* znp);
	bool drawFace(Mesh& mesh, Face& face);
	void releaseZbuffer(zbuffer_node*);
};

