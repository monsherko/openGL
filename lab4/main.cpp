#define _USE_MATH_DEFINES
#define GLEW_STATIC
//построчного заполнения с затравкой для четырехсвязной гранично-определенной области;
// целочисленный алгоритм Брезенхема с устранением ступенчатости;
#include <algorithm>
#include <vector>
#include <functional>
#include <memory>
#include <stack>
#include <iostream>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct Point {
	Point() : Point(0, 0) { }

	Point(int x, int y) : X(x), Y(y) { }

	int X;
	int Y;
};

struct Pixel {
	Pixel() : Pixel(0.0f, 0.0f, 1.0f) { }

	Pixel(float r, float g, float b) : R(r), G(g), B(b) { }

	bool IsWhite() { return R == 0.0f && G == 0.0f && B == 0.0f; }

        void RedFunc(float k) {
            B = k;
        }

	float R;
	float G;
	float B;
};

int Width, Height;
std::vector<Point> Points;
std::unique_ptr<Pixel[]> Pixels;



std::function<int(int)> sign = [](int x) { return x > 0 ? 1 : x < 0 ? -1 : 0; };

void Clear() {
	std::fill(Pixels.get(), Pixels.get() + Width * Height, Pixel(0, 0, 0));
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

		Pixels[(Height - p.Y) * Width + p.X].RedFunc(1.0);

		for (Point adjacent : { Point(-1, 0), Point(0, -1), Point(1, 0), Point(0, 1) }) {
			int pos = (Height - (p.Y + adjacent.Y)) * Width + (p.X + adjacent.X);
			if (pos >= 0 && pos < Width * Height && Pixels[pos].IsWhite()) {
				s.emplace(p.X + adjacent.X, p.Y + adjacent.Y);
			}
		}
	}
}
void Filter() {

      const double I = 2;

       Clear();


      for (size_t i = 0; i < Points.size(); i++) {
        int x = Points[i].X;
        int y = Points[i].Y;

        Point end = Points[(i + 1) % Points.size()];
        double x_End = end.X;
        double y_End = end.Y;

        int dx = abs(x - end.X);
        int dy = abs(y - end.Y);
        double m, w;

        double e = I/2;
        int s1 = sign(y - y_End);
        int s2 = sign(x - x_End);
        int sign_X;
        int sign_Y;

        bool compDyDx = (dy> dx) ? true : false;



        m = compDyDx? (double)(I * dx) / (double)dy : (I * dy) / dx;

        w = I - m;
         Pixels[(Height - y) * Width + x].RedFunc(0.5*e);
         if (s1 == 1 ){
             int tmp = x; x = x_End; x_End = tmp;
             tmp = y; y = y_End; y_End = tmp;
         }
         if (((s2 == -1 && compDyDx) || (s2 == 1 && !compDyDx))){
             int tmp = x; x = x_End; x_End = tmp;
             tmp = y; y = y_End; y_End = tmp;
         }
         if ((s1 == 1 && !compDyDx) || ((sign(x - x_End) == 1) && compDyDx)) {
             sign_X = -1;
         }  else {
             sign_X = 1;
         }
         if ((s2 == 1 && !compDyDx) || ((sign(y - y_End) == 1) && compDyDx)) {
             sign_Y = -1;
         }
         else  {
             sign_Y = 1;
         }

         for (; compDyDx && sign_Y*y < sign_Y*y_End;  Pixels[(Height - y) * Width + x].RedFunc(0.5*e)) {
             y += sign_Y;
             if (e < w) {
                 e += m;
             } else {
                 x += sign_X;
                 e -= w;
             }
         }
         for (; !compDyDx && sign_X*x < sign_X*x_End; Pixels[(Height - y) * Width + x].RedFunc(0.5*e)) {
             x += sign_X;
             if (e < w) {
                 e += m;
             } else {
                 y += sign_Y;
                 e -= w;
             }
         }
      }


 }



void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	if ( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
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

        glPointSize(4.0);
	while (glfwWindowShouldClose(window) == GL_FALSE) {
		glDrawPixels(Width, Height, GL_RGB, GL_FLOAT, Pixels.get());

		glfwSwapBuffers(window);
       glfwPollEvents();
		glfwWaitEvents();
	}

	glfwTerminate();

	return 0;
}
