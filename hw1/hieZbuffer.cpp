#include "hieZbuffer.h"

HieZbuffer::HieZbuffer(int w, int h) {

	width = w;
	height = h;
	assert(w == h);
	depth = int(std::log2(w - 1)) + 1;
	hiezbuffer.resize(depth + 1);
	hiezbuffer[depth].resize(w * h);

	fill(full_framebuffer[0], full_framebuffer[0] + w * h, bg_color);

	zbuffer_root = initZbufferNode(0, 0, w, 0, h, NULL);

}
HieZbuffer::~HieZbuffer() {
	releaseZbuffer(zbuffer_root);
}
void HieZbuffer::releaseZbuffer(zbuffer_node* zp) {
	if (!zp)return;
	for (int i = 0; i < 4; i++)releaseZbuffer(zp->sons[i]);
	delete zp;
}
//�½�ÿ��zbuffer�ڵ�
zbuffer_node* HieZbuffer::initZbufferNode(int layer, int xl, int xr, int yl, int yr, zbuffer_node* father) {

	if (xl >= xr || yl >= yr)return NULL;

	assert(depth >= layer);
	int size = (xr - xl) * (yr - yl);

	zbuffer_node* znp = new zbuffer_node;
	znp->layer = layer;
	znp->xl = xl;
	znp->xr = xr;
	znp->yl = yl;
	znp->yr = yr;
	znp->minz = MINZ;
	znp->father = father;
	//Ҷ�ӽڵ㣺���ⷽ��
	if (layer == depth) {
		znp->sons[ul] = NULL;
		znp->sons[ur] = NULL;
		znp->sons[dl] = NULL;
		znp->sons[dr] = NULL;
		hiezbuffer[depth][yl * width + xl] = znp;
	}
	else {
		//����Ϊ�����ڶ�������ж�
	// 1*1
		if (xr - xl <= 1 && yr - yl <= 1) {
			znp->sons[ul] = initZbufferNode(layer + 1, xl, xr, yl, yr, znp);
			znp->sons[ur] = NULL;
			znp->sons[dl] = NULL;
			znp->sons[dr] = NULL;
		}
		// 2*1
		else if (xr - xl <= 1) {
			int ymid = (yl + yr) / 2;
			znp->sons[ul] = initZbufferNode(layer + 1, xl, xr, yl, ymid, znp);
			znp->sons[ur] = NULL;
			znp->sons[dl] = initZbufferNode(layer + 1, xl, xr, ymid, yr, znp);
			znp->sons[dr] = NULL;
		}
		// 1*2
		else if (yr - yl <= 1) {
			int xmid = (xl + xr) / 2;
			znp->sons[ul] = initZbufferNode(layer + 1, xl, xmid, yl, yr, znp);
			znp->sons[ur] = initZbufferNode(layer + 1, xmid, xr, yl, yr, znp);
			znp->sons[dl] = NULL;
			znp->sons[dr] = NULL;
		}
		else {
			int xmid = (xl + xr) / 2;
			int ymid = (yl + yr) / 2;
			znp->sons[ul] = initZbufferNode(layer + 1, xl, xmid, yl, ymid, znp);
			znp->sons[ur] = initZbufferNode(layer + 1, xmid, xr, yl, ymid, znp);
			znp->sons[dl] = initZbufferNode(layer + 1, xl, xmid, ymid, yr, znp);
			znp->sons[dr] = initZbufferNode(layer + 1, xmid, xr, ymid, yr, znp);
		}
		//ÿһ��Ӧ���ǰ������ϵ�����˳���
		hiezbuffer[layer].push_back(znp);
	}

	return znp;
}

//�����Ҷ�ӽڵ㣬�ͷ���true
bool HieZbuffer::is_leaf_node(zbuffer_node* znp) {
	for (int i = 0; i < 4; i++) {
		if (znp->sons[i] != NULL)return false;
	}
	return true;
}
//���Ե����ڵ��Ƿ�ͨ��ztest�����û�б���ȫ�ڵ��ͷ���true����ģʽ
bool HieZbuffer::ztest_a_node(zbuffer_node* znp, BoundingBox& bb) {
	if (znp == NULL)return false;
	//���ų�boundingbox�����node
	if (znp->xl > bb.maxx || znp->xr <= bb.minx ||
		znp->yl > bb.maxy || znp->yr <= bb.miny)return false;
	//����ж�
	if (znp->minz >= bb.maxz)return false;
	else {
		if (is_leaf_node(znp))return true;
		//ֻҪ��һ���ӽڵ�test�ɹ����ͷ���true
		else return (ztest_a_node(znp->sons[0], bb) ||
			ztest_a_node(znp->sons[1], bb) ||
			ztest_a_node(znp->sons[2], bb) ||
			ztest_a_node(znp->sons[3], bb));
	}
}
//���Ե����ڵ��Ƿ�ͨ��ztest�����û�б���ȫ�ڵ��ͷ���true������ģʽ 
bool HieZbuffer::ztest_a_node(zbuffer_node* znp, OctreeNode* op, int faceIdx, BoundingBox& bb) {
	if (znp == NULL || op == NULL)return false;

	if (znp->xl > bb.maxx || znp->xr <= bb.minx ||
		znp->yl > bb.maxy || znp->yr <= bb.miny)return false;

	if (znp->minz >= bb.maxz)return false;
	else {
		if (otp->is_leaf_node(op))return true;
		else {
			return
				otp->findFace(op->sons[uln], faceIdx) && ztest_a_node(znp->sons[ul], op->sons[uln], faceIdx, bb) ||
				otp->findFace(op->sons[ulf], faceIdx) && ztest_a_node(znp->sons[ul], op->sons[ulf], faceIdx, bb) ||
				otp->findFace(op->sons[urn], faceIdx) && ztest_a_node(znp->sons[ur], op->sons[urn], faceIdx, bb) ||
				otp->findFace(op->sons[urf], faceIdx) && ztest_a_node(znp->sons[ur], op->sons[urf], faceIdx, bb) ||
				otp->findFace(op->sons[dln], faceIdx) && ztest_a_node(znp->sons[dl], op->sons[dln], faceIdx, bb) ||
				otp->findFace(op->sons[dlf], faceIdx) && ztest_a_node(znp->sons[dl], op->sons[dlf], faceIdx, bb) ||
				otp->findFace(op->sons[drn], faceIdx) && ztest_a_node(znp->sons[dr], op->sons[drn], faceIdx, bb) ||
				otp->findFace(op->sons[drf], faceIdx) && ztest_a_node(znp->sons[dr], op->sons[drf], faceIdx, bb);
		}
	}
}
//��ģʽ&�˲�������ģʽ����Ȳ���
void HieZbuffer::ztest(Mesh& mesh, bool octree_mode) {
	assert(hiezbuffer[0][0] == zbuffer_root);
	assert(hiezbuffer.size() == depth + 1);
	assert(hiezbuffer[depth].size() == width * height);

	if (octree_mode) {
		static Octree ot(width, height, mesh);
		otp = &ot;
	}
#pragma omp parallel for
	for (Face& face : mesh.faces) {
		zbuffer_node* znp = zbuffer_root;
		if (!octree_mode && !ztest_a_node(znp, face.bb)) {
			refused_cnt++;
		}
		else if (octree_mode && !ztest_a_node(znp, otp->oct_root, face.idx, face.bb)) {
			refused_cnt++;
		}
		else {
			drawFace(mesh, face); //���û�б���ȫ�ڵ����ͻ�������沢����zbuffer

		}
	}
	cout << refused_cnt << " faces have been refused!" << endl;
}
bool cmp_linetable(Linetable a, Linetable b) { return a > b; }
//��������ʱֻ�����ڲ����ӵĶ����,����zbuffer�Ƿ��޸�
bool HieZbuffer::drawFace(Mesh& mesh, Face& face) {
	if (face.idx == 4756) {
		int stipa = 1;
	}
	bool changed_zbuffer = false;

	if (fabs(face.parameter.C) <= 0.001) return changed_zbuffer;//ȥ���������ƽ�е���

	int dy = unsigned int(round(mesh.vertices[face.maxYVertexIdx].position.y)
		- round(mesh.vertices[face.minYVertexIdx].position.y));
	if (dy <= 0)return changed_zbuffer;

	vector<Linetable> linetables;
	//��ʼ������߱�����i->i+1�ı�
	for (int i = 0; i < face.vertexIdx.size(); i++) {
		int vid1 = face.vertexIdx[i];
		int vid2 = face.vertexIdx[(i + 1) % face.vertexIdx.size()];
		//y�������Ǹ�vertex����
		int maxvid = mesh.vertices[vid1].position.y > mesh.vertices[vid2].position.y ?
			vid1 : vid2;
		int minvid = maxvid == vid1 ? vid2 : vid1;

		vec3& maxp = mesh.vertices[maxvid].position;
		vec3& minp = mesh.vertices[minvid].position;

		Linetable line;

		line.x = maxp.x;
		line.y = maxp.y;
		line.z = maxp.z;
		line.dy = unsigned int(round(maxp.y) - round(minp.y));
		if (line.dy <= 0)continue;//����ˮƽ��

		line.dx = -(maxp.x - minp.x) / (maxp.y - minp.y);

		int index = int(round(maxp.y));

		//if (index >= h)index = h - 1;	//��ֹ���
		linetables.push_back(line);
	}
	sort(linetables.begin(), linetables.end(), cmp_linetable);

	//��ʼ����߱�
	Activated_Linetable aline;
	Linetable al = linetables[0];
	Linetable ar = linetables[1];
	if (al.x > ar.x)swap(al, ar);

	aline.xl = al.x;
	aline.dxl = al.dx;
	aline.dyl = al.dy;

	aline.xr = ar.x;
	aline.dxr = ar.dx;
	aline.dyr = ar.dy;

	//Ax+By+Cz+D=0

	aline.zl = al.z;
	aline.dzx = -face.parameter.A / face.parameter.C;
	aline.dzy = face.parameter.B / face.parameter.C;

	//��ʼ�����ж�
	int cnt = 2;
	int maxy = std::min(int(round(face.bb.maxy)), height - 1);
	int miny = std::max(int(round(face.bb.miny)), 0);
#pragma omp parallel for
	for (int y = maxy; y >= miny; y--) {
		if (aline.dyl <= 0 || aline.dyr <= 0)break;
		int minx = std::max(int(round(aline.xl)), 0);
		int maxx = std::min(int(round(aline.xr)), width - 1);
		float zx = aline.zl;
		if (maxx - minx >= 50) {
			int fffff = 1;
		}
#pragma omp parallel for
		for (int x = minx; x <= maxx; x++) {
			if (zx > hiezbuffer[depth][y * width + x]->minz) {
				bool changed_zbuffer = true;
				hiezbuffer[depth][y * width + x]->minz = zx;
				updateZbuffer(hiezbuffer[depth][y * width + x]->father);
#ifdef __DEEPMAP
				float c = (zx - FARTHEST) / (NEAREST - FARTHEST);
				vec3 dcolor = { c, c, c };
				full_framebuffer[y][x] = dcolor;
#else
				full_framebuffer[y][x] = face.color;		//����frame buffer
#endif	
			}
			zx += aline.dzx;
		}

		--aline.dyl;
		--aline.dyr;

		if (aline.dyl <= 0 && cnt < linetables.size()) {
			aline.xl = linetables[cnt].x;
			aline.zl = linetables[cnt].z;
			aline.dxl = linetables[cnt].dx;
			aline.dyl = linetables[cnt].dy;
			cnt++;
		}
		else {
			aline.xl += aline.dxl;
			aline.zl += aline.dzx * aline.dxl + aline.dzy;
		}

		if (aline.dyr <= 0 && cnt < linetables.size()) {
			aline.xr = linetables[cnt].x;
			aline.dxr = linetables[cnt].dx;
			aline.dyr = linetables[cnt].dy;
			cnt++;
		}
		else {
			aline.xr += aline.dxr;
		}

	}
	return changed_zbuffer;
}
void HieZbuffer::updateZbuffer(zbuffer_node* znp) {
	float minz_t = -MINZ;
	for (int i = 0; i < 4; i++) {
		if (znp->sons[i])minz_t = std::min(minz_t, znp->sons[i]->minz);
	}
	znp->minz = minz_t;
	if (znp->father)updateZbuffer(znp->father);
}