/*
	* ~/textdb (over million lines) is very slow. Why?
*/

#include <stdio.h>
//#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "lib.h"

const int MAX_FILE_PATH = 1000; //@ very random

struct color
{
	float R, G, B, A;
};

//struct quad
//{
//	float X, Y;
//	color Color;
//};

//struct charColor
//{
//	int Index;
//	color Color;
//	float Progress;
//};

struct textureCoordinates
{
	float X1, X2, Y1, Y2;
};

struct fontImage
{
	int Width, Height;
	int CharsInRow;
	int CharWidth, CharHeight;
	int HorizontalSpacing, VerticalSpacing;
	int LeftMargin, TopMargin;
};

// Could keep an index of the last character instead of storing an actual '\0'?
// Iterator can be equal to OneAfterLast even though there is no character at that location
struct textBuffer
{
	int Size;
	char *Data;
	int OneAfterLast; // also size of contents
	int Cursor;
};

struct editor
{
	fontImage FontImage;

	int WindowWidth, WindowHeight;
	int ViewportX, ViewportY;
	int CharWidth, CharHeight;
	int CharSpacing, LineSpacing;

	color TextColor;
	color CursorColor;
//	color CursorEffectColor;
//	float CursorEffectProgress;

	textBuffer TextBuffer;

//	array<quad> EffectQuads;
//	array<charColor> CharsWithEffect;

	char OpenFile[MAX_FILE_PATH];

	GLFWwindow *AppWindow;
} Editor;

void InitEditor(editor *Editor, GLFWwindow *AppWindow);

GLuint CreateShader(const char *VertexShaderFile, const char *FragmentShaderFile);
//void RenderText(const char *text, int X, int Y, int FontHeight, array<float> *Vertices);
//void RenderCharacter(char CharAscii, int CharX, int CharY, int CharWidth, int CharHeight, array<float> *Vertices);
//void CreateTextVertices
//(const char *Text, array<float> *Vertices, array<unsigned int> *Indices, textureCoordinates Chars[], editor *Editor);
void MakeTextVertices(array<float> *Vertices, textBuffer *Buffer, textureCoordinates Chars[], int NumChars, editor *Editor);
//void CreateTextVertices(array<float> *Vertices, const char *Text, textureCoordinates Chars[]);
void MakeCursorVertices(array<float> *CursorVertices, textBuffer *Buffer, editor *Editor);
void MakeQuad(array<float> *Vertices, int X, int Y, int Width, int Height, color Color);

//void SetCursor(textBuffer *Buffer, int At, editor *Editor);
//int GetCursorCharIndex(textBuffer *Buffer);
//int GetCursorLineIndex(textBuffer *Buffer);
//void InsertAtCursor(textBuffer *Buffer, char Char, editor *Editor);

void InitTextBuffer(textBuffer *Buffer);
char GetChar(textBuffer *Buffer, int At);
int GetStart(textBuffer *Buffer);
int GetEnd(textBuffer *Buffer);
bool IsStart(textBuffer *Buffer, int At);
bool IsEnd(textBuffer *Buffer, int At);
bool MoveForward(textBuffer *Buffer, int *Iter);
void MoveForwardFast(textBuffer *Buffer, int *Iter);
bool MoveBackward(textBuffer *Buffer, int *Iter);
bool MoveAtCharBackwards(textBuffer *Buffer, char Char, int *Iter);
//bool MoveAtCharForwards(textBuffer *Buffer, char Char, int *Iter);
int GetCursor(textBuffer *Buffer);
void SetCursor(textBuffer *Buffer, int At);
int GetCharsIntoLine(textBuffer *Buffer, int At);
int GetLinesIntoBuffer(textBuffer *Buffer, int At);
bool MoveToPrevLine(textBuffer *Buffer, int *Iter);
bool MoveToNextLine(textBuffer *Buffer, int *Iter);
//void CursorForward(textBuffer *Buffer); // replace: MoveForward(Buffer, *Iter)
//void CursorBackward(textBuffer *Buffer);
bool Insert(textBuffer *Buffer, char Char, int At);
bool Insert(textBuffer *Buffer, const char *Text, int At);
bool Delete(textBuffer *Buffer, int At);
bool Delete(textBuffer *Buffer, int At, int NumChars);

//void PossiblyUpdateViewportPosition(textBuffer *Buffer, editor *Editor);
void AdjustViewportIfNotVisible(editor *Editor, int CharIndex, int LineIndex);
float Lerp(float From, float To, float Progress);

void OnWindowResized(GLFWwindow *Window, int Width, int Height)
{
	glViewport(0, 0, Width, Height);
	Editor.WindowWidth = Width;
	Editor.WindowHeight = Height;
}

void OnCharEvent(GLFWwindow *Window, unsigned int Codepoint)
{
//	if(Codepoint >= ' ' && Codepoint <= '~')
	if( ' ' <= Codepoint && Codepoint <= '~')
	{
		// dont calculate positions relative to the window(viewport) because window might change
		// calculate relative to text-buffer
//		int CharIndex = GetCursorCharIndex(&Editor.TextBuffer);
//		int LineIndex = GetCursorLineIndex(&Editor.TextBuffer);
//		int X = CharIndex * (Editor.CharWidth + Editor.CharSpacing);
//		int Y = LineIndex * (Editor.CharHeight + Editor.LineSpacing);
////		ArrayAdd(&Editor.EffectQuads, {X, Y, CursorColor});

//		color CursorEffectColor;
//		CursorEffectColor.A = 1.0f;
//		CursorEffectColor.R = (rand() % 10) / 9.0f;
//		CursorEffectColor.G = (rand() % 10) / 9.0f;
//		CursorEffectColor.B = (rand() % 10) / 9.0f;
//		Editor.CursorEffectColor = CursorEffectColor;
//		Editor.CursorEffectProgress = 0.0f;

//		charColor CharColor;
//		CharColor.Index = Editor.TextBuffer.Cursor;
//		CharColor.Progress = 0.0f;
//		CharColor.Color = CursorEffectColor;
//		ArrayAdd(&Editor.CharsWithEffect, CharColor);

//		InsertAtCursor(&Editor.TextBuffer, (char)Codepoint, &Editor);
		textBuffer *Buffer = &Editor.TextBuffer;
		int Cursor = GetCursor(Buffer);
		if(Insert(Buffer, (char)Codepoint, Cursor))
		{
//			CursorForward(Buffer);
			MoveForward(Buffer, &Cursor);
			SetCursor(Buffer, Cursor);
			int NewCursor = GetCursor(Buffer);
			int NewCursorCharIndex = GetCharsIntoLine(Buffer, NewCursor);
			int NewCursorLineIndex = GetLinesIntoBuffer(Buffer, NewCursor);
			AdjustViewportIfNotVisible(&Editor, NewCursorCharIndex, NewCursorLineIndex);
		}
		else
		{
			printf("BUFFER IS FULL\n");
		}
	}
	else
	{
		printf("Codepoint %c(%d) not supported.\n", Codepoint, Codepoint);
	}
}

void OnKeyEvent(GLFWwindow *Window, int Key, int Scancode, int Action, int Mods)
{
	// press -> repeat ... -> release

	textBuffer *Buffer = &Editor.TextBuffer;

	// CTRL + S
	if(Key == GLFW_KEY_S && Mods == GLFW_MOD_CONTROL && Action == GLFW_PRESS)
	{
		printf("S + Ctrl\n");
		if(Editor.OpenFile[0] != '\0')
		{
			if(!WriteFile(Editor.OpenFile, (u8 *) Editor.TextBuffer.Data, Editor.TextBuffer.OneAfterLast))
			{
				printf("Error: Failed to write to %s\n", Editor.OpenFile);
			}
		}
		else
		{
			printf("NO FILE TO WRITE TO\n");
		}
		return;
	}

	if (Key == GLFW_KEY_UP && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
		int Iter = GetCursor(Buffer);
		int CharsIntoLine = GetCharsIntoLine(Buffer, Iter);
		if(MoveToPrevLine(Buffer, &Iter))
		{
			// If there is a previous line (we are not on the first line)
			while(CharsIntoLine > 0)
			{
				if(GetChar(Buffer, Iter) == '\n')
				{
					break;
				}
				--CharsIntoLine;
				++Iter;
			}
			SetCursor(Buffer, Iter);
			int NewCharsIntoLine = GetCharsIntoLine(Buffer, Iter);
			int NewLinesIntoBuffer = GetLinesIntoBuffer(Buffer, Iter);
			AdjustViewportIfNotVisible(&Editor, NewCharsIntoLine, NewLinesIntoBuffer);
		}
		else
		{
			printf("NO PREV LINE\n");
		}
	}
	if (Key == GLFW_KEY_DOWN && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
		int Cursor = GetCursor(Buffer);
		int CharsIntoLine = GetCharsIntoLine(Buffer, Cursor);
		if(MoveToNextLine(Buffer, &Cursor))
		{
			// If there is a previous line (we are not on the first line)
			while(CharsIntoLine > 0)
			{
				char Char = GetChar(Buffer, Cursor);
				if(Char == '\n' || Char == '\0') //@ '\0'?
				{
					break;
				}
				--CharsIntoLine;
				MoveForward(Buffer, &Cursor);
			}
			SetCursor(Buffer, Cursor);
			int NewCharsIntoLine = GetCharsIntoLine(Buffer, Cursor);
			int NewLinesIntoBuffer = GetLinesIntoBuffer(Buffer, Cursor);
			AdjustViewportIfNotVisible(&Editor, NewCharsIntoLine, NewLinesIntoBuffer);
		}
	}
	if (Key == GLFW_KEY_LEFT && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
//		int Cursor = GetCursor(Buffer);
//		if(!IsStart(Buffer, Cursor))
//		{
//			int NewCursor = Cursor-1;
//			SetCursor(Buffer, NewCursor);
//			int CharsIntoLine = GetCharsIntoLine(Buffer, NewCursor);
//			int LinesIntoBuffer = GetLinesIntoBuffer(Buffer, NewCursor);
//			AdjustViewportIfNotVisible(&Editor, CharsIntoLine, LinesIntoBuffer);
//		}
		int Cursor = GetCursor(Buffer);
		MoveBackward(Buffer, &Cursor);
		SetCursor(Buffer, Cursor);

		int CharsIntoLine = GetCharsIntoLine(Buffer, Cursor);
		int LinesIntoBuffer = GetLinesIntoBuffer(Buffer, Cursor);
		AdjustViewportIfNotVisible(&Editor, CharsIntoLine, LinesIntoBuffer);
	}
	if (Key == GLFW_KEY_RIGHT && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
//		int Cursor = GetCursor(Buffer);
//		if(!IsEnd(Buffer, Cursor))
//		{
//			int NewCursor = Cursor+1;
//			SetCursor(Buffer, NewCursor);
//			int CharsIntoLine = GetCharsIntoLine(Buffer, NewCursor);
//			int LinesIntoBuffer = GetLinesIntoBuffer(Buffer, NewCursor);
//			AdjustViewportIfNotVisible(&Editor, CharsIntoLine, LinesIntoBuffer);
//		}
		int Cursor = GetCursor(Buffer);
		if(MoveForward(Buffer, &Cursor))
		{
			int CharsIntoLine = GetCharsIntoLine(Buffer, Cursor);
			int LinesIntoBuffer = GetLinesIntoBuffer(Buffer, Cursor);
			AdjustViewportIfNotVisible(&Editor, CharsIntoLine, LinesIntoBuffer);
		}
		SetCursor(Buffer, Cursor);

	}

	if(Key == GLFW_KEY_ENTER && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
		int Cursor = GetCursor(Buffer);
		if(Insert(Buffer, '\n', Cursor))
		{
//			CursorForward(Buffer);
//			int NewCursor = GetCursor(Buffer);
//			int NewCursorCharIndex = GetCharsIntoLine(Buffer, NewCursor);
//			int NewCursorLineIndex = GetLinesIntoBuffer(Buffer, NewCursor);
			MoveForward(Buffer, &Cursor);
			SetCursor(Buffer, Cursor);
			int CursorCharIndex = GetCharsIntoLine(Buffer, Cursor);
			int CursorLineIndex = GetLinesIntoBuffer(Buffer, Cursor);
			AdjustViewportIfNotVisible(&Editor, CursorCharIndex, CursorLineIndex);
		}
		else
		{
			printf("BUFFER IS FULL\n");
		}
	}
	if(Key == GLFW_KEY_BACKSPACE && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
		// delete character before cursor
		int Iter = GetCursor(Buffer);
		if(!IsStart(Buffer, Iter))
		{
			MoveBackward(Buffer, &Iter);
			if(Delete(Buffer, Iter))
			{
				SetCursor(Buffer, Iter);
				int CharIndex = GetCharsIntoLine(Buffer, Iter);
				int LineIndex = GetLinesIntoBuffer(Buffer, Iter);
				AdjustViewportIfNotVisible(&Editor, CharIndex, LineIndex);
			}
			else
			{
				assert(false); // should never happen
			}
		}
	}
}

void OnFileDrop(GLFWwindow *Window, int Count, const char **Paths)
{
//	if(Editor.FileContents)
//	{
//		free(Editor.FileContents);
//	}

	char *Contents;
	if(!ReadTextFile(Paths[0], &Contents))
	{
		fprintf(stderr, "error: failed to read a dropped file: %s\n", Paths[0]);
		return;
	}
	printf("Read file \"%s\" successfully\n", Paths[0]);
	textBuffer *Buffer = &Editor.TextBuffer;
	int NumCharsInBuffer = Buffer->OneAfterLast;
	Delete(Buffer, GetStart(Buffer), NumCharsInBuffer);
	if(Insert(Buffer, Contents, GetStart(Buffer)))
	{
		printf("Loaded file successfully\n");
		strcpy(Editor.OpenFile, Paths[0]); //@ bounds
		glfwSetWindowTitle(Editor.AppWindow, Editor.OpenFile);
	}
	else
	{
		printf("File too large!\n");
	}
	SetCursor(Buffer, GetStart(Buffer));

	Editor.ViewportX = 0;
	Editor.ViewportY = 0;

	free(Contents);
}

int main()
{
//	srand(time(NULL));
//	printf("n: %f\n", (rand() % 10) / 9.0f);
//	printf("n: %f\n", (rand() % 10) / 9.0f);
//	printf("n: %f\n", (rand() % 10) / 9.0f);
//	return 0;
//	printf("%f\n", Lerp(9.0, 1.0, 0.0));
//	printf("%f\n", Lerp(9.0, 1.0, 0.5));
//	printf("%f\n", Lerp(9.0, 1.0, 0.75));
//	printf("%f\n", Lerp(9.0, 1.0, 1.0));
//	return 0;
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

	InitEditor(&Editor, window);

	// Dimensions given by glfwGetWindowSize() might not be in pixels
	glfwGetFramebufferSize(window, &Editor.WindowWidth, &Editor.WindowHeight);
	printf("window width: %d, window height: %d\n", Editor.WindowWidth, Editor.WindowHeight);

	glfwSetFramebufferSizeCallback(window, OnWindowResized);
	glfwSetKeyCallback(window, OnKeyEvent);
	glfwSetCharCallback(window, OnCharEvent);
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
	GLuint QuadShader = CreateShader("../quad-vs", "../quad-fs");

	array<float> TextVertices;
	ArrayInit(&TextVertices);
//	array<unsigned int> TextIndices;
//	ArrayInit(&TextIndices);
	
	GLuint TextVAO; // Vertex Array Object
	glGenVertexArrays(1, &TextVAO);
	glBindVertexArray(TextVAO);

	GLuint TextVBO; // Vertex Buffer Object
	glGenBuffers(1, &TextVBO);
	glBindBuffer(GL_ARRAY_BUFFER, TextVBO);

//	GLuint TextEBO; // Element Buffer Object
//	glGenBuffers(1, &TextEBO);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TextEBO);
	
	// position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *) 0);
	glEnableVertexAttribArray(0);

	// texture coordinates
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *) (2 * sizeof(float)));
	glEnableVertexAttribArray(1);

//	// foreground color
//	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *) (4 * sizeof(float)));
//	glEnableVertexAttribArray(2);
//
//	// background color
//	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *) (7 * sizeof(float)));
//	glEnableVertexAttribArray(3);

	// color
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *) (4 * sizeof(float)));
	glEnableVertexAttribArray(2);

	array<float> QuadVertices;
	ArrayInit(&QuadVertices);

	GLuint QuadVAO; // Vertex Array Object for cursor
	glGenVertexArrays(1, &QuadVAO);
	glBindVertexArray(QuadVAO);

	GLuint QuadVBO; // Vertex Buffer Object
	glGenBuffers(1, &QuadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
	
	// position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
	glEnableVertexAttribArray(0);
	// color
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (2 * sizeof(float)));
	glEnableVertexAttribArray(1);

//	array<double> Timings;
//	ArrayInit(&Timings);

	int FrameCount = 0;
	double PreviousTime;

	while(!glfwWindowShouldClose(window))
	{
//		printf("# chars with effect: %d\n", Editor.CharsWithEffect.Count);
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
////			printf("elapsed: %f us\n", TimeElapsed * 1000000.0);
//
////			if(FrameCount == 1000)
////			{
////				double Sum = 0;
////				for(int i = 0; i < Timings.Count; ++i)
////				{
////					Sum += Timings.Data[i];
////				}
////				printf("average frame timing: %f us\n", Sum / Timings.Count * 1000000.0);
////				glfwSetWindowShouldClose(window, 1);
////			}
////			else
////			{
////				ArrayAdd(&Timings, TimeElapsed);
////			}
		}
		FrameCount += 1;

		double Time1 = glfwGetTime();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		double Time2 = glfwGetTime();
//		printf("CLEAR: %f\n", Time2 - Time1);

		Time1 = glfwGetTime();

		glUseProgram(QuadShader);
		glUniform1f(glGetUniformLocation(QuadShader, "WindowWidth"), Editor.WindowWidth);
		glUniform1f(glGetUniformLocation(QuadShader, "WindowHeight"), Editor.WindowHeight);

		// Make the cursor
//		color CursorColor;
//		if(Editor.CursorEffectProgress < 1.0f)
//		{
//			CursorColor.A = Editor.CursorColor.A;
//			CursorColor.R = Lerp(Editor.CursorEffectColor.R, Editor.CursorColor.R, Editor.CursorEffectProgress);
//			CursorColor.G = Lerp(Editor.CursorEffectColor.G, Editor.CursorColor.G, Editor.CursorEffectProgress);
//			CursorColor.B = Lerp(Editor.CursorEffectColor.B, Editor.CursorColor.B, Editor.CursorEffectProgress);
//		}
//		else
//		{
//			CursorColor = Editor.CursorColor;
//		}
//		Editor.CursorEffectProgress += 0.01f;
//		if(Editor.CursorEffectProgress > 1.0f)
//		{
//			Editor.CursorEffectProgress = 1.0f;
//		}
		int CursorWidth = Editor.CharWidth;
		int CursorHeight = Editor.CharHeight;
		textBuffer *Buffer = &Editor.TextBuffer;
		int Cursor = GetCursor(Buffer);
		float X = GetCharsIntoLine(Buffer, Cursor) * (Editor.CharWidth + Editor.CharSpacing) - Editor.ViewportX;
		float Y = GetLinesIntoBuffer(Buffer, Cursor) * (Editor.CharHeight + Editor.LineSpacing) - Editor.ViewportY;
		MakeQuad(&QuadVertices, X, Y, CursorWidth, CursorHeight, Editor.CursorColor);

//		for(int i = 0; i < Editor.EffectQuads.Count; ++i)
//		{
//			MakeQuad(
//				&QuadVertices,
//				Editor.EffectQuads.Data[i].X - Editor.ViewportX,
//				Editor.EffectQuads.Data[i].Y - Editor.ViewportY,
//				Editor.CharWidth, Editor.CharHeight,
//				Editor.EffectQuads.Data[i].Color);
//			Editor.EffectQuads.Data[i].Color.A -= 0.01;
//			printf("effect quad %d alpha: %f\n", i, Editor.EffectQuads.Data[i].Color.A);
//			if(Editor.EffectQuads.Data[i].Color.A <= 0.0f)
//			{
//				ArrayRemove(&Editor.EffectQuads, i);
//				printf("removed an effect quad!\n");
//			}
//		}
		glBindVertexArray(QuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
		glBufferData(GL_ARRAY_BUFFER, QuadVertices.Count * sizeof(float), QuadVertices.Data, GL_STREAM_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, QuadVertices.Count / 6);

		Time2 = glfwGetTime();
//		printf("CURSOR: %f\n", Time2 - Time1);

		
		Time1 = glfwGetTime();

		glUseProgram(TextShader);
		glUniform1f(glGetUniformLocation(TextShader, "WindowWidth"), Editor.WindowWidth);
		glUniform1f(glGetUniformLocation(TextShader, "WindowHeight"), Editor.WindowHeight);

		double Before = glfwGetTime();
		MakeTextVertices(&TextVertices, &Editor.TextBuffer, Chars, NumChars, &Editor);
		double After = glfwGetTime();
//		printf("MakeTextVertices(): %f\n", After - Before);
		glBindVertexArray(TextVAO);
		glBindBuffer(GL_ARRAY_BUFFER, TextVBO);
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TextEBO);
//		glBufferData(GL_ARRAY_BUFFER, TextVertices.Count * sizeof(float), TextVertices.Data, GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, TextVertices.Count * sizeof(float), TextVertices.Data, GL_STREAM_DRAW);
//		glBufferData(GL_ELEMENT_ARRAY_BUFFER, TextIndices.Count * sizeof(unsigned int), TextIndices.Data, GL_STREAM_DRAW);
//		glDrawElements(GL_TRIANGLES, TextIndices.Count, GL_UNSIGNED_INT, 0);
		glDrawArrays(GL_TRIANGLES, 0, TextVertices.Count / 7);

		Time2 = glfwGetTime();
//		printf("TEXT: %f\n", Time2 - Time1);

		Time1 = glfwGetTime();

		glfwSwapBuffers(window);

		glfwPollEvents();

		Time2 = glfwGetTime();
//		printf("SWAP-POLL: %f\n", Time2 - Time1);
//		printf("\n");

		TextVertices.Count = 0;
		QuadVertices.Count = 0;
//		TextIndices.Count = 0;
	}

	glDeleteVertexArrays(1, &QuadVAO);
	glDeleteBuffers(1, &QuadVBO);
	glDeleteVertexArrays(1, &TextVAO);
	glDeleteBuffers(1, &TextVBO);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void MakeTextVertices(array<float> *Vertices, textBuffer *Buffer, textureCoordinates Chars[], int NumChars, editor *Editor)
{
	int XAdvance = Editor->CharWidth + Editor->CharSpacing;
	int YAdvance = Editor->CharHeight + Editor->LineSpacing;
	int OriginalX = -Editor->ViewportX; // X and Y are going to be in window-coordinates (not in buffer-coordinates)
	int OriginalY = -Editor->ViewportY;
	int X = OriginalX;
	int Y = OriginalY;

	int TotalCharCount, VisibleCharCount;
	TotalCharCount = 0;
	VisibleCharCount = 0;
	double TotalElapsedVisible = 0;

	for(int i = GetStart(Buffer); !IsEnd(Buffer, i); MoveForward(Buffer, &i))
//	for(int i = GetStart(Buffer); !IsEnd(Buffer, i); MoveForwardFast(Buffer, &i))
	{
		TotalCharCount += 1;
		unsigned char Char = GetChar(Buffer, i);
//		if(Char > '~')
//		{
//			printf("NON-ASCII CHARACTER\n");
//		}

		if(Y >= 0 && Y < Editor->WindowHeight && X >= 0 && X < Editor->WindowWidth
		   && ' ' <= Char && Char <= '~')
		{
			VisibleCharCount += 1;
			double T1 = glfwGetTime();

			float X1 = X;                      // upper left
			float Y1 = Y;                      // upper left
			float X2 = X + Editor->CharWidth;  // lower right
			float Y2 = Y + Editor->CharHeight; // lower right

			float TextR = Editor->TextColor.R;
			float TextG = Editor->TextColor.G;
			float TextB = Editor->TextColor.B;

			// if character should have an effect, use effect color
//			for(int j = 0; j < Editor->CharsWithEffect.Count; ++j)
//			{
//				if(Editor->CharsWithEffect.Data[j].Index == i)
//				{
//					charColor *p = &Editor->CharsWithEffect.Data[j];
//					TextR = Lerp(p->Color.R, Editor->TextColor.R, p->Progress);
//					TextG = Lerp(p->Color.G, Editor->TextColor.G, p->Progress);
//					TextB = Lerp(p->Color.B, Editor->TextColor.B, p->Progress);
//					p->Progress += 0.01f;
//					if(p->Progress >= 1.0f)
//					{
//						ArrayRemove(&Editor->CharsWithEffect, j);
//					}
//					break;
//				}
//			}

			int Index = Char - ' ';
	
			float QuadVertices[] = {
				X1, Y1, Chars[Index].X1, Chars[Index].Y1, TextR, TextG, TextB, /*top-left*/
				X1, Y2, Chars[Index].X1, Chars[Index].Y2, TextR, TextG, TextB, /*bottom-left*/
				X2, Y2, Chars[Index].X2, Chars[Index].Y2, TextR, TextG, TextB, /*bottom-right*/
				X2, Y2, Chars[Index].X2, Chars[Index].Y2, TextR, TextG, TextB, /*bottom-right*/
				X2, Y1, Chars[Index].X2, Chars[Index].Y1, TextR, TextG, TextB, /*top-right*/
				X1, Y1, Chars[Index].X1, Chars[Index].Y1, TextR, TextG, TextB, /*top-left*/
			};
	
			ArrayAdd(Vertices, QuadVertices, COUNT(QuadVertices));

			double T2 = glfwGetTime();
			TotalElapsedVisible += T2 - T1;
		}

		if(Char == '\n')
		{
			Y += YAdvance;
			X = OriginalX;
		}
		else
		{
			X += XAdvance;
		}
	}
//	printf("VisibleCharCount: %d, TotalCharCount: %d\n", VisibleCharCount, TotalCharCount);
//	printf("TotalElapsedOnVisible: %f\n", TotalElapsedVisible);
}

//void CreateTextVertices
//(const char *Text, array<float> *Vertices, array<unsigned int> *Indices, textureCoordinates Chars[], editor *Editor)
//{
//	int XAdvance = Editor->CharWidth + Editor->CharSpacing;
//	int YAdvance = Editor->CharHeight + Editor->LineSpacing;
//	int OriginalX = -Editor->ViewportX;
//	int OriginalY = -Editor->ViewportY;
//	int X = OriginalX;
//	int Y = OriginalY;
//
//	for(int i = 0; Text[i]; ++i)
//	{
//		if(Text[i] == '\r')
//		{
//			continue;
//		}
//
//		if(Y >= 0 && Y < Editor->WindowHeight && X >= 0 && X < Editor->WindowWidth)
//		{
//			char Char = Text[i];
//			if(Char == '\t')
//			{
//				Char = ' ';
//			}
//			if(Char == '\n')
//			{
//				Char = ' ';
//			}
//
//			float X1 = X;                      // upper left
//			float Y1 = Y;                      // upper left
//			float X2 = X + Editor->CharWidth;  // lower right
//			float Y2 = Y + Editor->CharHeight; // lower right
//
////			float FGR = Editor->TextFGColor.R;
////			float FGG = Editor->TextFGColor.G;
////			float FGB = Editor->TextFGColor.B;
////
////			float BGR, BGG, BGB;
////			if(i == Editor->CursorPos)
////			{
////				BGR = Editor->CursorColor.R;
////				BGG = Editor->CursorColor.G;
////				BGB = Editor->CursorColor.B;
////			}
////			else
////			{
////				BGR = Editor->TextBGColor.R;
////				BGG = Editor->TextBGColor.G;
////				BGB = Editor->TextBGColor.B;
////			}
//			float TextR = Editor->TextColor.R;
//			float TextG = Editor->TextColor.G;
//			float TextB = Editor->TextColor.B;
//
//			int Index = Char - ' ';
//
////			float QuadVertices[] = {
////				/*top-left*/      X1, Y1, Chars[Index].X1, Chars[Index].Y1, FGR, FGG, FGB, BGR, BGG, BGB,
////				/*bottom-left*/   X1, Y2, Chars[Index].X1, Chars[Index].Y2, FGR, FGG, FGB, BGR, BGG, BGB,
////				/*bottom-right*/  X2, Y2, Chars[Index].X2, Chars[Index].Y2, FGR, FGG, FGB, BGR, BGG, BGB,
////				/*bottom-right*/  X2, Y2, Chars[Index].X2, Chars[Index].Y2, FGR, FGG, FGB, BGR, BGG, BGB,
////				/*top-right*/     X2, Y1, Chars[Index].X2, Chars[Index].Y1, FGR, FGG, FGB, BGR, BGG, BGB,
////				/*top-left*/      X1, Y1, Chars[Index].X1, Chars[Index].Y1, FGR, FGG, FGB, BGR, BGG, BGB,
//////			};
////
////			float QuadVertices[] = {
////				X1, Y1, Chars[Index].X1, Chars[Index].Y1, FGR, FGG, FGB, BGR, BGG, BGB, /*top-left*/
////				X1, Y2, Chars[Index].X1, Chars[Index].Y2, FGR, FGG, FGB, BGR, BGG, BGB, /*bottom-left*/
////				X2, Y2, Chars[Index].X2, Chars[Index].Y2, FGR, FGG, FGB, BGR, BGG, BGB, /*bottom-right*/
////				X2, Y1, Chars[Index].X2, Chars[Index].Y1, FGR, FGG, FGB, BGR, BGG, BGB, /*top-right*/
////			};
//
//			float QuadVertices[] = {
//				X1, Y1, Chars[Index].X1, Chars[Index].Y1, TextR, TextG, TextB, /*top-left*/
//				X1, Y2, Chars[Index].X1, Chars[Index].Y2, TextR, TextG, TextB, /*bottom-left*/
//				X2, Y2, Chars[Index].X2, Chars[Index].Y2, TextR, TextG, TextB, /*bottom-right*/
//				X2, Y1, Chars[Index].X2, Chars[Index].Y1, TextR, TextG, TextB, /*top-right*/
//			};
//
//			for(int i = 0; i < COUNT(QuadVertices); ++i)
//			{
//				ArrayAdd(Vertices, QuadVertices[i]);
//			}
//
//			//@ Vertices->Count is not the number of vertices, its the number of floats in the array, really misleading
//			int OffsetNewVertices = Vertices->Count / 7 - 4;
//			unsigned int QuadIndices[] = {
//				0, 1, 2,
//				2, 3, 0,
//			};
//			for(int i = 0; i < COUNT(QuadIndices); ++i)
//			{
//				ArrayAdd(Indices, QuadIndices[i] + OffsetNewVertices);
//			}
//		}
//			
//		if(Text[i] == '\n')
//		{
//			Y += YAdvance;
//			X = OriginalX;
//		}
//		else
//		{
//			X += XAdvance;
//		}
//	}
//}

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

//// If the cursor has moved outside the viewport, adjust the viewport's offsets, so that cursor becomes visible again
//void PossiblyUpdateViewportPosition(textBuffer *Buffer, editor *Editor)
//{
//	// Calculate the cursor's position relative to the viewport
//	int CursorX1 = GetCursorCharIndex(Buffer) * (Editor->CharWidth + Editor->CharSpacing) - Editor->ViewportX;
//	int CursorY1 = GetCursorLineIndex(Buffer) * (Editor->CharHeight + Editor->LineSpacing) - Editor->ViewportY;
//	int CursorX2 = CursorX1 + Editor->CharWidth;
//	int CursorY2 = CursorY1 + Editor->CharHeight;
//
//	if(CursorY2 > Editor->WindowHeight)
//	{
//		Editor->ViewportY += CursorY2 - Editor->WindowHeight;
//	}
//	else if(CursorY1 < 0)
//	{
//		Editor->ViewportY += CursorY1;
//	}
//	if(CursorX2 > Editor->WindowWidth)
//	{
//		Editor->ViewportX += CursorX2 - Editor->WindowWidth;
//	}
//	else if(CursorX1 < 0)
//	{
//		Editor->ViewportX += CursorX1;
//	}
//}

//void MakeCursorVertices(array<float> *CursorVertices, textBuffer *Buffer, editor *Editor)
//{
//	int CursorWidth = Editor->CharWidth;
//	int CursorHeight = Editor->CharHeight;
//	float X1 = GetCursorCharIndex(Buffer) * (Editor->CharWidth + Editor->CharSpacing) - Editor->ViewportX;  // left edge
//	float X2 = X1 + CursorWidth;                                                        // right edge
//	float Y1 = GetCursorLineIndex(Buffer) * (Editor->CharHeight + Editor->LineSpacing) - Editor->ViewportY; // upper edge
//	float Y2 = Y1 + CursorHeight;                                                       // lower edge
//	float Vertices[] = {
//		/* upper-left*/  X1, Y1,
//		/* upper-right*/ X2, Y1,
//		/* lower-right*/ X2, Y2,
//
//		/* lower-right*/ X2, Y2,
//		/* lower-left*/  X1, Y2,
//		/* upper-left*/  X1, Y1,
//	};
//	for(unsigned int i = 0; i < COUNT(Vertices); ++i)
//	{
//		ArrayAdd(CursorVertices, Vertices[i]);
//	}
//}

//int GetCursorCharIndex(textBuffer *Buffer)
//{
//	int i = Buffer->Cursor;
//	while(i > 0)
//	{
//		--i;
//		if(Buffer->Data[i] == '\n')
//		{
//			i += 1;
//			break;
//		}
//	}
//	return Buffer->Cursor - i;
//}

//// Figure out on which line the cursor is at
//int GetCursorLineIndex(textBuffer *Buffer)
//{
//	int LineCount = 0; // number of lines before cursor
//	for(int i = Buffer->Cursor - 1; i >= 0; --i)
//	{
//		if(Buffer->Data[i] == '\n')
//		{
//			++LineCount;
//		}
//	}
//	return LineCount;
//}

void InitEditor(editor *Editor, GLFWwindow *AppWindow)
{
//	Editor->FontImage = {
//		.Width = 128,
//		.Height = 64,
//		.CharsInRow = 18,
//		.CharWidth = 5,
//		.CharHeight = 7,
//		.HorizontalSpacing = 2,
//		.VerticalSpacing = 2,
//		.LeftMargin = 1,
//		.TopMargin = 1
//	};
	fontImage FontImage = {0};
	FontImage.Width = 128;
	FontImage.Height = 64;
	FontImage.CharsInRow = 18;
	FontImage.CharWidth = 5;
	FontImage.CharHeight = 7;
	FontImage.HorizontalSpacing = 2;
	FontImage.VerticalSpacing = 2;
	FontImage.LeftMargin = 1;
	FontImage.TopMargin = 1;
	Editor->FontImage = FontImage;

//	Editor->WindowWidth = 0;
//	Editor->WindowHeight = 0;
//	Editor->ViewportX = 0;
//	Editor->ViewportY = 0;
	Editor->CharWidth = 3 * Editor->FontImage.CharWidth;
	Editor->CharHeight = 3 * Editor->FontImage.CharHeight;
	Editor->CharSpacing = 3;
	Editor->LineSpacing = 10;

	color TextColor = {1.0f, 1.0f, 1.0f, 1.0f};
	Editor->TextColor = TextColor;
	color CursorColor = {0.0f, 1.0f, 0.0f, 1.0f};
	Editor->CursorColor = CursorColor;
//	color CursorEffectColor = {0.0f, 0.0f, 0.0f, 1.0f};
//	Editor->CursorEffectColor = CursorEffectColor;
//	Editor->CursorEffectProgress = 1.0f;

	InitTextBuffer(&Editor->TextBuffer);
//	ArrayInit(&Editor->EffectQuads);
//	ArrayInit(&Editor->CharsWithEffect);

	Editor->OpenFile[0] = '\0';

	Editor->AppWindow = AppWindow;
}

//void SetCursor(textBuffer *Buffer, int At, editor *Editor)
//{
//	// Dont put the cursor past the end of the buffer
//	if(At < 0 || At >= Buffer->Size)
//	{
//		return;
//	}
//	// Dont put the cursor past the end of the contents in the text-buffer
//	for(int i = 0; i < At; ++i)
//	{
//		if(Buffer->Data[i] == '\0')
//		{
//			return;
//		}
//	}
//	Buffer->Cursor = At;
//	PossiblyUpdateViewportPosition(Buffer, Editor);
//	printf("Cursor at %d\n", Buffer->Cursor);
//}

//void InsertAtCursor(textBuffer *Buffer, char Char, editor *Editor)
//{
//	// find 0-char at the end
//	int i = Buffer->Cursor;
//	while(Buffer->Data[i] != '\0') ++i;
//
//	assert(i < Buffer->Size);
//	if(i == Buffer->Size-1)
//	{
//		// Dont insert if the buffer is full
//		return;
//	}
//
//	// copy characters
//	while(i >= Buffer->Cursor)
//	{
//		Buffer->Data[i+1] = Buffer->Data[i];
//		--i;
//	}
//
//	Buffer->Data[Buffer->Cursor] = Char;
//	SetCursor(Buffer, Buffer->Cursor+1, Editor);
//}

void MakeQuad(array<float> *Vertices, int X, int Y, int Width, int Height, color Color)
{
	float X1 = X;           // left edge
	float Y1 = Y;           // upper edge
	float X2 = X1 + Width;  // right edge
	float Y2 = Y1 + Height; // lower edge
	float QuadVertices[] = {
		/* upper-left*/  X1, Y1, Color.R, Color.G, Color.B, Color.A,
		/* upper-right*/ X2, Y1, Color.R, Color.G, Color.B, Color.A,
		/* lower-right*/ X2, Y2, Color.R, Color.G, Color.B, Color.A,

		/* lower-right*/ X2, Y2, Color.R, Color.G, Color.B, Color.A,
		/* lower-left*/  X1, Y2, Color.R, Color.G, Color.B, Color.A,
		/* upper-left*/  X1, Y1, Color.R, Color.G, Color.B, Color.A,
	};
	for(unsigned int i = 0; i < COUNT(QuadVertices); ++i)
	{
		ArrayAdd(Vertices, QuadVertices[i]);
	}
}

float Lerp(float From, float To, float Progress)
{
	return (1 - Progress) * From + Progress * To;
}

void CursorForward(textBuffer *Buffer)
{
	assert(Buffer->Data[Buffer->Cursor] != '\0');
	Buffer->Cursor += 1;
}

void CursorBackward(textBuffer *Buffer)
{
	assert(Buffer->Cursor != 0);
	Buffer->Cursor -= 1;
}

// adjust viewport if CharWidth * CharHeight quad at (CharIndex, LineIndex) is not visible, so that it becomes visible again
// returns true if actually adjusted the viewport?
void AdjustViewportIfNotVisible(editor *Editor, int CharIndex, int LineIndex)
{
	// Calculate the cursor's position relative to the viewport
	int CursorX1 = CharIndex * (Editor->CharWidth + Editor->CharSpacing) - Editor->ViewportX;
	int CursorY1 = LineIndex * (Editor->CharHeight + Editor->LineSpacing) - Editor->ViewportY;
	int CursorX2 = CursorX1 + Editor->CharWidth;
	int CursorY2 = CursorY1 + Editor->CharHeight;

	if(CursorY2 > Editor->WindowHeight)
	{
		Editor->ViewportY += CursorY2 - Editor->WindowHeight;
	}
	else if(CursorY1 < 0)
	{
		Editor->ViewportY += CursorY1;
	}
	if(CursorX2 > Editor->WindowWidth)
	{
		Editor->ViewportX += CursorX2 - Editor->WindowWidth;
	}
	else if(CursorX1 < 0)
	{
		Editor->ViewportX += CursorX1;
	}
}

void InitTextBuffer(textBuffer *Buffer)
{
//	Buffer->Size = 3;
	Buffer->Size = 1024;	
	Buffer->Data = (char *) malloc(Buffer->Size);
	assert(Buffer->Data);
	Buffer->OneAfterLast = 0;
	Buffer->Cursor = 0;

//	memset(Buffer->Data, 'x', Buffer->Size);

//	Buffer->Data[0] = 'a';
//	Buffer->Data[1] = 'b';
//	Buffer->Data[2] = 'c';
//	Buffer->OneAfterLast = 3;
}

char GetChar(textBuffer *Buffer, int At)
{
	// validate iterator
	assert(0 <= At && At < Buffer->OneAfterLast+1);

	if(At == Buffer->OneAfterLast)
	{
		return '\0'; //@ ?
	}
	else
	{
		return Buffer->Data[At];
	}
}

int GetStart(textBuffer *Buffer)
{
	return 0;
}

int GetEnd(textBuffer *Buffer)
{
//	int i = 0;
//	while(true)	
//	{
//		assert(i < Buffer->Size);
//		if(Buffer->Data[i] == '\0')
//		{
//			return i;
//		}
//		++i;
//	}
	return Buffer->OneAfterLast;
}

bool IsStart(textBuffer *Buffer, int At)
{
	return (At == 0);
}

bool IsEnd(textBuffer *Buffer, int At)
{
	return (At == Buffer->OneAfterLast);
}

bool MoveForward(textBuffer *Buffer, int *Iter)
{
	// validate iterator
	assert(0 <= *Iter && *Iter < Buffer->OneAfterLast+1);

	if(*Iter == Buffer->OneAfterLast)
	{
		return false; // at the end
	}
	else
	{
		*Iter += 1;
		return true;
	}
}

void MoveForwardFast(textBuffer *Buffer, int *Iter)
{
	*Iter += 1;
}

bool MoveBackward(textBuffer *Buffer, int *Iter)
{
	// validate iterator
	assert(0 <= *Iter && *Iter < Buffer->OneAfterLast+1);

	if(*Iter == 0)
	{
		return false; // at the beginning
	}
	else
	{
		*Iter -= 1;
		return true;
	}
}

int GetCursor(textBuffer *Buffer)
{
	return Buffer->Cursor;
}

void SetCursor(textBuffer *Buffer, int At)
{
	assert(0 <= At && At < Buffer->OneAfterLast+1);

	Buffer->Cursor = At;
}

int GetCharsIntoLine(textBuffer *Buffer, int At)
{
	assert(0 <= At && At < Buffer->OneAfterLast+1);

	int i = At;
	while(i > 0)
	{
		--i;
		if(Buffer->Data[i] == '\n')
		{
			i += 1;
			break;
		}
	}
	return At - i;
}

int GetLinesIntoBuffer(textBuffer *Buffer, int At)
{
	assert(0 <= At && At < Buffer->OneAfterLast+1);

	int LineCount = 0;
	for(int i = At - 1; i >= 0; --i)
	{
		if(Buffer->Data[i] == '\n')
		{
			++LineCount;
		}
	}
	return LineCount;
}

//bool MoveAtCharForwards(textBuffer *Buffer, char Char, int *Iter)
//{}
bool MoveAtCharBackwards(textBuffer *Buffer, char Char, int *Iter)
{
	assert(0 <= *Iter && *Iter < Buffer->OneAfterLast+1);

	bool Result = false;
	int i = *Iter;
	do
	{
		if(MoveBackward(Buffer, &i))
		{
			if(GetChar(Buffer, i) == '\n')
			{
				*Iter = i;
				Result = true;
				break;
			}
		}
		else
		{
			break;
		}
	}
	while(!IsStart(Buffer, *Iter));
	return Result;
}

// moves Iter to the beginning of previous line and returns true
// if no previous line, returns false
bool MoveToPrevLine(textBuffer *Buffer, int *Iter)
{
	assert(0 <= *Iter && *Iter < Buffer->OneAfterLast+1);

//	bool BrokeOut = false;
//	bool OnPreviousLine = false;
//	int i = *Iter;
////	while(i > 0)
//	while(!IsStart(Buffer, i))
//	{
//		--i;
//		if(GetChar(Buffer, i) == '\n')
//		{
//			if(OnPreviousLine)
//			{
//				BrokeOut = true;
//				break;
//			}
//			else
//			{
//				OnPreviousLine = true;
//			}
//		}
//	}
//	bool Result = (BrokeOut || OnPreviousLine);
//	if(Result)
//	{
//		*Iter = i;
//		*Iter += BrokeOut ? 1 : 0;
//	}
//	return Result;

	bool Result = false;
	if(MoveAtCharBackwards(Buffer, '\n', Iter))
	{
		// go to the beginning of the line
		while(!IsStart(Buffer, *Iter))
		{
			MoveBackward(Buffer, Iter);
			if(GetChar(Buffer, *Iter) == '\n')
			{
				MoveForward(Buffer, Iter);
				break;
			}
		}
		Result = true;
	}
	return Result;
}

// moves Iter to the beginning of next line and returns true
// if no next line, returns false
bool MoveToNextLine(textBuffer *Buffer, int *Iter)
{
	assert(0 <= *Iter && *Iter < Buffer->OneAfterLast+1);

	int i = *Iter;
	bool FoundLine = false;
//	while(GetChar(Buffer, i) != '\0')
	while(i < Buffer->OneAfterLast)
	{
		if(GetChar(Buffer, i) == '\n')
		{
			FoundLine = true;
			i += 1;
			break;
		}
		++i;
	}
	if(FoundLine)
	{
		*Iter = i;
	}
	return FoundLine;
}

bool Insert(textBuffer *Buffer, char Char, int At)
{
	assert(0 <= At && At <= Buffer->OneAfterLast);

	bool Result = false; // Assume buffer is full

	if(Buffer->OneAfterLast < Buffer->Size)
	{
		int i = Buffer->OneAfterLast;
		while(i > At)
		{
			Buffer->Data[i] = Buffer->Data[i-1];
			--i;
		}
		Buffer->Data[At] = Char;
		Buffer->OneAfterLast += 1;
		Result = true;
	}

	return Result;
}

bool Insert(textBuffer *Buffer, const char *Text, int At)
{
	assert(0 <= At && At <= Buffer->OneAfterLast);

	bool Result = false; // Assume buffer is full
	int TextLength = strlen(Text);

	if(!((Buffer->OneAfterLast + TextLength) <= Buffer->Size))
	{
		int NewDataSize = 2 * (Buffer->OneAfterLast + TextLength);
		char *NewData = (char *) malloc(NewDataSize);
		assert(NewData);
		for(int i = 0; i < Buffer->OneAfterLast; ++i)
		{
			NewData[i] = Buffer->Data[i];
		}
		free(Buffer->Data);
		Buffer->Data = NewData;
		Buffer->Size = NewDataSize;
		printf("ALLOCATED NEW MEMORY: %d\n", Buffer->Size);
	}

	int i = Buffer->OneAfterLast-1;
	while(i >= At)
	{
		Buffer->Data[i+TextLength] = Buffer->Data[i];
		--i;
	}
	for(int i = 0; i < TextLength; ++i)
	{
		Buffer->Data[At+i] = Text[i];
	}
	Buffer->OneAfterLast += TextLength;
	Result = true;

	return Result;
}

/*
we dont expect the caller to keep track of whether the buffer is empty or not (well return false if the buffer is empty)
we do expect the caller to not give us invalid[1] iterators though (when we see one well fail an assertion)

[1] invalid iterator is an iterator which is either outside of the buffer bounds or after the '\0' that marks the end of the buffer contents.
*/
bool Delete(textBuffer *Buffer, int At)
{
	// validate an iterator
	assert(0 <= At && At <= Buffer->OneAfterLast);

	if(Buffer->OneAfterLast > 0)
	{
//		do
//		{
//			++At;
//			Buffer->Data[At-1] = Buffer->Data[At];
//		}
//		while(At < Buffer->OneAfterLast-1);
		while(At < Buffer->OneAfterLast-1)
		{
			Buffer->Data[At] = Buffer->Data[At+1];
			++At;
		}
		Buffer->OneAfterLast -= 1;
		return true;
	}
	else
	{
		// buffer is empty or At is at the end of the buffer
		return false;
	}
}

bool Delete(textBuffer *Buffer, int At, int NumChars)
{
	// validate an iterator
	assert(0 <= At && At <= Buffer->OneAfterLast);

	// cant delete more characters than there are
	assert(NumChars <= Buffer->OneAfterLast);

	//@ if the caller wants to delete more chars than can be deleted... we dont check for that

	if(NumChars <= Buffer->OneAfterLast) //@ because of the assertion above else-block never executes
	{
		while(At+NumChars < Buffer->OneAfterLast)
		{
			Buffer->Data[At] = Buffer->Data[At+NumChars];
			++At;
		}
		Buffer->OneAfterLast -= NumChars;
		return true;
	}
	else
	{
		return false;
	}
	return false;
}