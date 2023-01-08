#include "mesh.h"

Mesh::Mesh() { ; }
Mesh::~Mesh() { ; }

void Mesh::init(const string& path) {
	unsigned int seed = RANDOM_SEED;
	srand(seed);
	clock_t t1 = clock();
	loadOBJ(path);
	clock_t t2 = clock();

	cout << "Model load success!" << endl
		<< "Vertices: " << vertices.size() << endl
		<< "Faces: " << faces.size() << endl
		<< "Use time: " << t2 - t1 << " ms" << endl;
}

void Mesh::loadOBJ(const string& path) {
	ifstream file(path);
	if (!file.is_open()) {
		cout << "File open failed!";
		exit(1);
	}
	string type;
	while (file >> type) {

		// new mesh
		//if (type == "o") {}
		// vertex
		if (type == "v") {
			Vertex v;
			file >> v.position.x >> v.position.y >> v.position.z;
			vertices.push_back(v);
		}
		//// texcoords
		//else if (type == "vt") {
		//	vec2 vt;
		//	file >> vt.x >> vt.y;
		//	textures.push_back(vt);
		//}
		// normal, 如果obj文件中有法向量，可以直接读入
		/*else if (type == "vn") {
			vec3 vn;
			file >> vn.x >> vn.y >> vn.z;
			normals.push_back(vn);
		}*/
		// indices
		else if (type == "f") {
			Face face;
			char ch;
			while (ch = file.get()) {
				if (ch == '\n' || ch == EOF)break;
				int vi = 0, ti = 0, ni = 0;
				string str;
				file >> str;
				vi = stoi(str.substr(0, str.find_first_of('/')));
				/*if (str.find_first_of('/') + 1 != str.find_last_of('/')) {
					ti = stoi(str.substr(str.find_first_of('/') + 1, str.find_last_of('/')));
				}
				ni = stoi(str.substr(str.find_last_not_of('/'), str.size()));*/
				face.vertexIdx.push_back(vi - 1);
				//face.textureIdx.push_back(ti-1);
				//face.normalIdx.push_back(ni-1);
			}


			//如果obj文件中没有颜色信息，则自己随机创建一个
#ifdef RANDER_COLOR
			face.color = randFaceColor();
#endif			
			faces.push_back(face);
		}
		else {
			getline(file, type);
		}
	}
	file.close();
}

bool Mesh::sortVertexIdxYMAX(int a, int b) {
	return vertices[a].position.y > vertices[b].position.y;
}

vec3 Mesh::randFaceColor() {

	float r = float(1.0 * rand() / RAND_MAX);
	float g = float(1.0 * rand() / RAND_MAX);
	float b = float(1.0 * rand() / RAND_MAX);
	vec3 color = { r, g, b };
	return color;
}

//找到最大y坐标和最小y坐标,计算平面方程
void Mesh::preprossess() {
	int cnt = 0;
#pragma omp parallel for
	for (Face& face : faces) {
		face.idx = cnt++;
		face.maxYVertexIdx = face.vertexIdx[0];
		face.minYVertexIdx = face.vertexIdx[0];
#pragma omp parallel for
		for (int& idx : face.vertexIdx) {
			face.bb.maxx = std::max(face.bb.maxx, vertices[idx].position.x);
			face.bb.minx = std::min(face.bb.minx, vertices[idx].position.x);
			face.bb.maxy = std::max(face.bb.maxy, vertices[idx].position.y);
			face.bb.miny = std::min(face.bb.miny, vertices[idx].position.y);
			face.bb.maxz = std::max(face.bb.maxz, vertices[idx].position.z);
			face.bb.minz = std::min(face.bb.minz, vertices[idx].position.z);

			if (vertices[idx].position.y > vertices[face.maxYVertexIdx].position.y)
				face.maxYVertexIdx = idx;
			if (vertices[idx].position.y < vertices[face.minYVertexIdx].position.y)
				face.minYVertexIdx = idx;
		}
		if (face.vertexIdx.size() > 2)		//取前三个点计算平面方程
		{
			vec3 &a = vertices[face.vertexIdx[0]].position,
				&b = vertices[face.vertexIdx[1]].position,
				&c = vertices[face.vertexIdx[2]].position;

			Parameter parameter = computePlaneParameter(a, b, c);
			face.parameter = parameter;
		}
	}
}