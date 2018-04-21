#include <algorithm>
#include <vector>
#include <functional>
#include <memory>
#include <stack>
#include <iostream>
#include <cmath>
#include <GLFW/glfw3.h>

struct Point {
	Point() : Point(0, 0) { }

	Point(int x, int y) : X(x), Y(y) { }

	int X;
	int Y;
};

struct Pixel {
	Pixel() : Pixel(0.0f, 0.0f, 0.0f) { }

	Pixel(float r, float g, float b) : R(r), G(g), B(b) { }

	bool IsWhite() { return R == 1.0f && G == 1.0f && B == 1.0f; }

	float R;
	float G;
	float B;
};

int Width, Height;
std::unique_ptr<Pixel[]> Pixels;
std::vector<Point> Points;

void Clear() {
	std::fill(Pixels.get(), Pixels.get() + Width * Height, Pixel(1, 1, 1));
}

void Draw() {
	Clear();

	std::function<int(int)> sign = [](int x) { return x > 0 ? 1 : x < 0 ? -1 : 0; };

	for (size_t i = 0; i < Points.size(); i++) {
		int x = Points[i].X;
		int y = Points[i].Y;

		Point end = Points[(i + 1) % Points.size()];

		int dx = abs(x - end.X);
		int dy = abs(y - end.Y);

		int error = dx - dy;

		while (x != end.X || y != end.Y) {
			Pixels[(Height - y) * Width + x] = Pixel();

			int err = error * 2;
			if (err > -dy) {
				error -= dy;
				x += sign(end.X - x);
			}
			if (err < dx) {
				error += dx;
				y += sign(end.Y - y);
			}
		}
		Pixels[(Height - end.Y) * Width + end.X] = Pixel();
	}
}

void Fill(int x, int y) {
	Draw();

	std::stack<Point> s;
	s.emplace(x, y);

	while (!s.empty()) {
		Point p = s.top();
		s.pop();
		
		Pixels[(Height - p.Y) * Width + p.X] = Pixel();

		for (Point adjacent : { Point(-1, 0), Point(0, -1), Point(1, 0), Point(0, 1) }) {
			int pos = (Height - (p.Y + adjacent.Y)) * Width + (p.X + adjacent.X);
			if (pos >= 0 && pos < Width * Height && Pixels[pos].IsWhite()) {
				s.emplace(p.X + adjacent.X, p.Y + adjacent.Y);
			}
		}
	}
}

void Filter() {
	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);

	std::unique_ptr<Pixel[]> buffer = std::unique_ptr<Pixel[]>(new Pixel[Width * Height]);
	std::copy(Pixels.get(), Pixels.get() + Width * Height, buffer.get());

	glDrawPixels(Width, Height, GL_RGB, GL_FLOAT, buffer.get());
	glAccum(GL_LOAD, 1.0f / 4.0f);

	for (Point adjacent : { Point(-1, 0), Point(0, -1), Point(1, 0), Point(0, 1) }) {
		for (int x = 0; x < Width; x++) {
			for (int y = 0; y < Height; y++) {
				int pos = (y + adjacent.Y) * Width + (x + adjacent.X);
				if (pos >= 0 && pos < Width * Height) {
					buffer[y * Width + x] = Pixels[pos];
				} else {
					buffer[y * Width + x] = Pixel(1.0f, 1.0f, 1.0f);
				}
			}
		}

		glDrawPixels(Width, Height, GL_RGB, GL_FLOAT, buffer.get());
		glAccum(GL_ACCUM, 1.0f / 8.0f);
	}

	for (Point adjacent : { Point(-1, -1), Point(1, 1), Point(1, -1), Point(-1, 1) }) {
		for (int x = 0; x < Width; x++) {
			for (int y = 0; y < Height; y++) {
				int pos = (y + adjacent.Y) * Width + (x + adjacent.X);
				if (pos >= 0 && pos < Width * Height) {
					buffer[y * Width + x] = Pixels[pos];
				} else {
					buffer[y * Width + x] = Pixel(1.0f, 1.0f, 1.0f);
				}
			}
		}

		glDrawPixels(Width, Height, GL_RGB, GL_FLOAT, buffer.get());
		glAccum(GL_ACCUM, 1.0f / 16.0f);
	}

	glAccum(GL_RETURN, 1.0f);

	glReadPixels(0, 0, Width, Height, GL_RGB, GL_FLOAT, Pixels.get());
}

void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		Points.emplace_back((int) floor(x), (int) floor(y));
		Draw();
	} else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		Fill((int) floor(x), (int) floor(y));
	}
}

void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		Filter();
	} else if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		Draw();
	} else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		Points.clear();
		Draw();
	}
}

void FrameBufferSizeCallback(GLFWwindow *window, int width, int height) {
	Pixels.release();
	Pixels = std::unique_ptr<Pixel[]>(new Pixel[width * height]);
	Points.clear();

	Width = width;
	Height = height;

	Draw();
}

int main() {
	Width = 1280, Height = 720;

	glfwInit();

	GLFWwindow *window = glfwCreateWindow(Width, Height, "created by sher", NULL, NULL);

	glfwMakeContextCurrent(window);

	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetFramebufferSizeCallback(window, FrameBufferSizeCallback);

	FrameBufferSizeCallback(window, Width, Height);

	while (glfwWindowShouldClose(window) == GL_FALSE) {
		glDrawPixels(Width, Height, GL_RGB, GL_FLOAT, Pixels.get());

		glfwSwapBuffers(window);

		glfwWaitEvents();
	}

	glfwTerminate();

	return 0;
}