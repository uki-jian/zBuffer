#include "scanlineZbuffer.h"

ScanLineZbuffer::ScanLineZbuffer(Mesh& mesh, int w, int h) {
	if (w <= 0 || h <= 0) {
		cout << "Width or height can't be lower than 1!" << endl;
		exit(1);
	}
	width = w;
	height = h;
	bg_color = DEFAULT_COLOR;

	//frameBuffer.resize(w);
	zBuffer.resize(w);
	//full_framebuffer.resize(w * h);


	polytables.resize(h);
	linetables.resize(h);
#pragma omp parallel for
	for (int cnt = 0; cnt < mesh.faces.size(); cnt++) {
		Face face = mesh.faces[cnt];
		//初始化分类多边形表 

		Polytable poly;
		poly.parameter = face.parameter;
		if (fabs(poly.parameter.C) <= 0.001) continue;//去除和摄像机平行的面
		poly.color = face.color;
		poly.id = cnt;
		poly.dy = unsigned int(round(mesh.vertices[face.maxYVertexIdx].position.y)
			- round(mesh.vertices[face.minYVertexIdx].position.y));
		if (poly.dy <= 0)continue;

		int index = int(round(mesh.vertices[face.maxYVertexIdx].position.y));
		//if (index >= h)index = h - 1;	//防止溢出
		polytables[index].push_back(poly);

		//初始化分类边表，创建i->i+1的边
#pragma omp parallel for
		for (int i = 0; i < face.vertexIdx.size(); i++) {
			int vid1 = face.vertexIdx[i];
			int vid2 = face.vertexIdx[(i + 1) % face.vertexIdx.size()];
			//y坐标大的那个vertex索引
			int maxvid = mesh.vertices[vid1].position.y > mesh.vertices[vid2].position.y ?
				vid1 : vid2;
			int minvid = maxvid == vid1 ? vid2 : vid1;

			vec3& maxp = mesh.vertices[maxvid].position;
			vec3& minp = mesh.vertices[minvid].position;

			Linetable line;

			line.x = maxp.x;
			line.z = maxp.z;
			line.dy = unsigned int(round(maxp.y) - round(minp.y));
			if (line.dy <= 0)continue;//跳过水平边

			line.dx = -(maxp.x - minp.x) / (maxp.y - minp.y);
			line.id = cnt;

			int index = int(round(maxp.y));
			assert(index < h);
			//if (index >= h)index = h - 1;	//防止溢出
			linetables[index].push_back(line);
		}
	}
}

ScanLineZbuffer::~ScanLineZbuffer() { ; }

//重置缓冲器
void ScanLineZbuffer::clearBuffer() {
	//fill(frameBuffer.begin(), frameBuffer.end(), bg_color);		//帧缓冲器设置为背景色
	fill(zBuffer.begin(), zBuffer.end(), MIN_DEPTH);			//z缓冲器设置为最小值
}
//根据活化边表逐像素写入zbuffer
void ScanLineZbuffer::set1Pixel(int y) {
	int n_aline = act_linetables.size();

	//遍历所有活化边表
#pragma omp parallel for
	for (int i = 0; i < n_aline; i++) {

		int xl, xr;		//每个边对的区间坐标[xl, xr)	
		int id;
		float zx;
		xl = round(act_linetables[i].xl);
		xr = round(act_linetables[i].xr) + 1;
		zx = act_linetables[i].zl;
		id = act_linetables[i].id;

		vec3 color;
		bool findapoly = false;
#pragma omp parallel for
		for (auto apoly : act_polytables) {	//从活化多边形表中找颜色
			if (id == apoly.id) {
				color = apoly.color;
				findapoly = true;
				break;
			}
		}
		if (!findapoly) {
			cout << "Can't find a polygon in activated polytable with respect to a line in activated linetable!" << endl;
			exit(1);
		}
#pragma omp parallel for
		for (int x = std::max(0, xl); x < std::min(width, xr); x++) {
			if (zx > zBuffer[x]) {
				zBuffer[x] = zx;	//更新zbuffer

				int id = act_linetables[i].id;
#ifdef __DEEPMAP
				float c = (zx - FARTHEST) / (NEAREST - FARTHEST);
				vec3 dcolor = { c, c, c };
				full_framebuffer[y][x] = dcolor;
#else
				full_framebuffer[y][x] = color;		//更新frame buffer
#endif				
			}
			zx += act_linetables[i].dzx;	//每向右走一个像素，更新zx=zx+dzx
		}
	}
}

//比较两个边表的顺序，x小的放前面；成对放入活化边表
bool ScanLineZbuffer::cmp_actLt(const Linetable& l1, const Linetable& l2) {
	return l1.x != l2.x ? l1.x < l2.x : l1.dx < l2.dx;
}

//将分类边表中的新边加入活化边表

void ScanLineZbuffer::findAndSortLinetable(int y, int id, vector<Linetable>& v) {
#pragma omp parallel for
	for (Linetable& line : linetables[y]) {
		if (line.id == id) {
			v.push_back(line);
		}
	}
	sort(v.begin(), v.end(), cmp_actLt);
}
void ScanLineZbuffer::pushNewLine2ActivatedLinetable(int y, Activated_Polytable& apoly) {
	vector<Linetable>v;	//q按照x从小到大排列
	findAndSortLinetable(y, apoly.id, v);

	if (v.size() % 2 == 1) {
		cout << "Error: linetable group for activated linetable can't be odd!" << endl;
		exit(1);
	}
#pragma omp parallel for
	for (int i = 0; i + 1 < v.size(); i++) {
		Linetable al = v[i];
		Linetable ar = v[i + 1];
		Activated_Linetable aline;
		aline.id = apoly.id;

		aline.xl = al.x;
		aline.dxl = al.dx;
		aline.dyl = al.dy;

		aline.xr = ar.x;
		aline.dxr = ar.dx;
		aline.dyr = ar.dy;

		//Ax+By+Cz+D=0

		aline.zl = al.z; // useXYcomputeZ(apoly.parameter, al.x, y);
		aline.dzx = -apoly.parameter.A / apoly.parameter.C;
		aline.dzy = apoly.parameter.B / apoly.parameter.C;

		act_linetables.push_back(aline);
	}

}

/*
边表的水平边、奇异点、c=0处理***
*/
//更新活化边表
void ScanLineZbuffer::updateActivatedLinetable(int y) {
	if (y == 539) {
		int sssss = 1;
	}
	int cnt = 0;
#pragma omp parallel for
	for (int cnt = 0; cnt < act_linetables.size();) {
		if (cnt >= act_linetables.size())break;
		Activated_Linetable& aline = act_linetables[cnt];

		Activated_Polytable* ptr_poly;
		bool findpoly = false;
#pragma omp parallel for
		for (Activated_Polytable& apoly : act_polytables) {
			//在活化多边形表中找到该多边形，说明还有剩余的边
			if (apoly.id == aline.id) {
				findpoly = true;
				break;
			}
		}
		if (aline.id == 40124) {
			int stoppp = 1;
		}
		//找不到，删除该边
		if (!findpoly) {
			act_linetables.erase(act_linetables.begin() + cnt);
			continue;
		}

		--aline.dyl;
		--aline.dyr;

		vector<Linetable> v;
		if (aline.dyl <= 0 || aline.dyr <= 0) {
			findAndSortLinetable(y, aline.id, v);
		}

		int cnt2 = 0;
		if (cnt2 < v.size() && aline.dyl <= 0) {
			aline.xl = v[cnt2].x;
			aline.zl = v[cnt2].z;
			aline.dxl = v[cnt2].dx;
			aline.dyl = v[cnt2].dy;
			cnt2++;
		}
		else {
			aline.xl += aline.dxl;
			aline.zl += (aline.dzx * aline.dxl + aline.dzy);
		}
		if (cnt2 < v.size() && aline.dyr <= 0) {
			aline.xr = v[cnt2].x;
			aline.dxr = v[cnt2].dx;
			aline.dyr = v[cnt2].dy;
			cnt2++;
		}
		else {
			aline.xr += aline.dxr;
		}



		cnt++;
	}
}

//将分类多边形表中的新多边形加入活化多边形表
void ScanLineZbuffer::pushNewPoly2ActivatedPolytable(int y) {
#pragma omp parallel for
	for (Polytable& poly : polytables[y]) {

		if (poly.id == 40124) {
			int stoppp = 1;
		}
		Activated_Polytable apoly;
		apoly = poly;
		act_polytables.push_back(apoly);
		pushNewLine2ActivatedLinetable(y, apoly);
	}
}

//更新活化多边形表dy-1
void ScanLineZbuffer::updateActivatedPolytable(int y) {
	//将活化多边形表中的每个多边形dy-1，若dy<0则从中除去 
#pragma omp parallel for
	for (int cnt = 0; cnt < act_polytables.size();) {
		Activated_Polytable& apoly = act_polytables[cnt];
		if (--apoly.dy <= 0) {
			act_polytables.erase(act_polytables.begin() + cnt);
		}
		else cnt++;
	}
}
//写每一行
void ScanLineZbuffer::set1Line(int y) {
	updateActivatedPolytable(y);
	updateActivatedLinetable(y);
	pushNewPoly2ActivatedPolytable(y);
	set1Pixel(y);
}
//写入整个buffer
void ScanLineZbuffer::setBuffer() {
	fill(full_framebuffer[0], full_framebuffer[0] + width * height, bg_color);	//frame buffer初始化为背景色
#pragma omp parallel for
	for (int y = height - 1; y >= 0; y--) {
		clearBuffer();
		set1Line(y);
	}
}