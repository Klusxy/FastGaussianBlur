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

		glTexImage2D(GL_TEXTURE_2D, 0, format, gImageWidth, gImageHeight, 0, format, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}
	else
		cout << "Failed to load texture" << endl;
	glBindTexture(GL_TEXTURE_2D, 0);
}