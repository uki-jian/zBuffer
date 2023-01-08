#include "super.h"
#include "camera.h"

void Camera::model2screen(Mesh& mesh, int width, int height, frame_counter& fc, mat4& trans_mat) {

	assert(width == height);


#ifdef __ROTATE
	mat4 rot(1.0f);
	rot = rotate(rot, (float)5 * radians(1.0f), vec3(0.0f, 1.0f, 0.0f));
	trans_mat *= rot;
#endif

	cout << "Rendering Counts: " << ++fc.frame_counts << " ";
	int pos = fc.frame_counts % fc.buffer_size;
	fc.time_buffer[pos] = clock();
	if (fc.frame_counts >= 10) {
		fc.fps = (float)fc.buffer_size / (fc.time_buffer[pos] - fc.time_buffer[(pos + 1) % fc.buffer_size]) * CLOCKS_PER_SEC;
		cout << "FPS: " << fc.fps << endl;
	}
	else cout << endl;

	for (auto& vertex : mesh.vertices) {
		vec4 newPos = trans_mat * vec4(vertex.position, 1.0);
		newPos /= newPos.w;
		vertex.position = { newPos.x, newPos.y, newPos.z };
	}

	float maxx = MAXPOS, minx = MINPOS,
		maxy = MAXPOS, miny = MINPOS,
		maxz = MAXPOS, minz = MINPOS,
		maxdis = MAXPOS;
	for (auto& vertax : mesh.vertices) {
		if (vertax.position.x > maxx)maxx = vertax.position.x;
		if (vertax.position.x < minx)minx = vertax.position.x;
		if (vertax.position.y > maxy)maxy = vertax.position.y;
		if (vertax.position.y < miny)miny = vertax.position.y;
		if (vertax.position.z > maxz)maxz = vertax.position.z;
		if (vertax.position.z < minz)minz = vertax.position.z;
	}
	if (maxx < minx || maxy < miny || maxz < minz) {
		cout << "Invalid position! (Max < Min)" << endl;
		exit(1);
	}
	maxdis = std::max(float(maxx - minx), float(maxy - miny));
	for (auto& vertex : mesh.vertices) {
		vertex.position.x = (vertex.position.x - minx) / maxdis;
		vertex.position.y = (vertex.position.y - miny) / maxdis;

		//将z坐标映射到[FARTHEST, NEAREST]
		vertex.position.z = (vertex.position.z - minz) / (maxz - minz) * (NEAREST - FARTHEST) + FARTHEST;

		vertex.position.x *= width;
		vertex.position.y *= height;

		//防止在屏幕边界溢出
		float shrink = 0.7;
		float offset = (1 - shrink) / 2;
		vertex.position.x = vertex.position.x * shrink + offset * width;
		vertex.position.y = vertex.position.y * shrink + offset * height;

	}
	mesh.preprossess();
}