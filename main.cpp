#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "lib.h"

struct color
{
	float R, G, B;
};

struct textureCoordinates
{
	float X1, X2, Y1, Y2;
};

struct editor
{
	struct
	{
		const int Width, Height;
		const int CharsInRow;
		const int CharWidth, CharHeight;
		const int HorizontalSpacing, VerticalSpacing;
		const int LeftMargin, TopMargin;
	} FontImage;

	int WindowWidth, WindowHeight;
	int ViewportX, ViewportY;
	int CharWidth, CharHeight;
	int CharSpacing, LineSpacing;

	int CursorPos;

	color TextFGColor;
	color TextBGColor;
	color CursorColor;

	char *FileContents;
};

editor Editor = {
	.FontImage = {
		.Width = 128,
		.Height = 64,
		.CharsInRow = 18,
		.CharWidth = 5,
		.CharHeight = 7,
		.HorizontalSpacing = 2,
		.VerticalSpacing = 2,
		.LeftMargin = 1,
		.TopMargin = 1
	},
	.WindowWidth = 0,
	.WindowHeight = 0,
	.ViewportX = 0,
	.ViewportY = 0,
	.CharWidth = 2 * Editor.FontImage.CharWidth,
	.CharHeight = 2 * Editor.FontImage.CharHeight,
	.CharSpacing = 3,
	.LineSpacing = 10,
//	.CharWidth = Editor.FontImage.CharWidth,
//	.CharHeight = Editor.FontImage.CharHeight,

	.CursorPos = 0,

	.TextFGColor = {1.0f, 1.0f, 1.0f},
	.TextBGColor = {0.0f, 0.0f, 0.0f},
	.CursorColor = {0.0f, 1.0f, 0.0f},

	.FileContents = NULL
};

GLuint CreateShader(const char *VertexShaderFile, const char *FragmentShaderFile);
//void RenderText(const char *text, int X, int Y, int FontHeight, array<float> *Vertices);
//void RenderCharacter(char CharAscii, int CharX, int CharY, int CharWidth, int CharHeight, array<float> *Vertices);
void CreateTextVertices
(const char *Text, array<float> *Vertices, array<unsigned int> *Indices, textureCoordinates Chars[], editor *Editor);

void PossiblyUpdateViewportPosition(editor *Editor);

void OnWindowResized(GLFWwindow *Window, int Width, int Height)
{
	glViewport(0, 0, Width, Height);
	Editor.WindowWidth = Width;
	Editor.WindowHeight = Height;
}

void OnKeyEvent(GLFWwindow *Window, int Key, int Scancode, int Action, int Mods)
{
	if(!Editor.FileContents)
	{
		return;
	}
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

	if (Key == GLFW_KEY_UP && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
		int i = Editor.CursorPos;
		while(i > 0)
		{
			--i;
			if(Editor.FileContents[i] == '\n')
			{
				i += 1;
				break;
			}
		}
		int Offset = Editor.CursorPos - i;
		int Count = 2;
		while(Editor.CursorPos > 0)
		{
			--Editor.CursorPos;
			if(Editor.FileContents[Editor.CursorPos] == '\n')
			{
				Count -= 1;
				printf("newline count: %d\n", Count);
				if(Count == 0)
				{
					Editor.CursorPos += 1;
					break;
				}
			}
		}
		while(Offset > 0)
		{
			assert(Editor.FileContents[Editor.CursorPos]);
			if(Editor.FileContents[Editor.CursorPos] == '\n')
			{
				break;
			}
			Offset -= 1;
			Editor.CursorPos += 1;
		}

		PossiblyUpdateViewportPosition(&Editor);
	}
	if (Key == GLFW_KEY_DOWN && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
		int i = Editor.CursorPos;
		while(i > 0)
		{
			--i;
			if(Editor.FileContents[i] == '\n')
			{
				i += 1;
				break;
			}
		}
		int Offset = Editor.CursorPos - i;

		while(Editor.FileContents[Editor.CursorPos])
		{
			if(Editor.FileContents[Editor.CursorPos] == '\n')
			{
				Editor.CursorPos += 1;
				break;
			}
			++Editor.CursorPos;
		}

		while(Offset > 0)
		{
			if(!Editor.FileContents[Editor.CursorPos] || Editor.FileContents[Editor.CursorPos] == '\n')
			{
				break;
			}
			Offset -= 1;
			Editor.CursorPos += 1;
		}

		PossiblyUpdateViewportPosition(&Editor);
	}
	if (Key == GLFW_KEY_LEFT && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
		Editor.CursorPos -= Editor.CursorPos != 0;
		printf("cursor position: %d\n", Editor.CursorPos);
		PossiblyUpdateViewportPosition(&Editor);
	}
	if (Key == GLFW_KEY_RIGHT && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
		Editor.CursorPos += Editor.FileContents[Editor.CursorPos] != '\0';
		printf("cursor position: %d\n", Editor.CursorPos);
		PossiblyUpdateViewportPosition(&Editor);
	}
}

void OnFileDrop(GLFWwindow *Window, int Count, const char **Paths)
{
	if(Editor.FileContents)
	{
		free(Editor.FileContents);
	}

	if(!ReadTextFile(Paths[0], &Editor.FileContents))
	{
		fprintf(stderr, "error: failed to read a dropped file: %s\n", Paths[0]);
		return;
	}

	Editor.ViewportX = 0;
	Editor.ViewportY = 0;

	Editor.CursorPos = 0;
//	Editor.CursorPos = strlen(Editor.FileContents) - 1;
}

int main()
{
	if(glfwInit() == GLFW_FALSE)
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
//	glfwSwapInterval(0); // VSync off
	gladLoadGL();

	// Dimensions given by glfwGetWindowSize() might not be in pixels
	glfwGetFramebufferSize(window, &Editor.WindowWidth, &Editor.WindowHeight);
	printf("window width: %d, window height: %d\n", Editor.WindowWidth, Editor.WindowHeight);

	glfwSetFramebufferSizeCallback(window, OnWindowResized);
	glfwSetKeyCallback(window, OnKeyEvent);
	glfwSetDropCallback(window, OnFileDrop);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

	const int NumChars = 127 - ' ';
	textureCoordinates Chars[NumChars];
	{
		float PixelWidth = 1.0f / Editor.FontImage.Width;
		float PixelHeight = 1.0f / Editor.FontImage.Height;
		for(char Char = ' '; Char < 127; ++Char)
		{
			int Index = Char - ' ';
			int Row = Index / Editor.FontImage.CharsInRow;
			int Column = Index % Editor.FontImage.CharsInRow;
			int CharWidthAndSome = Editor.FontImage.CharWidth + Editor.FontImage.HorizontalSpacing;
			int CharHeightAndSome = Editor.FontImage.CharHeight + Editor.FontImage.VerticalSpacing;

			Chars[Index].X1 = (Editor.FontImage.LeftMargin + Column * CharWidthAndSome) * PixelWidth;
			Chars[Index].X2 = Chars[Index].X1 + Editor.FontImage.CharWidth * PixelWidth;
			Chars[Index].Y1 = ((Editor.FontImage.Height - Editor.FontImage.TopMargin) - Row * CharHeightAndSome) * PixelHeight;
			Chars[Index].Y2 = Chars[Index].Y1 - Editor.FontImage.CharHeight * PixelHeight;
		}
	}

	GLuint TextShader = CreateShader("../text-vs", "../text-fs");
//	GLuint CursorShader = CreateShader("../cursor-vs", "../cursor-fs");

	array<float> TextVertices;
	ArrayInit(&TextVertices);
	array<unsigned int> TextIndices;
	ArrayInit(&TextIndices);
	
	GLuint TextVAO; // Vertex Array Object
	glGenVertexArrays(1, &TextVAO);
	glBindVertexArray(TextVAO);

	GLuint TextVBO; // Vertex Buffer Object
	glGenBuffers(1, &TextVBO);
	glBindBuffer(GL_ARRAY_BUFFER, TextVBO);

	GLuint TextEBO; // Element Buffer Object
	glGenBuffers(1, &TextEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TextEBO);
	
	// position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *) 0);
	glEnableVertexAttribArray(0);

	// texture coordinates
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *) (2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// foreground color
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *) (4 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// background color
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *) (7 * sizeof(float)));
	glEnableVertexAttribArray(3);

////	int CursorWidth =;
////	int CursorHeight =;
//	float CursorVertices[] = {
//		/* upper-left*/  -0.5f, +0.5f,
//		/* upper-right*/ +0.5f, +0.5f,
//		/* lower-right*/ +0.5f, -0.5f,
//
//		/* lower-right*/ +0.5f, -0.5f,
//		/* lower-left*/  -0.5f, -0.5f,
//		/* upper-left*/  -0.5f, +0.5f,
//	};
//
//	GLuint CursorVAO; // Vertex Array Object for cursor
//	glGenVertexArrays(1, &CursorVAO);
//	glBindVertexArray(CursorVAO);
//
//	GLuint CursorVBO; // Vertex Buffer Object
//	glGenBuffers(1, &CursorVBO);
//	glBindBuffer(GL_ARRAY_BUFFER, CursorVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(CursorVertices), CursorVertices, GL_STATIC_DRAW);
//	
//	// position
//	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *) 0);
//	glEnableVertexAttribArray(0);

//	array<float> CursorVertices;
//	ArrayInit(&CursorVertices);

//	if(!ReadTextFile("/home/eero/all/text-editor/main.cpp", &Editor.FileContents))
//	{
//		return 0;
//	}

//	array<double> Timings;
//	ArrayInit(&Timings);

	int FrameCount = 0;
	double PreviousTime;

	while(!glfwWindowShouldClose(window))
	{
		if(FrameCount == 0)
		{
			PreviousTime = glfwGetTime();
		}
		else
		{
			double CurrentTime = glfwGetTime();
			double TimeElapsed = CurrentTime - PreviousTime;
			PreviousTime = CurrentTime;
//			printf("elapsed: %.3f s\n", TimeElapsed);
//			printf("elapsed: %f us\n", TimeElapsed * 1000000.0);

//			if(FrameCount == 1000)
//			{
//				double Sum = 0;
//				for(int i = 0; i < Timings.Count; ++i)
//				{
//					Sum += Timings.Data[i];
//				}
//				printf("average frame timing: %f us\n", Sum / Timings.Count * 1000000.0);
//				glfwSetWindowShouldClose(window, 1);
//			}
//			else
//			{
//				ArrayAdd(&Timings, TimeElapsed);
//			}
		}
		FrameCount += 1;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

//		glUseProgram(CursorShader);
//		glBindVertexArray(CursorVAO);
//		glDrawArrays(GL_TRIANGLES, 0, COUNT(CursorVertices));

		glUseProgram(TextShader);
		glUniform1f(glGetUniformLocation(TextShader, "WindowWidth"), Editor.WindowWidth);
		glUniform1f(glGetUniformLocation(TextShader, "WindowHeight"), Editor.WindowHeight);

		if(Editor.FileContents)
		{
			CreateTextVertices(Editor.FileContents, &TextVertices, &TextIndices, Chars, &Editor);
		}

		glBindVertexArray(TextVAO);
		glBindBuffer(GL_ARRAY_BUFFER, TextVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TextEBO);
//		glBufferData(GL_ARRAY_BUFFER, TextVertices.Count * sizeof(float), TextVertices.Data, GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, TextVertices.Count * sizeof(float), TextVertices.Data, GL_STREAM_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, TextIndices.Count * sizeof(unsigned int), TextIndices.Data, GL_STREAM_DRAW);
		glDrawElements(GL_TRIANGLES, TextIndices.Count, GL_UNSIGNED_INT, 0);
//		glDrawArrays(GL_TRIANGLES, 0, TextVertices.Count / 10);

		glfwSwapBuffers(window);

		glfwPollEvents();

		TextVertices.Count = 0;
		TextIndices.Count = 0;
	}

//	glDeleteVertexArrays(1, &CursorVAO);
//	glDeleteBuffers(1, &CursorVBO);
	glDeleteVertexArrays(1, &TextVAO);
	glDeleteBuffers(1, &TextVBO);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void CreateTextVertices
(const char *Text, array<float> *Vertices, array<unsigned int> *Indices, textureCoordinates Chars[], editor *Editor)
{
	int XAdvance = Editor->CharWidth + Editor->CharSpacing;
	int YAdvance = Editor->CharHeight + Editor->LineSpacing;
	int OriginalX = -Editor->ViewportX;
	int OriginalY = -Editor->ViewportY;
	int X = OriginalX;
	int Y = OriginalY;

	for(int i = 0; Text[i]; ++i)
	{
		if(Text[i] == '\r')
		{
			continue;
		}

		if(Y >= 0 && Y < Editor->WindowHeight && X >= 0 && X < Editor->WindowWidth)
		{
			char Char = Text[i];
			if(Char == '\t')
			{
				Char = ' ';
			}
			if(Char == '\n')
			{
				Char = ' ';
			}

			float X1 = X;                      // upper left
			float Y1 = Y;                      // upper left
			float X2 = X + Editor->CharWidth;  // lower right
			float Y2 = Y + Editor->CharHeight; // lower right

			float FGR = Editor->TextFGColor.R;
			float FGG = Editor->TextFGColor.G;
			float FGB = Editor->TextFGColor.B;

			float BGR, BGG, BGB;
			if(i == Editor->CursorPos)
			{
				BGR = Editor->CursorColor.R;
				BGG = Editor->CursorColor.G;
				BGB = Editor->CursorColor.B;
			}
			else
			{
				BGR = Editor->TextBGColor.R;
				BGG = Editor->TextBGColor.G;
				BGB = Editor->TextBGColor.B;
			}

			int Index = Char - ' ';

//			float QuadVertices[] = {
//				/*top-left*/      X1, Y1, Chars[Index].X1, Chars[Index].Y1, FGR, FGG, FGB, BGR, BGG, BGB,
//				/*bottom-left*/   X1, Y2, Chars[Index].X1, Chars[Index].Y2, FGR, FGG, FGB, BGR, BGG, BGB,
//				/*bottom-right*/  X2, Y2, Chars[Index].X2, Chars[Index].Y2, FGR, FGG, FGB, BGR, BGG, BGB,
//				/*bottom-right*/  X2, Y2, Chars[Index].X2, Chars[Index].Y2, FGR, FGG, FGB, BGR, BGG, BGB,
//				/*top-right*/     X2, Y1, Chars[Index].X2, Chars[Index].Y1, FGR, FGG, FGB, BGR, BGG, BGB,
//				/*top-left*/      X1, Y1, Chars[Index].X1, Chars[Index].Y1, FGR, FGG, FGB, BGR, BGG, BGB,
//			};

			float QuadVertices[] = {
				X1, Y1, Chars[Index].X1, Chars[Index].Y1, FGR, FGG, FGB, BGR, BGG, BGB, /*top-left*/
				X1, Y2, Chars[Index].X1, Chars[Index].Y2, FGR, FGG, FGB, BGR, BGG, BGB, /*bottom-left*/
				X2, Y2, Chars[Index].X2, Chars[Index].Y2, FGR, FGG, FGB, BGR, BGG, BGB, /*bottom-right*/
				X2, Y1, Chars[Index].X2, Chars[Index].Y1, FGR, FGG, FGB, BGR, BGG, BGB, /*top-right*/
			};

			for(int i = 0; i < COUNT(QuadVertices); ++i)
			{
				ArrayAdd(Vertices, QuadVertices[i]);
			}

			//@ Vertices->Count is not the number of vertices, its the number of floats in the array, really misleading
			int OffsetNewVertices = Vertices->Count / 10 - 4;
			unsigned int QuadIndices[] = {
				0, 1, 2,
				2, 3, 0,
			};
			for(int i = 0; i < COUNT(QuadIndices); ++i)
			{
				ArrayAdd(Indices, QuadIndices[i] + OffsetNewVertices);
			}
		}
			
		if(Text[i] == '\n')
		{
			Y += YAdvance;
			X = OriginalX;
		}
		else
		{
			X += XAdvance;
		}
	}
}

//void RenderText(const char *Text, int X, int Y, int FontHeight, array<float> *Vertices)
//{
//	int CharHeight = FontHeight;
////	int CharWidth = CharHeight * 5.0f / 7.0f; //@ ?
//	int CharWidth = CAST_ROUND(CharHeight * 5.0f / 7.0f); //@ ?
//
//	int CharX = X;
//	int CharY = Y;
//	for (int i = 0; Text && Text[i]; ++i)
//	{
//		if (Text[i] == '\n')
//		{
//			CharY += FontHeight + 5;
//			CharX = X;
//			continue;
//		}
//		RenderCharacter(Text[i], CharX, CharY, CharWidth, CharHeight, Vertices);
//		CharX += CharWidth + 5;
//	}
//}
//
//void RenderCharacter(char CharAscii, int CharX, int CharY, int CharWidth, int CharHeight, array<float> *Vertices)
//{
//	int CharsInRow = 18;
//	int FontImageWidth = 128;
//	int FontImageHeight = 64;
//	int OrigCharWidth = 5;
//	int OrigCharHeight = 7;
//	int OrigCharWidthAndSome = OrigCharWidth + 2;
//	int OrigCharHeightAndSome = OrigCharHeight + 2;
//
//	int Row = (CharAscii - 32) / CharsInRow;
//	int Column = (CharAscii - 32) % CharsInRow;
//
//	float PixelWidthTex = 1.0f / FontImageWidth;
//	float PixelHeightTex = 1.0f / FontImageHeight;
//	float x1 = PixelWidthTex * (1 + Column * OrigCharWidthAndSome);
//	float x2 = PixelWidthTex * ((1 + Column * OrigCharWidthAndSome) + OrigCharWidth);
//	float y1 = PixelHeightTex * ((64 - 1) - Row * OrigCharHeightAndSome);
//	float y2 = PixelHeightTex * ((64 - 1) - (Row * OrigCharHeightAndSome + OrigCharHeight));
////	printf("x1: %f, x2: %f, y1: %f, y2: %f\n", x1, x2, y1, y2);
//
//	float PixelWidthGL = 2.0f / WindowWidth;
//	float PixelHeightGL = 2.0f / WindowHeight;
//	float Width = CharWidth * PixelWidthGL;
//	float Height = CharHeight * PixelHeightGL;
//	float X = PixelWidthGL * CharX - 1;
//	float Y = 1 - PixelHeightGL * CharY;
////	printf("x: %f, y: %f\n", X, Y);
//
//	const float vertices[] = {
//		/* upper-left */     X, Y,                  x1, y1,
//		/* upper-right */    X + Width, Y,          x2, y1,
//		/* lower-right */    X + Width, Y - Height, x2, y2,
//
//		/* lower-right */    X + Width, Y - Height, x2, y2,
//		/* lower-left */     X, Y - Height,         x1, y2,
//		/* upper-left */     X, Y,                  x1, y1
//	};
//
//	for(int i = 0; i < COUNT(vertices); ++i){
//		ArrayAdd(Vertices, vertices[i]);
//	}
//}

GLuint CreateShader(const char *VertexShaderFile, const char *FragmentShaderFile)
{
	int success;
	char info_log[512];

	char *vertexShaderText;
	if(!ReadTextFile(VertexShaderFile, &vertexShaderText)){
		fprintf(stderr, "Error: couldnt read shader file: %s\n", VertexShaderFile);
		return 0; //@
	}
		
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderText, NULL);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, sizeof(info_log), NULL, info_log);
		printf("Shader error: %s: %s\n", VertexShaderFile, info_log);
	}

	char *fragmentShaderText;
	if(!ReadTextFile(FragmentShaderFile, &fragmentShaderText)){
		fprintf(stderr, "Error: couldnt read shader file: %s\n", fragmentShaderText);
		return 0; //@
	}
		
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderText, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, sizeof(info_log), NULL, info_log);
		printf("Shader error: %s: %s\n", FragmentShaderFile, info_log);
	}

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	//@ delete shaders?
	free(vertexShaderText);
	free(fragmentShaderText);

	return shaderProgram;
}

// If the cursor has moved outside the viewport, adjust the viewport's offsets, so that cursor becomes visible again
void PossiblyUpdateViewportPosition(editor *Editor)
{
	int i = Editor->CursorPos;
	while(i > 0)
	{
		--i;
		if(Editor->FileContents[i] == '\n')
		{
			i += 1;
			break;
		}
	}
	int CharCount = Editor->CursorPos - i; // number of chars before cursor

// Figure out on which line the cursor is at
	int LineCount = 0; // number of lines before cursor
	for(int i = Editor->CursorPos - 1; i >= 0; --i)
	{
		if(Editor->FileContents[i] == '\n')
		{
			++LineCount;
		}
	}
//	printf("Cursor is at %dth line\n", LineCount + 1);

	// Calculate the cursor's position in text-buffer-coordinates
	int CursorX1 = CharCount * (Editor->CharWidth + Editor->CharSpacing);
	int CursorY1 = LineCount * (Editor->CharHeight + Editor->LineSpacing);
	int CursorX2 = CursorX1 + Editor->CharWidth;
	int CursorY2 = CursorY1 + Editor->CharHeight;

	if(CursorY2 > Editor->ViewportY + Editor->WindowHeight)
	{
		Editor->ViewportY = CursorY2 - Editor->WindowHeight;
	}
	if(CursorY1 < Editor->ViewportY)
	{
		Editor->ViewportY = CursorY1;
	}
	if(CursorX2 > Editor->ViewportX + Editor->WindowWidth)
	{
		Editor->ViewportX = CursorX2 - Editor->WindowWidth;
	}
	if(CursorX1 < Editor->ViewportX)
	{
		Editor->ViewportX = CursorX1;
	}
}