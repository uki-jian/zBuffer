#pragma once

#include "super.h"
#include "mesh.h"

using namespace std;
using namespace glm;

struct faceInfo {
	vec3 color;
	vector<vec3> vertices;
};

struct OctreeNode {
	int xl, xr, yl, yr; // [xl, xr) [yl, yr)
	float zl, zr;
	int layer;
	vector<int> faces;
	OctreeNode* father;

	/* 2 3
	   0 1
	   ---
	   6 7
	   4 5
	*/
	// up down left right near far
#define uln 0
#define urn 1
#define ulf 2
#define urf 3
#define dln 4
#define drn 5
#define dlf 6
#define drf 7
	OctreeNode* sons[8];
};

class Octree {
public:
	int width;
	int height;
	int depth; //八叉树的深度：根节点=第0层，叶子节点=第depth层
	int epsilon = 800; //八叉树叶子节点的精度,现在没设置好
	int n_faces = 0;
	OctreeNode* oct_root;

	Octree(int w, int h, Mesh& mesh);
	~Octree();

	bool is_leaf_node(OctreeNode* op);
	OctreeNode* initOctreeNode(int layer, int xl, int xr, int yl, int yr, float zl, float zr, OctreeNode* father);
	void addFaces(Mesh& mesh);
	void addFace_per_node(BoundingBox& bb, int faceIdx, OctreeNode* op);
	bool findFace(OctreeNode* op, int faceIdx);
	void releaseNode(OctreeNode* op);
};

