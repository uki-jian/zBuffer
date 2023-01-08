#pragma once

#include "super.h"
#include "mymath.h"

using namespace std;
using namespace glm;


#define MAXVALUE 10000000
struct BoundingBox {
	float maxx = -MAXVALUE, minx = MAXVALUE, 
		maxy = -MAXVALUE, miny = MAXVALUE, 
		maxz = -MAXVALUE, minz = MAXVALUE;
};

struct Vertex {
	vec3 position;
};

struct Face {
	vector<int> vertexIdx;
	BoundingBox bb;
	int idx;
	int maxYVertexIdx = 0;
	int minYVertexIdx = 0;
	//vector<int> textureIdx;
	//vector<int> normalIdx;
	Parameter parameter; //平面方程a,b,c,d; (a,b,c)==normal
	vec3 color;
};

class Mesh
{
public:
	vector<Vertex> vertices;
	vector<Face> faces;
	//vector<vec3> normals;
	//vector<vec2> textures;

	Mesh();
	~Mesh();

	void init(const string& path);
	void preprossess();

private:
	void loadOBJ(const string& path);
	bool sortVertexIdxYMAX(int a, int b);
	vec3 randFaceColor();
};

