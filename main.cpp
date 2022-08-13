#include <GL/freeglut.h>
#include <GL/gl.h>

#include <chrono>
#include <cstdio>
#include <random>
#include <vector>

#include "DirectXMath.h"

using namespace DirectX;

static unsigned pack(unsigned r, unsigned g, unsigned b) { return b << 16 | g << 8 | r; }

struct sphere
{
	XMFLOAT4 position;
	float radius;
	unsigned color;
};

static const int WIDTH = 1280, HEIGHT = 720;
static std::vector<unsigned> BACKBUFFER;
static std::vector<sphere> SPHERES;

// TODO:
// - Spheres are not rendered perspectively correctly, only approximatively.
// - There is no depth buffering.
// - Maximize performance.
// - Bonus: Implement a simple shading model.
static void render(const std::vector<sphere> &spheres, XMVECTOR eye, XMVECTOR target, float fov, XMMATRIX model2world, std::vector<unsigned> &out_backbuffer)
{
	auto world2eye = XMMatrixLookAtRH(eye, target, XMVectorSet(0, 1, 0, 0));
	auto eye2clip = XMMatrixPerspectiveFovRH(fov, (double)WIDTH / HEIGHT, 1, 100);
	auto model2clip = model2world * world2eye * eye2clip;

	std::fill(out_backbuffer.begin(), out_backbuffer.end(), 0);

	for (const auto &sphere : spheres)
	{
		auto center = XMLoadFloat4(&sphere.position);
		auto clip = XMVector4Transform(center, model2clip);
		auto w = XMVectorGetW(clip);
		clip = XMVectorDivide(clip, XMVectorSplatW(clip));
		int screenX = (XMVectorGetX(clip) * 0.5 + 0.5) * WIDTH;
		int screenY = (XMVectorGetY(clip) * 0.5 + 0.5) * HEIGHT;

		float size = sphere.radius / w * WIDTH;

		for (int x = screenX - size; x <= screenX + size; x++)
			for (int y = screenY - size; y <= screenY + size; y++)
			{
				if (sqrt(std::pow(x - screenX, 2) + std::pow(y - screenY, 2)) > size)
					continue;

				int idx = x + (HEIGHT - 1 - y) * WIDTH;
				out_backbuffer[idx] = sphere.color;
			}
	}
}

static void idle()
{
	using namespace std::chrono;

	static double angle = 0;

	auto model2world = XMMatrixRotationAxis(XMVectorSet(0, 1, 0, 0), angle);

	auto start = high_resolution_clock::now();
	render(SPHERES, XMVectorSet(0, 0, 40, 1), XMVectorSet(0, 0, 0, 1), M_PI / 4, model2world, BACKBUFFER);
	double dur = (duration_cast<microseconds>(high_resolution_clock::now() - start).count() + 1) * 1e-6;
	char fps[16];
	std::sprintf(fps, "FPS: %.1f", 1 / dur);
	glutSetWindowTitle(fps);

	angle += M_PI / 4 * dur;

	glutPostRedisplay();
}

static void display()
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0,
				 GL_RGBA, GL_UNSIGNED_BYTE, BACKBUFFER.data());

	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 0.0);
	glVertex2d(-1.0, 1.0);
	glTexCoord2d(1.0, 0.0);
	glVertex2d(1.0, 1.0);
	glTexCoord2d(1.0, 1.0);
	glVertex2d(1.0, -1.0);
	glTexCoord2d(0.0, 1.0);
	glVertex2d(-1.0, -1.0);
	glEnd();

	glutSwapBuffers();
}

int main(int argc, char **argv)
{
	BACKBUFFER.resize(WIDTH * HEIGHT);
	SPHERES.resize(2000);

	std::mt19937 rng(1337);
	std::uniform_real_distribution<float> iid_pos(-10, 10);
	std::uniform_real_distribution<float> iid_rad(0.1, 0.3);
	std::uniform_int_distribution iid_col(102, 255);
	for (auto &sphere : SPHERES)
	{
		sphere.position = {iid_pos(rng), iid_pos(rng), iid_pos(rng), 1};
		sphere.radius = iid_rad(rng);
		sphere.color = pack(iid_col(rng), iid_col(rng), iid_col(rng));
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Hiring Test");
	glutIdleFunc(idle);
	glutDisplayFunc(display);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glEnable(GL_TEXTURE_2D);

	glutMainLoop();
}