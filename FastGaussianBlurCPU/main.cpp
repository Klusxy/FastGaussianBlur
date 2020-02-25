#include <string>
#include <vector>
#include <iostream>
#include <fstream>
// gl
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

// const
const int gWindowWidth = 800;
const int gWindowHeight = 200;
GLfloat gTriangles[] =
{
	-1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
	1.0f,  -1.0f, 1.0f, 1.0f, 0.0f,
	-1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	1.0f,  -1.0f, 1.0f, 1.0f, 0.0f
};
GLuint gTriangleVAO = 0;
GLuint gTriangleVBO = 0;
GLuint gTex = 0;
GLuint gShowShader = 0;
double gTime = 0;
int gImageWidth = 0, gImageHeight = 0, gImageChannels = 0;

// callback fun
void FramebufferSizeCallback(GLFWwindow *window, int w, int h);
void ProcessInput(GLFWwindow *window);

// gl utils
GLuint InitProgram(string vertexShaderSource, string fragShaderSource);
GLuint InitShader(string shaderSource, GLenum shaderType);
string ReadFile(string filePath);
void InitTriangle();
void InitTexture();
void InitGL();
void DrawScene(GLFWwindow *window);

// callback
void FramebufferSizeCallback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}
void ProcessInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

int main()
{
	// init glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(gWindowWidth, gWindowHeight, "FastGaussianBlurCPU", NULL, NULL);
	if (window == NULL)
	{
		cout << "GLFW Window == NULL" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// registe callback
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

	// init glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "GLAD Init Failed" << endl;
		return -1;
	}

	InitGL();

	while (!glfwWindowShouldClose(window))
	{
		char windowTitle[50];
		sprintf_s(windowTitle, "FastGaussianBlurCPU: %.3f ms", gTime);
		glfwSetWindowTitle(window, windowTitle);

		// event
		glfwPollEvents();
		ProcessInput(window);

		// render
		DrawScene(window);
	}

	if (glIsVertexArray(gTriangleVAO))
	{
		glDeleteVertexArrays(1, &gTriangleVAO);
		gTriangleVAO = 0;
	}

	if (glIsBuffer(gTriangleVBO))
	{
		glDeleteBuffers(1, &gTriangleVBO);
		gTriangleVBO = 0;
	}

	if (glIsTexture(gTex))
	{
		glDeleteTextures(1, &gTex);
		gTex = 0;
	}

	if (gShowShader)
	{
		glDeleteProgram(gShowShader);
		gShowShader = 0;
	}

	// release glfw
	glfwTerminate();

	return 0;
}

GLuint InitProgram(string vertexShaderSource, string fragShaderSource)
{
	GLuint vertexShader = InitShader(vertexShaderSource, GL_VERTEX_SHADER);
	GLuint fragShader = InitShader(fragShaderSource, GL_FRAGMENT_SHADER);
	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);
	int  success;
	char infoLog[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		cout << "LINK PROGRAM ERROR:" << infoLog << endl;
	}
	else
		cout << "LINK PROGRAM SUCCESS" << endl;
	glUseProgram(program);
	// delete shader
	glDeleteShader(vertexShader);
	glDeleteShader(fragShader);
	return program;
}
GLuint InitShader(string shaderSource, GLenum shaderType)
{
	GLuint shader = glCreateShader(shaderType);
	const char* source = shaderSource.c_str();
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	int  success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		cout << "COMPILE SHADER ERROR:" << infoLog << endl;
	}
	else
		cout << "COMPILE SHADER SUCCESS" << endl;
	return shader;
}
void InitTriangle()
{
	glGenVertexArrays(1, &gTriangleVAO);
	glBindVertexArray(gTriangleVAO);

	glGenBuffers(1, &gTriangleVBO);
	glBindBuffer(GL_ARRAY_BUFFER, gTriangleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gTriangles), gTriangles, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, (void*)(sizeof(GLfloat) * 3));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
string ReadFile(string filePath) {
	char* txt = nullptr;
	string fileContent;
	ifstream fileContentStream(filePath, ios::in);
	if (fileContentStream.is_open()) {
		string line = "";
		while (getline(fileContentStream, line)) {
			fileContent += "\n" + line;
		}
		fileContentStream.close();
	}
	return fileContent;
}
void InitGL()
{
	gShowShader = InitProgram(ReadFile("../Shader/Show.vs"), ReadFile("../Shader/Show.fs"));
	InitTriangle();
	InitTexture();
}
void DrawScene(GLFWwindow *window)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(gTriangleVAO);
	glViewport(0, 0, gWindowWidth, gWindowHeight);
	glUseProgram(gShowShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTex);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glfwSwapBuffers(window);
}

double PI = 3.141592653589793;
vector<int> BoxesForGauss(int sigma, int n)  // standard deviation, number of boxes
{
	float wIdeal = sqrt((12 * sigma*sigma / n) + 2);  // Ideal averaging filter w 
	int wl = floor(wIdeal);
	if (wl % 2 == 0) wl--;
	int wu = wl + 2;

	float mIdeal = (12 * sigma*sigma - n*wl*wl - 4 * n*wl - 3 * n) / (-4 * wl - 4);
	int m = round(mIdeal);
	// var sigmaActual = Math.sqrt( (m*wl*wl + (n-m)*wu*wu - n)/12 );

	vector<int> sizes;
	for (int i = 0; i < n; i++)
		sizes.push_back(i < m ? wl : wu);
	return sizes;
}
// standard gaussian

void GaussianBlur(unsigned char *scl, unsigned char *tcl, int w, int h, int ch, int r)
{
	int radius = static_cast<int>(ceil(r * 2.57));
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w*ch; j += ch)
		{
			glm::vec3 color = glm::vec3(0.0f);
			float allWeights = 0.0f;
			for (int ix = i - radius; ix < i + radius + 1; ix++)
			{
				for (int iy = j - radius*ch; iy < j + (radius + 1)*ch; iy += ch)
				{
					int dsq = (iy / ch - j / ch)*(iy / ch - j / ch) + (ix - i)*(ix - i);// x^2 + y^2
					float weight = exp(-dsq / (2 * r * r)) / (PI * 2 * r * r);// gaussian function: 1/(2*pi*sgima^2) * e^(-(x^2+y^2)/(2*sigma^2))

					int x = glm::min((w - 1) * ch, glm::max(0, iy));
					int y = glm::min(h - 1, glm::max(0, ix));

					color.r += scl[y*w*ch + x] * weight;
					color.g += scl[y*w*ch + x + 1] * weight;
					color.b += scl[y*w*ch + x + 2] * weight;

					allWeights += weight;
				}
			}
			tcl[i*w*ch + j] = glm::round(color.r / allWeights);
			tcl[i*w*ch + j + 1] = glm::round(color.g / allWeights);
			tcl[i*w*ch + j + 2] = glm::round(color.b / allWeights);
		}
	}
}
// algorithm2
void BoxBlur_2(unsigned char *scl, unsigned char *tcl, int w, int h, int ch, int r)
{
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w*ch; j += ch)
		{
			glm::vec3 val = glm::vec3(0.0f);
			for (int ix = i - r; ix < i + r + 1; ix++)
			{
				for (int iy = j - r*ch; iy < j + (r + 1)*ch; iy += ch)
				{
					int x = glm::min((w - 1) * ch, glm::max(0, iy));
					int y = glm::min(h - 1, glm::max(0, ix));

					val.r += scl[y*w*ch + x];
					val.g += scl[y*w*ch + x + 1];
					val.b += scl[y*w*ch + x + 2];
				}
			}
			tcl[i*w*ch + j] = val.r / ((r + r + 1)*(r + r + 1));
			tcl[i*w*ch + j + 1] = val.g / ((r + r + 1)*(r + r + 1));
			tcl[i*w*ch + j + 2] = val.b / ((r + r + 1)*(r + r + 1));
		}
	}
}

void GaussianBlur2(unsigned char *scl, unsigned char *tcl, int w, int h, int ch, int r) {
	vector<int> bxs = BoxesForGauss(r, 3);
	BoxBlur_2(scl, tcl, w, h, ch, (bxs[0] - 1) / 2);
	BoxBlur_2(tcl, scl, w, h, ch, (bxs[1] - 1) / 2);
	BoxBlur_2(scl, tcl, w, h, ch, (bxs[2] - 1) / 2);
}

// algorithm3
void BoxBlurH_3(unsigned char *scl, unsigned char *tcl, int w, int h, int ch, int r)
{
	for (int i = 0; i < h; i++)
		for (int j = 0; j < w * ch; j += ch)
		{
			glm::vec3 color = glm::vec3(0.0f);
			for (int ix = j - r*ch; ix < j + r*ch + ch; ix += ch)
			{
				int x = glm::min(w * ch - ch, glm::max(0, ix));
				color.r += scl[i*w*ch + x];
				color.g += scl[i*w*ch + x + 1];
				color.b += scl[i*w*ch + x + 2];
			}
			tcl[i*w*ch + j] = color.r / (r + r + 1);
			tcl[i*w*ch + j + 1] = color.g / (r + r + 1);
			tcl[i*w*ch + j + 2] = color.b / (r + r + 1);
		}
}

void BoxBlurT_3(unsigned char *scl, unsigned char *tcl, int w, int h, int ch, int r)
{
	for (int i = 0; i < h; i++)
		for (int j = 0; j < w*ch; j += ch)
		{
			glm::vec3 color = glm::vec3(0.0f);
			for (int iy = i - r; iy < i + r + 1; iy++)
			{
				int y = glm::min(h - 1, glm::max(0, iy));
				color.r += scl[y*w*ch + j];
				color.g += scl[y*w*ch + j + 1];
				color.b += scl[y*w*ch + j + 2];
			}
			tcl[i*w*ch + j] = color.r / (r + r + 1);
			tcl[i*w*ch + j + 1] = color.g / (r + r + 1);
			tcl[i*w*ch + j + 2] = color.b / (r + r + 1);
		}
}

void BoxBlur_3(unsigned char *scl, unsigned char *tcl, int w, int h, int ch, int r)
{
	memcpy(tcl, scl, w*h*ch);
	BoxBlurH_3(tcl, scl, w, h, ch, r);
	BoxBlurT_3(scl, tcl, w, h, ch, r);
}

void GaussianBlur3(unsigned char *scl, unsigned char *tcl, int w, int h, int ch, int r) {
	vector<int> bxs = BoxesForGauss(r, 3);
	BoxBlur_3(scl, tcl, w, h, ch, (bxs[0] - 1) / 2);
	BoxBlur_3(tcl, scl, w, h, ch, (bxs[1] - 1) / 2);
	BoxBlur_3(scl, tcl, w, h, ch, (bxs[2] - 1) / 2);
}

// algorithm4
void BoxBlurH_4(unsigned char *scl, unsigned char *tcl, int w, int h, int ch, int r)
{
	float iarr = 1.0f / (r + r + 1.0f);
	for (int i = 0; i < h; i++) {
		int ti = i*w*ch;// middle index
		int li = ti;// left index
		int ri = ti + r*ch;// right index
		glm::vec3 fv = glm::vec3(scl[ti], scl[ti + 1], scl[ti + 2]);// first value
		glm::vec3 lv = glm::vec3(scl[ti + (w - 1)*ch], scl[ti + (w - 1)*ch + 1], scl[ti + (w - 1)*ch + 2]);// last value
		glm::vec3 val = glm::vec3(fv.r*(r + 1), fv.g*(r + 1), fv.b*(r + 1));// (r+1)/(2r+1)
		for (int j = 0; j < r*ch; j += ch)
		{
			val.r += scl[ti + j];
			val.g += scl[ti + j + 1];
			val.b += scl[ti + j + 2];
		}
		for (int j = 0; j <= r*ch; j += ch)
		{
			val.r += scl[ri] - fv.r;
			val.g += scl[ri + 1] - fv.g;
			val.b += scl[ri + 2] - fv.b;

			tcl[ti] = val.r*iarr;
			tcl[ti + 1] = val.g*iarr;
			tcl[ti + 2] = val.b*iarr;

			ri += ch;
			ti += ch;
		}
		for (int j = (r + 1)*ch; j < (w - r)*ch; j += ch)
		{
			val.r += scl[ri] - scl[li];
			val.g += scl[ri + 1] - scl[li + 1];
			val.b += scl[ri + 2] - scl[li + 2];

			tcl[ti] = val.r*iarr;
			tcl[ti + 1] = val.g*iarr;
			tcl[ti + 2] = val.b*iarr;

			ri += ch;
			li += ch;
			ti += ch;
		}
		for (int j = (w - r)*ch; j < w*ch; j += ch)
		{
			val.r += lv.r - scl[li];
			val.g += lv.g - scl[li + 1];
			val.b += lv.b - scl[li + 2];

			tcl[ti] = val.r*iarr;
			tcl[ti + 1] = val.g*iarr;
			tcl[ti + 2] = val.b*iarr;

			li += ch;
			ti += ch;
		}
	}
}

void BoxBlurT_4(unsigned char *scl, unsigned char *tcl, int w, int h, int ch, int r)
{
	float iarr = 1.0f / (r + r + 1.0f);
	for (int i = 0; i < w*ch; i += ch) {
		int ti = i;
		int li = ti;
		int ri = ti + r*w*ch;
		glm::vec3 fv = glm::vec3(scl[ti], scl[ti + 1], scl[ti + 2]);
		glm::vec3 lv = glm::vec3(scl[ti + w*(h - 1)*ch], scl[ti + w*(h - 1)*ch + 1], scl[ti + w*(h - 1)*ch + 2]);
		glm::vec3 val = glm::vec3((r + 1)*fv.r, (r + 1)*fv.g, (r + 1)*fv.b);
		for (int j = 0; j < r; j++)
		{
			val.r += scl[ti + j*w*ch];
			val.g += scl[ti + j*w*ch + 1];
			val.b += scl[ti + j*w*ch + 2];
		}
		for (int j = 0; j <= r; j++)
		{
			val.r += scl[ri] - fv.r;
			val.g += scl[ri + 1] - fv.g;
			val.b += scl[ri + 2] - fv.b;

			tcl[ti] = val.r*iarr;
			tcl[ti + 1] = val.g*iarr;
			tcl[ti + 2] = val.b*iarr;

			ri += w*ch;
			ti += w*ch;
		}
		for (int j = r + 1; j < h - r; j++)
		{
			val.r += scl[ri] - scl[li];
			val.g += scl[ri + 1] - scl[li + 1];
			val.b += scl[ri + 2] - scl[li + 2];

			tcl[ti] = val.r*iarr;
			tcl[ti + 1] = val.g*iarr;
			tcl[ti + 2] = val.b*iarr;

			li += w*ch;
			ri += w*ch;
			ti += w*ch;
		}
		for (int j = h - r; j < h; j++)
		{
			val.r += lv.r - scl[li];
			val.g += lv.g - scl[li + 1];
			val.b += lv.b - scl[li + 2];

			tcl[ti] = val.r*iarr;
			tcl[ti + 1] = val.g*iarr;
			tcl[ti + 2] = val.b*iarr;

			li += w*ch;
			ti += w*ch;
		}
	}
}

void BoxBlur_4(unsigned char *scl, unsigned char *tcl, int w, int h, int ch, int r)
{
	memcpy(tcl, scl, w*h*ch);
	BoxBlurH_4(tcl, scl, w, h, ch, r);
	BoxBlurT_4(scl, tcl, w, h, ch, r);
}

void GaussianBlur4(unsigned char *scl, unsigned char *tcl, int w, int h, int ch, int r)
{
	vector<int> bxs = BoxesForGauss(r, 3);
	BoxBlur_4(scl, tcl, w, h, ch, (bxs[0] - 1) / 2);
	BoxBlur_4(tcl, scl, w, h, ch, (bxs[1] - 1) / 2);
	BoxBlur_4(scl, tcl, w, h, ch, (bxs[2] - 1) / 2);
}

void InitTexture()
{
	glGenTextures(1, &gTex);
	glBindTexture(GL_TEXTURE_2D, gTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load("../images/800x200.png", &gImageWidth, &gImageHeight, &gImageChannels, 0);

	if (data)
	{
		GLint format = 0;
		if (gImageChannels == 4)
			format = GL_RGBA;
		if (gImageChannels == 3)
			format = GL_RGB;

		double firstTime = glfwGetTime();
		// cpu gaussian blur
		unsigned char *new_data = new unsigned char[gImageWidth * gImageHeight * gImageChannels];

		// standard gaussian
		// GaussianBlur(data, new_data, gImageWidth, gImageHeight, gImageChannels, 5.0f);
		// algorithm2
		// GaussianBlur2(data, new_data, gImageWidth, gImageHeight, gImageChannels, 5.0f);
		// algorithm3
		// GaussianBlur3(data, new_data, gImageWidth, gImageHeight, gImageChannels, 5.0f);
		// algorithm4
		GaussianBlur4(data, new_data, gImageWidth, gImageHeight, gImageChannels, 5.0f);

		double secondTime = glfwGetTime();
		gTime = secondTime - firstTime;

		glTexImage2D(GL_TEXTURE_2D, 0, format, gImageWidth, gImageHeight, 0, format, GL_UNSIGNED_BYTE, new_data);
		stbi_image_free(data);
		delete[] new_data;
	}
	else
		cout << "Failed to load texture" << endl;
	glBindTexture(GL_TEXTURE_2D, 0);
}