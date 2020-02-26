#include <string>
#include <vector>
#include <iostream>
#include <fstream>
// imgui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
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
// gaussian blur const
const int compareW = 1920;
const int compareH = 1080;
const float max_scale = 4.0f; // 2^max_scale
const int minBlurLevel = 0;
const int maxBlurLevel = 100;
static int BlurLevel = 0;

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
GLuint gGaussianBlurShader = 0;
double gTime = 0;
GLuint gFbo = 0;
GLuint gDownSampleTex = 0;
GLuint gGaussianBlurHTex = 0;
GLuint gGaussianBlurVTex = 0;
// image info
int gImageWidth = 0, gImageHeight = 0, gImageChannels = 0;
int gImageScaleWidth = 0, gImageScaleHeight = 0;

// callback fun
void FramebufferSizeCallback(GLFWwindow *window, int w, int h);
void ProcessInput(GLFWwindow *window);

// gl utils
GLuint InitProgram(string vertexShaderSource, string fragShaderSource);
GLuint InitShader(string shaderSource, GLenum shaderType);
string ReadFile(string filePath);
void InitTriangle();
void LoadImage();
void UpdateGaussianBlurTexture();
GLuint Create2DTexture(GLint format, int w, int h, unsigned char* data = nullptr);
void Delete2DTexture(GLuint& tex);
void InitGL();
void DrawScene(GLFWwindow *window);
void CalculateScaleWH(int rawW, int rawH);
void BlurFactory(int phase, GLuint inputTex, GLuint outputTex);

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
	glfwSwapInterval(0);

	// registe callback
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

	// init glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "GLAD Init Failed" << endl;
		return -1;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	const char* glsl_version = "#version 330 core";
	ImGui_ImplOpenGL3_Init(glsl_version);

	InitGL();

	while (!glfwWindowShouldClose(window))
	{
		char windowTitle[50];
		sprintf_s(windowTitle, "FastGaussianBlurCPU: %.4f ms", gTime);
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

	Delete2DTexture(gTex);
	Delete2DTexture(gDownSampleTex);
	Delete2DTexture(gGaussianBlurHTex);
	Delete2DTexture(gGaussianBlurVTex);

	if (glIsFramebuffer(gFbo))
	{
		glDeleteFramebuffers(1, &gFbo);
		gFbo = 0;
	}

	if (gShowShader)
	{
		glDeleteProgram(gShowShader);
		gShowShader = 0;
	}

	if (gGaussianBlurShader)
	{
		glDeleteProgram(gGaussianBlurShader);
		gGaussianBlurShader = 0;
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
	gGaussianBlurShader = InitProgram(ReadFile("../Shader/Show.vs"), ReadFile("../Shader/GaussianBlur.fs"));
	InitTriangle();
	LoadImage();
}
GLuint Create2DTexture(GLint format, int w, int h, unsigned char* data)
{
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	return tex;
}
void Delete2DTexture(GLuint& tex)
{
	if (glIsTexture(tex))
	{
		glDeleteTextures(1, &tex);
		tex = 0;
	}
}

void UpdateGaussianBlurTexture()
{
	Delete2DTexture(gDownSampleTex);
	Delete2DTexture(gGaussianBlurHTex);
	Delete2DTexture(gGaussianBlurVTex);
	gDownSampleTex = Create2DTexture(GL_RGBA, gImageScaleWidth, gImageScaleHeight);
	gGaussianBlurHTex = Create2DTexture(GL_RGBA, gImageScaleWidth, gImageScaleHeight);
	gGaussianBlurVTex = Create2DTexture(GL_RGBA, gImageScaleWidth, gImageScaleHeight);
}

void CalculateScaleWH(int rawW, int rawH)
{
	float shrinkFactor = 1.0f;
	if (rawW > compareW || rawH > compareH)
	{
		float wFactor = static_cast<float>(rawW) / static_cast<float>(compareW);
		float hFactor = static_cast<float>(rawH) / static_cast<float>(compareH);
		shrinkFactor = wFactor > hFactor ? wFactor : hFactor;
	}
	float weightFactor = pow(2, static_cast<float>(BlurLevel) / maxBlurLevel * max_scale);
	int scaleW = static_cast<float>(rawW) / (shrinkFactor * weightFactor);
	int scaleH = static_cast<float>(rawH) / (shrinkFactor * weightFactor);

	if (gImageScaleWidth != scaleW || gImageScaleHeight != scaleH)
	{
		gImageScaleWidth = scaleW;
		gImageScaleHeight = scaleH;
		cout << "Scale = " << gImageScaleWidth << "x" << gImageScaleHeight << endl;

		UpdateGaussianBlurTexture();
	}
}
void BlurFactory(int phase, GLuint inputTex, GLuint outputTex)
{
	if (!gFbo)
		glGenFramebuffers(1, &gFbo);

	glBindFramebuffer(GL_FRAMEBUFFER, gFbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTex, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, gImageScaleWidth, gImageScaleHeight);

	glUseProgram(gGaussianBlurShader);
	glUniform1i(glGetUniformLocation(gGaussianBlurShader, "phase"), phase);
	glUniform1i(glGetUniformLocation(gGaussianBlurShader, "radius"), BlurLevel);
	glUniform1f(glGetUniformLocation(gGaussianBlurShader, "width"), static_cast<float>(gImageScaleWidth));
	glUniform1f(glGetUniformLocation(gGaussianBlurShader, "height"), static_cast<float>(gImageScaleHeight));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, inputTex);
	glUniform1i(glGetUniformLocation(gGaussianBlurShader, "tex"), 0);

	glBindVertexArray(gTriangleVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void DrawScene(GLFWwindow *window)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Properties");
	ImGui::SliderInt("BlurLevel", &BlurLevel, minBlurLevel, maxBlurLevel, "%.3f");
	ImGui::End();

	double firstTime = glfwGetTime();

	CalculateScaleWH(gImageWidth, gImageHeight);

	int phase = 0;
	GLuint srcTex = gTex;
	if (BlurLevel > 0)
	{
		// ÏÂ²ÉÑù
		BlurFactory(0, gTex, gDownSampleTex);
		// H
		BlurFactory(1, gDownSampleTex, gGaussianBlurHTex);
		// V
		BlurFactory(2, gGaussianBlurHTex, gGaussianBlurVTex);
		// output
		srcTex = gGaussianBlurVTex;
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(gTriangleVAO);
	glViewport(0, 0, gWindowWidth, gWindowHeight);
	glUseProgram(gShowShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, srcTex);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(window);

	double secondTime = glfwGetTime();
	gTime = secondTime - firstTime;
}

void LoadImage()
{
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load("../images/800x200.png", &gImageWidth, &gImageHeight, &gImageChannels, 0);
	GLint format = GL_RGBA;
	if (data)
	{
		if (gImageChannels == 3)
			format = GL_RGB;

		gTex = Create2DTexture(format, gImageWidth, gImageHeight, data);
		stbi_image_free(data);
	}
	else
		cout << "Failed to load texture" << endl;
}