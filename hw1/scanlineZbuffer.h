#pragma once


#include "super.h"
#include "mesh.h"


using namespace std;
using namespace glm;


#define MIN_DEPTH  -100000
#define DEFAULT_COLOR (vec3(1.0f,1.0f,1.0f))

struct Polytable {		//分类多边形表表项,根据ymax放入相应类中
	Parameter parameter;		// a, b, c, d	
	unsigned int dy;	//跨越扫描线的数目
	unsigned int id;
	vec3 color;
};

#define Activated_Polytable Polytable		//活化多边形表表项,记录当前扫描线与多边形在oxy投影面上相交的多边形

struct Linetable{		//分类边表表项，根据ymax放入相应类中
	float y=0;			//层次zbuffer要用
	float x;			//边端点的x坐标，非极值点：截断1 pixel
	float dx;			//相邻两条扫描线交点的x坐标差dx -(1/k)
	float z;			//边端点的z坐标：我自己新增的
	unsigned int dy;	//跨越扫描线的数目
	unsigned int id;	//边所属多边形的编号
	//bool operator<(const Linetable& a)const {
	//	return x != a.x ? x < a.x : dx < a.dx;
	//}
	bool operator>(const Linetable& a)const {
		return round(y) != round(a.y) ? y > a.y : x != a.x ? x < a.x : dx < a.dx;
	}
};

struct Activated_Linetable {		//活化边表表项，存放投影多边形边界与扫描线相交的边对
	float xl;						//左交点的x坐标
	float dxl;						//左交点所在的边上，两相邻扫描线交点的x坐标之差
	float dyl;						//左交点所在边与扫描线相交数，每向下处理一条-1
	float xr;						//
	float dxr;						//
	float dyr;						//
	float zl;						//左交点处多边形所在平面的深度值
	float dzx;						//沿扫描线向右走一个像素时，所在平面的深度增量
	float dzy;						//向下移一根扫面线时，所在平面的深度增量
	unsigned int id;				//边所属多边形的编号
};

class ScanLineZbuffer {
public:
	int width;
	int height;
	vec3 bg_color;

	//vector<vec3> full_framebuffer;
	//vector<vec3> frameBuffer;		//存储每条扫描线上的颜色
	vector<float> zBuffer;			//存储每条扫描线上的深度

	vector<vector<Polytable> > polytables;
	vector<vector<Linetable> > linetables;

	vector<Activated_Polytable> act_polytables;
	vector<Activated_Linetable> act_linetables;

	ScanLineZbuffer(Mesh& mesh, int w, int h);
	~ScanLineZbuffer();

	void clearBuffer();
	void setBuffer();
	void pushNewLine2ActivatedLinetable(int y, Activated_Polytable& apoly);
	void updateActivatedLinetable(int y);
	void pushNewPoly2ActivatedPolytable(int y);
	void updateActivatedPolytable(int y);
	void set1Line(int y);
	void set1Pixel(int y);

	bool static cmp_actLt(const Linetable& l1, const Linetable& l2);
	void findAndSortLinetable(int y, int id, vector<Linetable>& v);

	
};

