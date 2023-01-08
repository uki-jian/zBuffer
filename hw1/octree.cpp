#include "octree.h"

Octree::Octree(int w, int h, Mesh& mesh) {
	clock_t t1 = clock();
	width = w;
	height = h;
	assert(w == h);
	depth = int(std::log2(w - 1)) + 1;
	n_faces = mesh.faces.size();

	oct_root = initOctreeNode(0, 0, w, 0, h, FARTHEST, NEAREST, NULL);

	addFaces(mesh);
	clock_t t2 = clock();
	cout << "an octree has been built, using: " << t2 - t1 << " ms" << endl;
}
Octree::~Octree() {
	releaseNode(oct_root);
}

void Octree::releaseNode(OctreeNode* op) {
	if (!op)return;
	for (int i = 0; i < 8; i++)releaseNode(op->sons[i]);
	delete op;
}

//如果是叶子节点，就返回true
bool Octree::is_leaf_node(OctreeNode* op) {
	for (int i = 0; i < 8; i++) {
		if (op->sons[i] != NULL)return false;
	}
	return true;
}
//初始化八叉树节点
OctreeNode* Octree::initOctreeNode(int layer, int xl, int xr, int yl, int yr, float zl, float zr, OctreeNode* father) {

	if (xl >= xr || yl >= yr)return NULL;

	assert(depth >= layer);
	int size = (xr - xl) * (yr - yl);

	OctreeNode* op = new OctreeNode;
	op->layer = layer;
	op->xl = xl;
	op->xr = xr;
	op->yl = yl;
	op->yr = yr;
	op->zl = zl;
	op->zr = zr;
	op->father = father;
	op->faces.resize(n_faces);
	fill(op->faces.begin(), op->faces.end(), 0);
	if (xr - xl <= epsilon || yr - yl <= epsilon) {
		for (int i = 0; i < 8; i++) {
			op->sons[i] = NULL;
		}
		return op;
	}

	int xmid = (xl + xr) / 2;
	int ymid = (yl + yr) / 2;
	float zmid = (zl + zr) / 2;

	op->sons[uln] = initOctreeNode(layer + 1, xl, xmid, yl, ymid, zmid, zr, op);
	op->sons[urn] = initOctreeNode(layer + 1, xmid, xr, yl, ymid, zmid, zr, op);
	op->sons[ulf] = initOctreeNode(layer + 1, xl, xmid, yl, ymid, zl, zmid, op);
	op->sons[urf] = initOctreeNode(layer + 1, xmid, xr, yl, ymid, zl, zmid, op);
	op->sons[dln] = initOctreeNode(layer + 1, xl, xmid, ymid, yr, zmid, zr, op);
	op->sons[drn] = initOctreeNode(layer + 1, xmid, xr, ymid, yr, zmid, zr, op);
	op->sons[dlf] = initOctreeNode(layer + 1, xl, xmid, ymid, yr, zl, zmid, op);
	op->sons[drf] = initOctreeNode(layer + 1, xmid, xr, ymid, yr, zl, zmid, op);

	return op;
}

void Octree::addFaces(Mesh& mesh) {
	for (Face& face : mesh.faces) {
		addFace_per_node(face.bb, face.idx, oct_root);
	}
}
void Octree::addFace_per_node(BoundingBox& bb, int faceIdx, OctreeNode* op) {
	//face完全在这个node之外，直接return
	if (op == NULL)return;
	if (op->xl > bb.maxx || op->xr <= bb.minx)return;
	if (op->yl > bb.maxy || op->yr <= bb.miny)return;
	if (op->zl > bb.maxz || op->zr <= bb.minz)return;

	op->faces[faceIdx] = 1;
	for (int i = 0; i < 8; i++) {
		addFace_per_node(bb, faceIdx, op->sons[i]);
	}
}
bool Octree::findFace(OctreeNode* op, int faceIdx) {
	if (!op)return false;
	if (op->faces[faceIdx] == 1)return true;
	else return false;
}