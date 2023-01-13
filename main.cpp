#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "lib.h"

int WindowWidth, WindowHeight;

float ViewportX = 0.0f;
float ViewportY = 0.0f;

char *FileContents;

void RenderText(const char *text, int X, int Y, int FontHeight, array<float> *Vertices);
void RenderCharacter(char CharAscii, int CharX, int CharY, int CharWidth, int CharHeight, array<float> *Vertices);

void OnWindowResized(GLFWwindow *Window, int Width, int Height)
{
	glViewport(0, 0, Width, Height);
	WindowWidth = Width;
	WindowHeight = Height;
}

void OnKeyEvent(GLFWwindow *Window, int Key, int Scancode, int Action, int Mods)
{
	// press -> repeat ... -> release
//	if (Key == GLFW_KEY_Q)
//	{
//		if (Action == GLFW_PRESS)
//		{
//			printf("GLFW_PRESS\n");
//		}
//		else if (Action == GLFW_RELEASE)
//		{
//			printf("GLFW_RELEASE\n");
//		}
//		else if (Action == GLFW_REPEAT)
//		{
//			printf("GLFW_REPEAT\n");
//		}
//		else
//		{
//			assert(false);
//		}
//	}
	if (Key == GLFW_KEY_DOWN && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
		ViewportY += 0.1f; // do we want these in pixels?
	}
	if (Key == GLFW_KEY_UP && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
		ViewportY -= 0.1f;
	}
	if (Key == GLFW_KEY_LEFT && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
		ViewportX -= 0.1f;
	}
	if (Key == GLFW_KEY_RIGHT && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
		ViewportX += 0.1f;
	}
}

void OnFileDrop(GLFWwindow *Window, int Count, const char **Paths)
{
	if (FileContents)
	{
		free(FileContents);
	}

	assert(Count > 0);
	if (!ReadTextFile(Paths[0], &FileContents))
	{
		fprintf(stderr, "error: failed to read a dropped file: %s\n", Paths[0]);
		return;
	}

	ViewportX = 0.0f;
	ViewportY = 0.0f;
}

int main()
{
	if (glfwInit() == GLFW_FALSE)
	{
		printf("error: glfwInit()\n");
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow *window = glfwCreateWindow(800, 600, "My Window", NULL, NULL);
	if(window == NULL)
	{
		printf("error: glfwCreateWindow()\n");
		return 1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);
	gladLoadGL();

	// Dimensions given by glfwGetWindowSize() might not be in pixels
	glfwGetFramebufferSize(window, &WindowWidth, &WindowHeight);
	printf("window width: %d, window height: %d\n", WindowWidth, WindowHeight);

	glfwSetFramebufferSizeCallback(window, OnWindowResized);
	glfwSetKeyCallback(window, OnKeyEvent);
	glfwSetDropCallback(window, OnFileDrop);

//	glViewport(0, 0, WindowWidth, WindowHeight); // This happens by default

//	// RGB 2x2
//	unsigned char ImageData[] = {
//		0xff, 0xff, 0xff,
//		0x00, 0x00, 0x00,
//		0x00, 0x00, // row must be a multiple of 4 (?)
//
//		0x00, 0x00, 0x00,
//		0xff, 0xff, 0xff,
//		0x00, 0x00, // row must be a multiple of 4 (?)
//	};
//	int ImageWidth = 2;
//	int ImageHeight = 2;

	int ImageWidth, ImageHeight, ImageNChannels;
//	const char *ImageFile = "../charmap-oldschool_white.png";
	const char *ImageFile = "../charmap-oldschool_black.png";
	stbi_set_flip_vertically_on_load(1);
	unsigned char *ImageData = stbi_load(ImageFile, &ImageWidth, &ImageHeight, &ImageNChannels, 0);
	if(!ImageData)
	{
		fprintf(stderr, "error: stbi_load()\n");
		return 1;
	}
	else
	{
		printf("width: %d, height: %d, num channels: %d\n", ImageWidth, ImageHeight, ImageNChannels);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLuint myTexture;
	glGenTextures(1, &myTexture);
	glBindTexture(GL_TEXTURE_2D, myTexture);

	// What to do when texture coordinates are outside of the texture:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// What to do when the texture is minified/magnified:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ImageWidth, ImageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, ImageData);
//	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(ImageData);

	int success;
	char info_log[512];

	const char *vertexShaderText =
		"#version 330 core\n"
		"layout (location = 0) in vec2 pos;\n"
		"layout (location = 1) in vec2 texIn;\n"
		"uniform float viewportX;\n"
		"uniform float viewportY;\n"
		"out vec2 tex;\n"
		"void main() {\n"
		"gl_Position = vec4(pos.x - viewportX, pos.y + viewportY, 0.0, 1.0);\n"
		"tex = texIn;\n"
		"}\n";
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderText, NULL);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, sizeof(info_log), NULL, info_log);
		printf("vertex shader error: %s\n", info_log);
	}

	const char *fragmentShaderText =
		"#version 330 core\n"
		"in vec2 tex;\n"
		"out vec4 outColor;\n"
		"uniform sampler2D sampler;\n"
		"void main() {\n"
		"outColor = texture(sampler, tex);\n"
		"outColor.a = 1.0 - outColor.r;\n"
//		"outColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n";
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderText, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, sizeof(info_log), NULL, info_log);
		printf("fragment shader error: %s\n", info_log);
	}

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glUseProgram(shaderProgram);
//	glUniform1i(glGetUniformLocation(shaderProgram, "sampler"), 0);

	array<float> Vertices;
	ArrayInit(&Vertices);

	int FrameCount = 0;
	double PreviousTime;

	while(!glfwWindowShouldClose(window))
	{
		if (FrameCount == 0)
		{
			PreviousTime = glfwGetTime();
			FrameCount += 1;
		}
		else
		{
			double CurrentTime = glfwGetTime();
			double TimeElapsed = CurrentTime - PreviousTime;
			PreviousTime = CurrentTime;
			printf("elapsed: %.3fs\n", TimeElapsed);
		}
		glUniform1f(glGetUniformLocation(shaderProgram, "viewportX"), ViewportX);
		glUniform1f(glGetUniformLocation(shaderProgram, "viewportY"), ViewportY);

		RenderText(FileContents, 10, 10, 14, &Vertices);
	
		GLuint VAO; // Vertex Array Object
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
	
		GLuint VBO; // Vertex Buffer Object
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, Vertices.Count * sizeof(float), Vertices.Data, GL_STATIC_DRAW);
	//	GLint bufferSize;
	//	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
	//	printf("buffer size: %d\n", bufferSize);
	//	printf("sizeof(vertices): %ld\n", sizeof(vertices));
	
		// position
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
		glEnableVertexAttribArray(0);
	
		// texture coordinates
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
//		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, Vertices.Count / 4);
		glfwSwapBuffers(window);
		glfwPollEvents();

		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);

		Vertices.Count = 0;
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void RenderText(const char *Text, int X, int Y, int FontHeight, array<float> *Vertices)
{
	int CharHeight = FontHeight;
	int CharWidth = CharHeight * 5.0f / 7.0f; //@ ?

	int CharX = X;
	int CharY = Y;
	for (int i = 0; Text && Text[i]; ++i)
	{
		if (Text[i] == '\n')
		{
			CharY += FontHeight + 5;
			CharX = X;
			continue;
		}
		RenderCharacter(Text[i], CharX, CharY, CharWidth, CharHeight, Vertices);
		CharX += CharWidth + 5;
	}
}

void RenderCharacter(char CharAscii, int CharX, int CharY, int CharWidth, int CharHeight, array<float> *Vertices)
{
	int CharsInRow = 18;
	int FontImageWidth = 128;
	int FontImageHeight = 64;
	int OrigCharWidth = 5;
	int OrigCharHeight = 7;
	int OrigCharWidthAndSome = OrigCharWidth + 2;
	int OrigCharHeightAndSome = OrigCharHeight + 2;

	int Row = (CharAscii - 32) / CharsInRow;
	int Column = (CharAscii - 32) % CharsInRow;

	float PixelWidthTex = 1.0f / FontImageWidth;
	float PixelHeightTex = 1.0f / FontImageHeight;
	float x1 = PixelWidthTex * (1 + Column * OrigCharWidthAndSome);
	float x2 = PixelWidthTex * ((1 + Column * OrigCharWidthAndSome) + OrigCharWidth);
	float y1 = PixelHeightTex * ((64 - 1) - Row * OrigCharHeightAndSome);
	float y2 = PixelHeightTex * ((64 - 1) - (Row * OrigCharHeightAndSome + OrigCharHeight));
//	printf("x1: %f, x2: %f, y1: %f, y2: %f\n", x1, x2, y1, y2);

	float Width = CharWidth * 2.0f / WindowWidth;
	float Height = CharHeight * 2.0f / WindowHeight;
	float X = 2.0f / WindowWidth * CharX - 1;
	float Y = 1 - 2.0f / WindowHeight * CharY;
//	printf("x: %f, y: %f\n", X, Y);

	const float vertices[] = {
		/* upper-left */     X, Y,                  x1, y1,
		/* upper-right */    X + Width, Y,          x2, y1,
		/* lower-right */    X + Width, Y - Height, x2, y2,

		/* lower-right */    X + Width, Y - Height, x2, y2,
		/* lower-left */     X, Y - Height,         x1, y2,
		/* upper-left */     X, Y,                  x1, y1
	};

	for (int i = 0; i < COUNT(vertices); ++i)
	{
		ArrayAdd(Vertices, vertices[i]);
	}
}
