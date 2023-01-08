
#include "super.h"
#include "mesh.h"
#include "camera.h"
#include "scanlineZbuffer.h"
#include "hieZbuffer.h"
#include "octree.h"

using namespace std;
using namespace glm;

int MODE;
#define __SCANLINE 0
#define __SIMPLEHIE 1
#define __OCTHIE 2

glm::vec3 full_framebuffer[SCR_HEIGHT][SCR_WIDTH];

// 视角变换矩阵
mat4 trans_mat(1.0f);
frame_counter fc;

Mesh mesh;

int WINDOWIDX;
vector<float> speed_monitor;
clock_t setFramebuffer(clock_t t1) {
	clock_t t2 = clock();
	cout << "rendering time: " << t2 - t1 << " ms" << endl;
	
	if (speed_monitor.size() == 36) {
		float ave = 0;
		for (int i = 0; i < speed_monitor.size(); i++)ave += speed_monitor[i];
		ave /= speed_monitor.size();
		cout << "average rendering time: " << ave << " ms" << endl;
	}
	else speed_monitor.push_back(t2 - t1);

#pragma omp parallel for
	for (int y = 0; y < SCR_HEIGHT; ++y) {
#pragma omp parallel for
		for (int x = 0; x < SCR_WIDTH; ++x) {

			vec3& rgb = full_framebuffer[y][x];
			glColor3f(rgb.r, rgb.g, rgb.b);
			glVertex2i(x, y);
		}
	}
	return t2;
}

void mydisplay() {
	
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, SCR_WIDTH, 0, SCR_HEIGHT);

	int height = SCR_HEIGHT, width = SCR_WIDTH;
	int cnt = 0;
#ifdef __ROTATE
	for (; 1; cnt++) {
#endif
		glBegin(GL_POINTS);
		clock_t t1 = clock();
		Mesh mesh_t = mesh;
		Camera::model2screen(mesh_t, SCR_WIDTH, SCR_HEIGHT, fc, trans_mat);
		
		if (MODE == __SCANLINE) {
			ScanLineZbuffer zbuffer(mesh_t, SCR_WIDTH, SCR_HEIGHT);
			zbuffer.setBuffer();
		}
		else if (MODE == __SIMPLEHIE) {
			HieZbuffer hb(SCR_WIDTH, SCR_HEIGHT);
			hb.ztest(mesh_t, false);
		}
		else {
			HieZbuffer hb(SCR_WIDTH, SCR_HEIGHT);
			hb.ztest(mesh_t, true);
		}
		t1 = setFramebuffer(t1);
		glEnd();
		glFinish();
#ifdef __ROTATE
	}
#endif
	
}
//键盘控制： left right up down pageUp pageDown旋转模型， esc退出
void myspecialkeyboard(GLint key, GLint x, GLint y) {

	mat4 rot(1.0f);
	if (key == GLUT_KEY_LEFT) {	
		rot = rotate(rot, (float)10 * radians(1.0f), vec3(0.0f, 1.0f, 0.0f));	
		cout << "left" << endl;
	}

	if (key == GLUT_KEY_RIGHT) {
		rot = rotate(rot, (float)-10 * radians(1.0f), vec3(0.0f, 1.0f, 0.0f));
		cout << "right" << endl;
	}

	if (key == GLUT_KEY_UP) {
		rot = rotate(rot, (float)10 * radians(1.0f), vec3(1.0f, 0.0f, 0.0f));
		cout << "up" << endl;
	}

	if (key == GLUT_KEY_DOWN) {
		rot = rotate(rot, (float)-10 * radians(1.0f), vec3(1.0f, 0.0f, 0.0f));
		cout << "down" << endl;
	}
	if (key == GLUT_KEY_PAGE_UP) {
		rot = rotate(rot, (float)10 * radians(1.0f), vec3(0.0f, 0.0f, 1.0f));
		cout << "up" << endl;
	}

	if (key == GLUT_KEY_PAGE_DOWN) {
		rot = rotate(rot, (float)-10 * radians(1.0f), vec3(0.0f, 0.0f, 1.0f));
		cout << "down" << endl;
	}
	trans_mat = trans_mat * rot;
	mydisplay();
}
void mykeyboard(unsigned char key, int x, int y) {
	if (key == 27) {
		glutDestroyWindow(WINDOWIDX);
		cout << "window destroyed" << endl;
		exit(0);
	}
}
void initWindow() {

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
	glutInitWindowPosition(800, 0);
	WINDOWIDX = glutCreateWindow("Z-Buffer");
}
void initViewport() {
	mat4 model(1.0f);			//将顶点坐标变换为世界坐标（旋转位移缩放）
    //model = scale(model, vec3(0.5, 0.5, 0.5));			//缩放
	//model = rotate(model, (float)45 * radians(1.0f), vec3(1.0f, 0.0f, 0.0f));
	//model = rotate(model, (float)40 * radians(1.0f), vec3(0.0f, 1.0f, 0.0f));
	//model = glm::rotate(model, (float)45 * glm::radians(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	mat4 view(1.0f);			//将世界坐标变换为观察坐标（摄像机窗口）
	view = translate(view, vec3(0.0f, 0.0f, -8.0f));

	mat4 projection(1.0f);		//建立透视投影
	projection = perspective(radians(45.0f), float(SCR_WIDTH) / float(SCR_HEIGHT), -0.1f, 1.1f);
	
	trans_mat = projection * view * model;
}

int main(int argc, char** argv) {

	if (argc != 3) {
		cout << "Usage: zbuffer [a] [b]" << endl
			<< "[a]: 0 for scanline zbuffer" << endl
			<< "     1 for hierarchical zbuffer(simple)" << endl
			<< "     2 for hierarchical zbuffer(octree)" << endl
			<< "[b]: the name of model(.obj), which is included in 'model/'" << endl
			<< "sample: zbuffer 2 miku.obj" << endl;
		exit(0);
	}
	if (*argv[1] == '0') {
		MODE = __SCANLINE;
		cout << "scanline zbuffer mode on" << endl;
	}
	else if (*argv[1] == '1') {
		MODE = __SIMPLEHIE;
		cout << "hierarchical zbuffer mode on(Simple)" << endl;
	}
	else if(*argv[1] == '2') {
		MODE = __OCTHIE;
		cout << "hierarchical zbuffer mode on(Octree)" << endl;
	}

	mesh.init("../model/" + string(argv[2]));

	cout << "==============================================================" << endl
		<< "			[Menu]" << endl
		<< "use [up] [down] [left] [right] [PageUp] [PageDown] to rotate this model" << endl
		<< "use [Esc] to exit" << endl
		<< "==============================================================" << endl;
	initWindow();
	initViewport();
	glutDisplayFunc(mydisplay);
	glutKeyboardFunc(mykeyboard);
	glutSpecialFunc(myspecialkeyboard);
	glutMainLoop();
	return 0;
}