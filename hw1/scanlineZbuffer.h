#pragma once


#include "super.h"
#include "mesh.h"


using namespace std;
using namespace glm;


#define MIN_DEPTH  -100000
#define DEFAULT_COLOR (vec3(1.0f,1.0f,1.0f))

struct Polytable {		//�������α����,����ymax������Ӧ����
	Parameter parameter;		// a, b, c, d	
	unsigned int dy;	//��Խɨ���ߵ���Ŀ
	unsigned int id;
	vec3 color;
};

#define Activated_Polytable Polytable		//�����α����,��¼��ǰɨ������������oxyͶӰ�����ཻ�Ķ����

struct Linetable{		//����߱�������ymax������Ӧ����
	float y=0;			//���zbufferҪ��
	float x;			//�߶˵��x���꣬�Ǽ�ֵ�㣺�ض�1 pixel
	float dx;			//��������ɨ���߽����x�����dx -(1/k)
	float z;			//�߶˵��z���꣺���Լ�������
	unsigned int dy;	//��Խɨ���ߵ���Ŀ
	unsigned int id;	//����������εı��
	//bool operator<(const Linetable& a)const {
	//	return x != a.x ? x < a.x : dx < a.dx;
	//}
	bool operator>(const Linetable& a)const {
		return round(y) != round(a.y) ? y > a.y : x != a.x ? x < a.x : dx < a.dx;
	}
};

struct Activated_Linetable {		//��߱������ͶӰ����α߽���ɨ�����ཻ�ı߶�
	float xl;						//�󽻵��x����
	float dxl;						//�󽻵����ڵı��ϣ�������ɨ���߽����x����֮��
	float dyl;						//�󽻵����ڱ���ɨ�����ཻ����ÿ���´���һ��-1
	float xr;						//
	float dxr;						//
	float dyr;						//
	float zl;						//�󽻵㴦���������ƽ������ֵ
	float dzx;						//��ɨ����������һ������ʱ������ƽ����������
	float dzy;						//������һ��ɨ����ʱ������ƽ����������
	unsigned int id;				//����������εı��
};

class ScanLineZbuffer {
public:
	int width;
	int height;
	vec3 bg_color;

	//vector<vec3> full_framebuffer;
	//vector<vec3> frameBuffer;		//�洢ÿ��ɨ�����ϵ���ɫ
	vector<float> zBuffer;			//�洢ÿ��ɨ�����ϵ����

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

