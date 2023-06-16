/*
	* ~/textdb (over million lines) is very slow. Why?
	* things shaking when resizing the window
	* Why is glfwSwapBuffers() taking ~1-3 ms if we are not even drawing anything besides the background?
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
//#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

#include "lib.h"

#include "text_buffer.hpp"
#include "text_drawing.hpp"
#include "ui.hpp"

extern shaders Shaders; // should probably be in editor

const int MAX_FILE_PATH = 1000; //@ very random

enum messageType
{
	NONE,
	SUCCESS_MESSAGE,
	ERROR_MESSAGE
};

struct textureCoordinatesOld
{
	float X1, X2, Y1, Y2;
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

struct textMetrics
{
	int CharWidth;
	int CharHeight;
	int VSpacing;
	int HSpacing;
};

struct fontImage
{
	int Width, Height;
	int CharsInRow;
	int CharWidth, CharHeight;
	int HorizontalSpacing, VerticalSpacing;
	int LeftMargin, TopMargin;
};

struct editor
{
	int WindowWidth, WindowHeight;

	color TextColor;
	color CursorColor;
//	color CursorEffectColor;
//	float CursorEffectProgress;

//	array<quad> EffectQuads;
//	array<charColor> CharsWithEffect;

	char OpenFile[MAX_FILE_PATH];

	GLFWwindow *AppWindow;

//	// Message-box
//	double MBDuration;
//	bool MBShouldDisplay;
//	double MBStartTime;

	messageType Message;
	char MessageText[1000];
	double MessageStartTime;

	textBuffer TextBuffer;

	editableText EditableText;

	bitmapFont *Font8x16px;
	bitmapFont *Font16x32px;
	bitmapFont *Font32x64px;
} Editor;

//void InitEditor(editor *Editor, GLFWwindow *AppWindow);
void InitEditor(editor *Editor);

//void CreateTextVertices
//(const char *Text, array<float> *Vertices, array<unsigned int> *Indices, textureCoordinates Chars[], editor *Editor);
//void MakeTextVertices(array<float> *Vertices, textBuffer *Buffer, textureCoordinatesOld Chars[], int NumChars, editor *Editor);
void MakeTextVertices(array<float> *Vertices, const char *Text, int OriginalX, int OriginalY, color TextColor, textMetrics TextMetrics, textureCoordinatesOld Chars[], int NumChars, editor *Editor);
//void CreateTextVertices(array<float> *Vertices, const char *Text, textureCoordinates Chars[]);
//void MakeCursorVertices(array<float> *CursorVertices, textBuffer *Buffer, editor *Editor);
void MakeQuad(array<float> *Vertices, int X, int Y, int Width, int Height, color Color);

//void AdjustViewportIfNotVisible(editor *Editor, int CharIndex, int LineIndex);
void AdjustViewportIfNotVisible(editableText *Editable, int Iter);
float Lerp(float From, float To, float Progress);
void DisplayMessage(messageType MessageType, const char *MessageText, editor *Editor);

void OnWindowResized(GLFWwindow *Window, int Width, int Height)
{
	glViewport(0, 0, Width, Height);
	Editor.WindowWidth = Width;
	Editor.WindowHeight = Height;

	// Shaders is in drawing.cpp
	Shaders.WindowWidth = Width;
	Shaders.WindowHeight = Height;
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

//		int Cursor = GetCursor(Buffer);
//		if(Insert(Buffer, (char)Codepoint, Cursor))
//		{
////			CursorForward(Buffer);
//			MoveForward(Buffer, &Cursor);
//			SetCursor(Buffer, Cursor);
//			int NewCursor = GetCursor(Buffer);
//			int NewCursorCharIndex = GetCharsIntoLine(Buffer, NewCursor);
//			int NewCursorLineIndex = GetLinesIntoBuffer(Buffer, NewCursor);
//			AdjustViewportIfNotVisible(&Editor, NewCursorCharIndex, NewCursorLineIndex);
//		}
//		else
//		{
//			printf("BUFFER IS FULL\n");
//		}

		if(Insert(Buffer, (char)Codepoint, Editor.EditableText.Cursor))
		{
			MoveForward(Buffer, &Editor.EditableText.Cursor);
			AdjustViewportIfNotVisible(&Editor.EditableText, Editor.EditableText.Cursor);
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
	editableText *Editable = &Editor.EditableText;

//	// CTRL + S
//	if(Key == GLFW_KEY_S && Mods == GLFW_MOD_CONTROL && Action == GLFW_PRESS)
//	{
//		printf("S + Ctrl\n");
//		if(Editor.OpenFile[0] != '\0')
//		{
//			if(!WriteFile(Editor.OpenFile, (u8 *) Editor.TextBuffer.Data, Editor.TextBuffer.OneAfterLast))
//			{
////				printf("Error: Failed to write to %s\n", Editor.OpenFile);
//				DisplayMessage(ERROR_MESSAGE, "Error: Failed to write!", &Editor);
//			}
//			else
//			{
//				DisplayMessage(SUCCESS_MESSAGE, "Saved!", &Editor);
//			}
//		}
//		else
//		{
//			DisplayMessage(ERROR_MESSAGE, "Error: No file!", &Editor);
//		}
//
//		Editor.MBShouldDisplay = true;
//		Editor.MBStartTime = glfwGetTime();
//
//		return;
//	}

	if (Key == GLFW_KEY_UP && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
//		int Iter = GetCursor(Buffer);
//		int CharsIntoLine = GetCharsIntoLine(Buffer, Iter);
//		if(MoveToPrevLine(Buffer, &Iter))
//		{
//			// If there is a previous line (we are not on the first line)
//			while(CharsIntoLine > 0)
//			{
//				if(GetChar(Buffer, Iter) == '\n')
//				{
//					break;
//				}
//				--CharsIntoLine;
//				++Iter;
//			}
//			SetCursor(Buffer, Iter);
//			int NewCharsIntoLine = GetCharsIntoLine(Buffer, Iter);
//			int NewLinesIntoBuffer = GetLinesIntoBuffer(Buffer, Iter);
//			AdjustViewportIfNotVisible(&Editor, NewCharsIntoLine, NewLinesIntoBuffer);
//		}
//		else
//		{
//			printf("NO PREV LINE\n");
//		}

		int CharsIntoLine = GetCharsIntoLine(Buffer, Editable->Cursor);
		if(MoveToPrevLine(Buffer, &Editable->Cursor))
		{
			// If there is a previous line (we are not on the first line)
			while(CharsIntoLine > 0)
			{
				if(GetChar(Buffer, Editable->Cursor) == '\n')
				{
					break;
				}
				--CharsIntoLine;
				MoveForward(Buffer, &Editable->Cursor);
			}
			AdjustViewportIfNotVisible(Editable, Editable->Cursor);
		}
		else
		{
			// no previous line
		}
	}
	if (Key == GLFW_KEY_DOWN && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
//		int Cursor = GetCursor(Buffer);
//		int CharsIntoLine = GetCharsIntoLine(Buffer, Cursor);
//		if(MoveToNextLine(Buffer, &Cursor))
//		{
//			// If there is a previous line (we are not on the first line)
//			while(CharsIntoLine > 0)
//			{
//				char Char = GetChar(Buffer, Cursor);
//				if(Char == '\n' || Char == '\0') //@ '\0'?
//				{
//					break;
//				}
//				--CharsIntoLine;
//				MoveForward(Buffer, &Cursor);
//			}
//			SetCursor(Buffer, Cursor);
//			int NewCharsIntoLine = GetCharsIntoLine(Buffer, Cursor);
//			int NewLinesIntoBuffer = GetLinesIntoBuffer(Buffer, Cursor);
//			AdjustViewportIfNotVisible(&Editor, NewCharsIntoLine, NewLinesIntoBuffer);
//		}

		int CharsIntoLine = GetCharsIntoLine(Buffer, Editable->Cursor);
		if(MoveToNextLine(Buffer, &Editable->Cursor))
		{
			// If there is a previous line (we are not on the first line)
			while(CharsIntoLine > 0)
			{
				char Char = GetChar(Buffer, Editable->Cursor);
				if(Char == '\n' || Char == '\0') //@ '\0'?
				{
					break;
				}
				--CharsIntoLine;
				MoveForward(Buffer, &Editable->Cursor);
			}
			AdjustViewportIfNotVisible(Editable, Editable->Cursor);
		}
	}
	if (Key == GLFW_KEY_LEFT && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
////		int Cursor = GetCursor(Buffer);
////		if(!IsStart(Buffer, Cursor))
////		{
////			int NewCursor = Cursor-1;
////			SetCursor(Buffer, NewCursor);
////			int CharsIntoLine = GetCharsIntoLine(Buffer, NewCursor);
////			int LinesIntoBuffer = GetLinesIntoBuffer(Buffer, NewCursor);
////			AdjustViewportIfNotVisible(&Editor, CharsIntoLine, LinesIntoBuffer);
////		}
//		int Cursor = GetCursor(Buffer);
//		MoveBackward(Buffer, &Cursor);
//		SetCursor(Buffer, Cursor);
//
//		int CharsIntoLine = GetCharsIntoLine(Buffer, Cursor);
//		int LinesIntoBuffer = GetLinesIntoBuffer(Buffer, Cursor);
//		AdjustViewportIfNotVisible(&Editor, CharsIntoLine, LinesIntoBuffer);

		if(MoveBackward(Buffer, &Editable->Cursor))
		{
			AdjustViewportIfNotVisible(Editable, Editable->Cursor);
		}
	}
	if (Key == GLFW_KEY_RIGHT && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
////		int Cursor = GetCursor(Buffer);
////		if(!IsEnd(Buffer, Cursor))
////		{
////			int NewCursor = Cursor+1;
////			SetCursor(Buffer, NewCursor);
////			int CharsIntoLine = GetCharsIntoLine(Buffer, NewCursor);
////			int LinesIntoBuffer = GetLinesIntoBuffer(Buffer, NewCursor);
////			AdjustViewportIfNotVisible(&Editor, CharsIntoLine, LinesIntoBuffer);
////		}
//		int Cursor = GetCursor(Buffer);
//		if(MoveForward(Buffer, &Cursor))
//		{
//			int CharsIntoLine = GetCharsIntoLine(Buffer, Cursor);
//			int LinesIntoBuffer = GetLinesIntoBuffer(Buffer, Cursor);
//			AdjustViewportIfNotVisible(&Editor, CharsIntoLine, LinesIntoBuffer);
//		}
//		SetCursor(Buffer, Cursor);

		if(MoveForward(Buffer, &Editable->Cursor))
		{
			AdjustViewportIfNotVisible(Editable, Editable->Cursor);
		}
	}

	if(Key == GLFW_KEY_ENTER && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
//		int Cursor = GetCursor(Buffer);
//		if(Insert(Buffer, '\n', Cursor))
//		{
////			CursorForward(Buffer);
////			int NewCursor = GetCursor(Buffer);
////			int NewCursorCharIndex = GetCharsIntoLine(Buffer, NewCursor);
////			int NewCursorLineIndex = GetLinesIntoBuffer(Buffer, NewCursor);
//			MoveForward(Buffer, &Cursor);
//			SetCursor(Buffer, Cursor);
//			int CursorCharIndex = GetCharsIntoLine(Buffer, Cursor);
//			int CursorLineIndex = GetLinesIntoBuffer(Buffer, Cursor);
//			AdjustViewportIfNotVisible(&Editor, CursorCharIndex, CursorLineIndex);
//		}
//		else
//		{
//			printf("BUFFER IS FULL\n");
//		}

		if(Insert(Buffer, '\n', Editable->Cursor))
		{
			MoveForward(Buffer, &Editable->Cursor);
			AdjustViewportIfNotVisible(Editable, Editable->Cursor);
		}
		else
		{
			printf("BUFFER IS FULL\n");
		}
	}
	if(Key == GLFW_KEY_BACKSPACE && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
//		// delete character before cursor
//		int Iter = GetCursor(Buffer);
//		if(!IsStart(Buffer, Iter))
//		{
//			MoveBackward(Buffer, &Iter);
//			if(Delete(Buffer, Iter))
//			{
//				SetCursor(Buffer, Iter);
//				int CharIndex = GetCharsIntoLine(Buffer, Iter);
//				int LineIndex = GetLinesIntoBuffer(Buffer, Iter);
//				AdjustViewportIfNotVisible(&Editor, CharIndex, LineIndex);
//			}
//			else
//			{
//				assert(false); // should never happen
//			}
//		}

		// delete character before cursor
		if(!IsStart(Buffer, Editable->Cursor))
		{
			MoveBackward(Buffer, &Editable->Cursor);
			if(Delete(Buffer, Editable->Cursor))
			{
				AdjustViewportIfNotVisible(Editable, Editable->Cursor);
			}
			else
			{
				assert(false); // should never happen
			}
		}
	}
}

//void OnFileDrop(GLFWwindow *Window, int Count, const char **Paths)
//{
////	if(Editor.FileContents)
////	{
////		free(Editor.FileContents);
////	}
//
//	char *Contents;
//	if(!ReadTextFile(Paths[0], &Contents))
//	{
//		fprintf(stderr, "error: failed to read a dropped file: %s\n", Paths[0]);
//		return;
//	}
//	printf("Read file \"%s\" successfully\n", Paths[0]);
//	textBuffer *Buffer = &Editor.TextBuffer;
//	int NumCharsInBuffer = Buffer->OneAfterLast;
//	Delete(Buffer, GetStart(Buffer), NumCharsInBuffer);
//	if(Insert(Buffer, Contents, GetStart(Buffer)))
//	{
//		printf("Loaded file successfully\n");
//		strcpy(Editor.OpenFile, Paths[0]); //@ bounds
//		glfwSetWindowTitle(Editor.AppWindow, Editor.OpenFile);
//	}
//	else
//	{
//		printf("File too large!\n");
//	}
//	SetCursor(Buffer, GetStart(Buffer));
//
//	Editor.ViewportX = 0;
//	Editor.ViewportY = 0;
//
//	free(Contents);
//}

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

	GLFWmonitor *Monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *VideoMode = glfwGetVideoMode(Monitor);
	printf("screen width: %d, screen height: %d, screen refresh rate: %d\n", VideoMode->width, VideoMode->height, VideoMode->refreshRate);

//	GLFWwindow *window = glfwCreateWindow(800, 600, "My Window", NULL, NULL);
	GLFWwindow *window = glfwCreateWindow(VideoMode->width, VideoMode->height, "My Window", NULL, NULL);
	if(window == NULL)
	{
		printf("error: glfwCreateWindow()\n");
		return 1;
	}

	glfwMakeContextCurrent(window);
//	glfwSwapInterval(0); // VSync off
	gladLoadGL();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // some images need this to display correctly

	// Dimensions given by glfwGetWindowSize() might not be in pixels
	glfwGetFramebufferSize(window, &Editor.WindowWidth, &Editor.WindowHeight);
	printf("window width: %d, window height: %d\n", Editor.WindowWidth, Editor.WindowHeight);

	init_shaders(Editor.WindowWidth, Editor.WindowHeight);

	InitEditor(&Editor);

	glfwSetFramebufferSizeCallback(window, OnWindowResized);
	glfwSetCharCallback(window, OnCharEvent);
	glfwSetKeyCallback(window, OnKeyEvent);
//	glfwSetDropCallback(window, OnFileDrop);

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

//	int ImageWidth, ImageHeight, ImageNChannels;
////	const char *ImageFile = "../charmap-oldschool_white.png";
//	const char *ImageFile = "../charmap-oldschool_black.png";
//	stbi_set_flip_vertically_on_load(1);
//	unsigned char *ImageData = stbi_load(ImageFile, &ImageWidth, &ImageHeight, &ImageNChannels, 0);
//	if(!ImageData)
//	{
//		fprintf(stderr, "error: stbi_load()\n");
//		return 1;
//	}
//	else
//	{
//		printf("width: %d, height: %d, num channels: %d\n", ImageWidth, ImageHeight, ImageNChannels);
//	}
//
//	GLuint myTexture;
//	glGenTextures(1, &myTexture);
//	glBindTexture(GL_TEXTURE_2D, myTexture);
//
//	// What to do when texture coordinates are outside of the texture:
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//
//	// What to do when the texture is minified/magnified:
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
////	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ImageWidth, ImageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, ImageData);
////	glGenerateMipmap(GL_TEXTURE_2D);
//
//	stbi_image_free(ImageData);
//
//	const int NumChars = 127 - ' ';
//	textureCoordinatesOld Chars[NumChars];
//	{
//		float PixelWidth = 1.0f / Editor.FontImage.Width;
//		float PixelHeight = 1.0f / Editor.FontImage.Height;
//		for(char Char = ' '; Char < 127; ++Char)
//		{
//			int Index = Char - ' ';
//			int Row = Index / Editor.FontImage.CharsInRow;
//			int Column = Index % Editor.FontImage.CharsInRow;
//			int CharWidthAndSome = Editor.FontImage.CharWidth + Editor.FontImage.HorizontalSpacing;
//			int CharHeightAndSome = Editor.FontImage.CharHeight + Editor.FontImage.VerticalSpacing;
//
//			Chars[Index].X1 = (Editor.FontImage.LeftMargin + Column * CharWidthAndSome) * PixelWidth;
//			Chars[Index].X2 = Chars[Index].X1 + Editor.FontImage.CharWidth * PixelWidth;
//			Chars[Index].Y1 = ((Editor.FontImage.Height - Editor.FontImage.TopMargin) - Row * CharHeightAndSome) * PixelHeight;
//			Chars[Index].Y2 = Chars[Index].Y1 - Editor.FontImage.CharHeight * PixelHeight;
//		}
//	}

//	GLuint TextShader = CreateShader("../text-vs", "../text-fs");
//	GLuint QuadShader = CreateShader("../quad-vs", "../quad-fs");

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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
	glEnableVertexAttribArray(0);

	// texture coordinates
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (2 * sizeof(float)));
	glEnableVertexAttribArray(1);

//	// foreground color
//	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *) (4 * sizeof(float)));
//	glEnableVertexAttribArray(2);
//
//	// background color
//	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *) (7 * sizeof(float)));
//	glEnableVertexAttribArray(3);

	// color
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (4 * sizeof(float)));
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

	image *Image1 = make_image("/home/eero/Desktop/pics/desktop-wallpaper-mystic-for-computer-mystical-nature.jpg");
//	image *TheImage = make_image("/home/eero/Desktop/pics/desktop-wallpaper-best-5-mystical-on-hip-mystical-landscape.png");
	image *Image2 = make_image("/home/eero/Desktop/pics/desktop-wallpaper-best-5-mystical-on-hip-mystical-landscape.jpg");
//	image *TheImage = make_image("/home/eero/Desktop/pics/ylad25oswrua1.png");

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
			printf("elapsed: %.3f ms\n", TimeElapsed * 1000);
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

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		{
			int X = 0;
			int Y = 0;
			int W = Image1->W;
			int H = Image1->H;
			draw_quad(X, Y, W, H, Image1->Tex);
		}
		{
			int W = Image2->W;
			int H = Image2->H;
			int X = Editor.WindowWidth - W;
			int Y = Editor.WindowHeight - H;
			draw_quad(X, Y, W, H, Image2->Tex);
		}

		{
			int X = 10;
			int Y = 10;
			int W = 100;
			int H = 100;
			color Color = {1.0f, 0.0f, 0.0f, 1.0f};
			draw_quad(X, Y, W, H, Color);
		}

		{
			int X = 10;
			int Y = 10;
			color Color = {1.0f, 1.0f, 1.0f, 1.0f};
			draw_text("Hello world!", X, Y, Color, Editor.Font8x16px, &Shaders);
		}
		{
			int X = 10;
			int Y = 40;
			color Color = {0.0f, 1.0f, 0.0f, 1.0f};
			draw_text("Hello world!", X, Y, Color, Editor.Font16x32px, &Shaders);
		}
		{
			int X = 10;
			int Y = 50;
			color Color = {0.0f, 0.0f, 1.0f, 0.5f};
			draw_text("This is supposed to be a really long sentence...",
				X, Y, Color, Editor.Font32x64px, &Shaders);
		}
		{
			int X = 10;
			int Y = 150;
			int W = 95 * 16;
			int H = 32;
			draw_quad(X, Y, W, H, Editor.Font16x32px->TextureAtlas->Tex);
		}

		draw_editable_text(&Editor.EditableText, &Shaders);

//		glUseProgram(QuadShader);
//		glUniform1f(glGetUniformLocation(QuadShader, "WindowWidth"), Editor.WindowWidth);
//		glUniform1f(glGetUniformLocation(QuadShader, "WindowHeight"), Editor.WindowHeight);

//		// Make the cursor
////		color CursorColor;
////		if(Editor.CursorEffectProgress < 1.0f)
////		{
////			CursorColor.A = Editor.CursorColor.A;
////			CursorColor.R = Lerp(Editor.CursorEffectColor.R, Editor.CursorColor.R, Editor.CursorEffectProgress);
////			CursorColor.G = Lerp(Editor.CursorEffectColor.G, Editor.CursorColor.G, Editor.CursorEffectProgress);
////			CursorColor.B = Lerp(Editor.CursorEffectColor.B, Editor.CursorColor.B, Editor.CursorEffectProgress);
////		}
////		else
////		{
////			CursorColor = Editor.CursorColor;
////		}
////		Editor.CursorEffectProgress += 0.01f;
////		if(Editor.CursorEffectProgress > 1.0f)
////		{
////			Editor.CursorEffectProgress = 1.0f;
////		}
////		int CursorWidth = Editor.CharWidth;
//		int CursorWidth = Editor.EditableText.Font->Config.CharWidth;
////		int CursorHeight = Editor.CharHeight;
//		int CursorHeight = Editor.EditableText.Font->Config.CharHeight;
//		textBuffer *Buffer = &Editor.TextBuffer;
//		int Cursor = GetCursor(Buffer);
////		float X = GetCharsIntoLine(Buffer, Cursor) * (Editor.CharWidth + Editor.CharSpacing) - Editor.ViewportX;
//		float X = GetCharsIntoLine(Buffer, Cursor) * (Editor.EditableText.Font->Config.CharWidth + Editor.EditableText.Font->Config.CharSpacing) + 100;
////		float Y = GetLinesIntoBuffer(Buffer, Cursor) * (Editor.CharHeight + Editor.LineSpacing) - Editor.ViewportY;
//		float Y = GetLinesIntoBuffer(Buffer, Cursor) * (Editor.EditableText.Font->Config.CharHeight + Editor.EditableText.Font->Config.LineSpacing) + 100;
//		MakeQuad(&QuadVertices, X, Y, CursorWidth, CursorHeight, Editor.CursorColor);
//
////		for(int i = 0; i < Editor.EffectQuads.Count; ++i)
////		{
////			MakeQuad(
////				&QuadVertices,
////				Editor.EffectQuads.Data[i].X - Editor.ViewportX,
////				Editor.EffectQuads.Data[i].Y - Editor.ViewportY,
////				Editor.CharWidth, Editor.CharHeight,
////				Editor.EffectQuads.Data[i].Color);
////			Editor.EffectQuads.Data[i].Color.A -= 0.01;
////			printf("effect quad %d alpha: %f\n", i, Editor.EffectQuads.Data[i].Color.A);
////			if(Editor.EffectQuads.Data[i].Color.A <= 0.0f)
////			{
////				ArrayRemove(&Editor.EffectQuads, i);
////				printf("removed an effect quad!\n");
////			}
////		}
//		glBindVertexArray(QuadVAO);
//		glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
//		glBufferData(GL_ARRAY_BUFFER, QuadVertices.Count * sizeof(float), QuadVertices.Data, GL_STREAM_DRAW);
//		glDrawArrays(GL_TRIANGLES, 0, QuadVertices.Count / 6);

//		glBindTexture(GL_TEXTURE_2D, myTexture);
//		glUseProgram(TextShader);
//		glUniform1f(glGetUniformLocation(TextShader, "WindowWidth"), Editor.WindowWidth);
//		glUniform1f(glGetUniformLocation(TextShader, "WindowHeight"), Editor.WindowHeight);
//
//		double Before = glfwGetTime();
//		MakeTextVertices(&TextVertices, &Editor.TextBuffer, Chars, NumChars, &Editor);
//		double After = glfwGetTime();
////		printf("MakeTextVertices(): %f\n", After - Before);
//		glBindVertexArray(TextVAO);
//		glBindBuffer(GL_ARRAY_BUFFER, TextVBO);
////		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TextEBO);
////		glBufferData(GL_ARRAY_BUFFER, TextVertices.Count * sizeof(float), TextVertices.Data, GL_STATIC_DRAW);
//		glBufferData(GL_ARRAY_BUFFER, TextVertices.Count * sizeof(float), TextVertices.Data, GL_STREAM_DRAW);
////		glBufferData(GL_ELEMENT_ARRAY_BUFFER, TextIndices.Count * sizeof(unsigned int), TextIndices.Data, GL_STREAM_DRAW);
////		glDrawElements(GL_TRIANGLES, TextIndices.Count, GL_UNSIGNED_INT, 0);
//		glDrawArrays(GL_TRIANGLES, 0, TextVertices.Count / 8);

//		int TextBoxX = 100;
//		int TextBoxY = 100;
//		int TextBoxW = 100;
//		int TextBoxH = 100;
//		int OffsH = 0;
//		int OffsV = 0;
//		draw_text_buffer(&Editor.TextBuffer, Editor.EditableText.Font,
//			TextBoxX, TextBoxY, TextBoxW, TextBoxH,
//			OffsH, OffsV,
//			Editor.WindowWidth, Editor.WindowHeight);

//		// Make message-box
//		if(Editor.Message != NONE)
//		{
//			int Duration = 3.0;
//			double T = glfwGetTime();
//			double TimeTaken = T - Editor.MessageStartTime;
//			if(TimeTaken < Duration)
//			{
//				float Alpha = 1.0 - TimeTaken / Duration;
////				float Alpha = 1.0;
//
//				textMetrics TextMetrics = {0};
//				TextMetrics.CharWidth = 2 * Editor.FontImage.CharWidth;
//				TextMetrics.CharHeight = 2 * Editor.FontImage.CharHeight;
//				TextMetrics.HSpacing = 6;
//				TextMetrics.VSpacing = 10;
//
//				const char *MessageText = Editor.MessageText;
////				const char *MessageText = "Hello, world!";
////				const char *MessageText = "Hello,\n world!";
////				const char *MessageText = "1\n2\n3\n4\n5";
////				const char *MessageText = "1\n12\n123\n1234\n12345";
//				int NumNewlines = 0;
//				int CharCount = 0;
//				int MaxCharCount = 0; // longest line
//				for(int i = 0; MessageText[i]; ++i)
//				{
//					if(MessageText[i] == '\n')
//					{
//						NumNewlines += 1;
//						if(CharCount > MaxCharCount)
//						{
//							MaxCharCount = CharCount;
//						}
//						CharCount = 0;
//						continue;
//					}
//					CharCount += 1;
//				}
//				if(MaxCharCount < CharCount) MaxCharCount = CharCount; // If no newlines
//				int TextWidth = MaxCharCount * TextMetrics.CharWidth + (MaxCharCount - 1) * (TextMetrics.HSpacing);
//				int TextHeight = NumNewlines * (TextMetrics.CharHeight + TextMetrics.VSpacing) + TextMetrics.CharHeight;
//	
////				color MessageBoxColor = (Editor.Message == SUCCESS_MESSAGE) ? {0.0f, 0.5f, 0.0f, Alpha} : {0.5f, 0.0f, 0.0f, Alpha};
//				color MessageBoxColor, MessageTextColor;
//				color SuccessMessageColor = {0.0f, 0.5f, 0.0f, Alpha};
//				color SuccessTextColor = {0.6f, 1.0f, 0.6f, Alpha};
//				color ErrorMessageColor = {0.5f, 0.0f, 0.0f, Alpha};
//				color ErrorTextColor = {1.0f, 0.6f, 0.6f, Alpha};
//				if(Editor.Message == SUCCESS_MESSAGE)
//				{
//					MessageBoxColor = SuccessMessageColor;
//					MessageTextColor = SuccessTextColor;
//				}
//				else
//				{
//					MessageBoxColor = ErrorMessageColor;
//					MessageTextColor = ErrorTextColor;
//				}
//				int HPadding = 10;
//				int VPadding = 10;
//				int MessageBoxWidth = TextWidth + HPadding * 2;
//				int MessageBoxHeight = TextHeight + VPadding * 2;
//				int MessageBoxX = Editor.WindowWidth / 2 - MessageBoxWidth / 2;
//				int MessageBoxY = Editor.WindowHeight / 2 - MessageBoxHeight / 2;
//				QuadVertices.Count = 0;
//				MakeQuad(&QuadVertices, MessageBoxX, MessageBoxY, MessageBoxWidth, MessageBoxHeight, MessageBoxColor);
//
//				glUseProgram(QuadShader);
//				glUniform1f(glGetUniformLocation(QuadShader, "WindowWidth"), Editor.WindowWidth);
//				glUniform1f(glGetUniformLocation(QuadShader, "WindowHeight"), Editor.WindowHeight);
//				glBindVertexArray(QuadVAO);
//				glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
//				glBufferData(GL_ARRAY_BUFFER, QuadVertices.Count * sizeof(float), QuadVertices.Data, GL_STREAM_DRAW);
//				glDrawArrays(GL_TRIANGLES, 0, QuadVertices.Count / 6);
//
////				color TextColor       = {1.0f, 1.0f, 0.0f, Alpha};
////				color TextColor       = {0.6f, 1.0f, 0.6f, Alpha};
//				color TextColorDarker = {0.0f, 0.0f, 0.0f, Alpha};
//				glUseProgram(TextShader);
//				glUniform1f(glGetUniformLocation(TextShader, "WindowWidth"), Editor.WindowWidth);
//				glUniform1f(glGetUniformLocation(TextShader, "WindowHeight"), Editor.WindowHeight);
//				TextVertices.Count = 0;
//				int TextX = MessageBoxX + HPadding;
//				int TextY = MessageBoxY + VPadding;
//				MakeTextVertices(&TextVertices, MessageText, TextX+2, TextY+2, TextColorDarker, TextMetrics, Chars, NumChars, &Editor);
//				MakeTextVertices(&TextVertices, MessageText, TextX, TextY, MessageTextColor, TextMetrics, Chars, NumChars, &Editor);
//				glBindVertexArray(TextVAO);
//				glBindBuffer(GL_ARRAY_BUFFER, TextVBO);
////				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TextEBO);
////				glBufferData(GL_ARRAY_BUFFER, TextVertices.Count * sizeof(float), TextVertices.Data, GL_STATIC_DRAW);
//				glBufferData(GL_ARRAY_BUFFER, TextVertices.Count * sizeof(float), TextVertices.Data, GL_STREAM_DRAW);
////				glBufferData(GL_ELEMENT_ARRAY_BUFFER, TextIndices.Count * sizeof(unsigned int), TextIndices.Data, GL_STREAM_DRAW);
////				glDrawElements(GL_TRIANGLES, TextIndices.Count, GL_UNSIGNED_INT, 0);
//				glDrawArrays(GL_TRIANGLES, 0, TextVertices.Count / 8);
//			}
//			else
//			{
//				Editor.Message = NONE;
//			}
//		}

//		double T1 = glfwGetTime();
		glfwSwapBuffers(window);
//		double T2 = glfwGetTime();
//		printf("glfwSwapBuffers(): %.3f\n", (T2 - T1) * 1000);

//		T1 = glfwGetTime();
		glfwPollEvents();
//		T2 = glfwGetTime();
//		printf("glfwPollEvents(): %.3f\n", (T2 - T1) * 1000);

//		TextVertices.Count = 0;
//		QuadVertices.Count = 0;
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

void MakeTextVertices(array<float> *Vertices, const char *Text, int OriginalX, int OriginalY, color TextColor, textMetrics TextMetrics, textureCoordinatesOld Chars[], int NumChars, editor *Editor)
{
	int CharWidth = TextMetrics.CharWidth;
	int CharHeight = TextMetrics.CharHeight;
	int XAdvance = CharWidth + TextMetrics.HSpacing;
	int YAdvance = CharHeight + TextMetrics.VSpacing;
	int X = OriginalX;
	int Y = OriginalY;

	for(int i = 0; Text[i]; ++i)
	{
		unsigned char Char = Text[i];
//		if(Char > '~')
//		{
//			printf("NON-ASCII CHARACTER\n");
//		}

		if(Char == '\n')
		{
			Y += YAdvance;
			X = OriginalX;
			continue;
		}

		float X1 = X;                      // upper left
		float Y1 = Y;                      // upper left
		float X2 = X + CharWidth;  // lower right
		float Y2 = Y + CharHeight; // lower right

//		float TextR = 0.0f;
//		float TextG = 0.0f;
//		float TextB = 1.0f;
//		float TextA = 0.5f;

		int Index = Char - ' ';

		float QuadVertices[] = {
			X1, Y1, Chars[Index].X1, Chars[Index].Y1, TextColor.R, TextColor.G, TextColor.B, TextColor.A, /*top-left*/
			X1, Y2, Chars[Index].X1, Chars[Index].Y2, TextColor.R, TextColor.G, TextColor.B, TextColor.A, /*bottom-left*/
			X2, Y2, Chars[Index].X2, Chars[Index].Y2, TextColor.R, TextColor.G, TextColor.B, TextColor.A, /*bottom-right*/
			X2, Y2, Chars[Index].X2, Chars[Index].Y2, TextColor.R, TextColor.G, TextColor.B, TextColor.A, /*bottom-right*/
			X2, Y1, Chars[Index].X2, Chars[Index].Y1, TextColor.R, TextColor.G, TextColor.B, TextColor.A, /*top-right*/
			X1, Y1, Chars[Index].X1, Chars[Index].Y1, TextColor.R, TextColor.G, TextColor.B, TextColor.A, /*top-left*/
		};

		ArrayAdd(Vertices, QuadVertices, COUNT(QuadVertices));

		X += XAdvance;
	}
}

//void MakeTextVertices(array<float> *Vertices, textBuffer *Buffer, textureCoordinatesOld Chars[], int NumChars, editor *Editor)
//{
//	int XAdvance = Editor->CharWidth + Editor->CharSpacing;
//	int YAdvance = Editor->CharHeight + Editor->LineSpacing;
//	int OriginalX = -Editor->ViewportX; // X and Y are going to be in window-coordinates (not in buffer-coordinates)
//	int OriginalY = -Editor->ViewportY;
//	int X = OriginalX;
//	int Y = OriginalY;
//
//	int TotalCharCount, VisibleCharCount;
//	TotalCharCount = 0;
//	VisibleCharCount = 0;
//	double TotalElapsedVisible = 0;
//
//	for(int i = GetStart(Buffer); !IsEnd(Buffer, i); MoveForward(Buffer, &i))
////	for(int i = GetStart(Buffer); !IsEnd(Buffer, i); MoveForwardFast(Buffer, &i))
//	{
//		TotalCharCount += 1;
//		unsigned char Char = GetChar(Buffer, i);
////		if(Char > '~')
////		{
////			printf("NON-ASCII CHARACTER\n");
////		}
//
//		if(Y >= 0 && Y < Editor->WindowHeight && X >= 0 && X < Editor->WindowWidth
//		   && ' ' <= Char && Char <= '~')
//		{
//			VisibleCharCount += 1;
//			double T1 = glfwGetTime();
//
//			float X1 = X;                      // upper left
//			float Y1 = Y;                      // upper left
//			float X2 = X + Editor->CharWidth;  // lower right
//			float Y2 = Y + Editor->CharHeight; // lower right
//
//			float TextR = Editor->TextColor.R;
//			float TextG = Editor->TextColor.G;
//			float TextB = Editor->TextColor.B;
//
//			// if character should have an effect, use effect color
////			for(int j = 0; j < Editor->CharsWithEffect.Count; ++j)
////			{
////				if(Editor->CharsWithEffect.Data[j].Index == i)
////				{
////					charColor *p = &Editor->CharsWithEffect.Data[j];
////					TextR = Lerp(p->Color.R, Editor->TextColor.R, p->Progress);
////					TextG = Lerp(p->Color.G, Editor->TextColor.G, p->Progress);
////					TextB = Lerp(p->Color.B, Editor->TextColor.B, p->Progress);
////					p->Progress += 0.01f;
////					if(p->Progress >= 1.0f)
////					{
////						ArrayRemove(&Editor->CharsWithEffect, j);
////					}
////					break;
////				}
////			}
//
//			int Index = Char - ' ';
//	
//			float QuadVertices[] = {
//				X1, Y1, Chars[Index].X1, Chars[Index].Y1, TextR, TextG, TextB, 1.0f, /*top-left*/
//				X1, Y2, Chars[Index].X1, Chars[Index].Y2, TextR, TextG, TextB, 1.0f, /*bottom-left*/
//				X2, Y2, Chars[Index].X2, Chars[Index].Y2, TextR, TextG, TextB, 1.0f, /*bottom-right*/
//				X2, Y2, Chars[Index].X2, Chars[Index].Y2, TextR, TextG, TextB, 1.0f, /*bottom-right*/
//				X2, Y1, Chars[Index].X2, Chars[Index].Y1, TextR, TextG, TextB, 1.0f, /*top-right*/
//				X1, Y1, Chars[Index].X1, Chars[Index].Y1, TextR, TextG, TextB, 1.0f, /*top-left*/
//			};
//	
//			ArrayAdd(Vertices, QuadVertices, COUNT(QuadVertices));
//
//			double T2 = glfwGetTime();
//			TotalElapsedVisible += T2 - T1;
//		}
//
//		if(Char == '\n')
//		{
//			Y += YAdvance;
//			X = OriginalX;
//		}
//		else
//		{
//			X += XAdvance;
//		}
//	}
////	printf("VisibleCharCount: %d, TotalCharCount: %d\n", VisibleCharCount, TotalCharCount);
////	printf("TotalElapsedOnVisible: %f\n", TotalElapsedVisible);
//}

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

void InitEditor(editor *Editor)
{
////	Editor->FontImage = {
////		.Width = 128,
////		.Height = 64,
////		.CharsInRow = 18,
////		.CharWidth = 5,
////		.CharHeight = 7,
////		.HorizontalSpacing = 2,
////		.VerticalSpacing = 2,
////		.LeftMargin = 1,
////		.TopMargin = 1
////	};
//	fontImage FontImage = {0};
//	FontImage.Width = 128;
//	FontImage.Height = 64;
//	FontImage.CharsInRow = 18;
//	FontImage.CharWidth = 5;
//	FontImage.CharHeight = 7;
//	FontImage.HorizontalSpacing = 2;
//	FontImage.VerticalSpacing = 2;
//	FontImage.LeftMargin = 1;
//	FontImage.TopMargin = 1;
//	Editor->FontImage = FontImage;
//
////	Editor->WindowWidth = 0;
////	Editor->WindowHeight = 0;
////	Editor->ViewportX = 0;
////	Editor->ViewportY = 0;
//	Editor->CharWidth = 3 * Editor->FontImage.CharWidth;
//	Editor->CharHeight = 3 * Editor->FontImage.CharHeight;
//	Editor->CharSpacing = 3;
//	Editor->LineSpacing = 10;
//
//	color TextColor = {1.0f, 1.0f, 1.0f, 1.0f};
//	Editor->TextColor = TextColor;
//	color CursorColor = {0.0f, 1.0f, 0.0f, 1.0f};
//	Editor->CursorColor = CursorColor;
////	color CursorEffectColor = {0.0f, 0.0f, 0.0f, 1.0f};
////	Editor->CursorEffectColor = CursorEffectColor;
////	Editor->CursorEffectProgress = 1.0f;

//	ArrayInit(&Editor->EffectQuads);
//	ArrayInit(&Editor->CharsWithEffect);

	Editor->OpenFile[0] = '\0';

//	Editor->AppWindow = AppWindow;

	Editor->Message = NONE;
	Editor->MessageText[0] = '\0';
	Editor->MessageStartTime = 0.0;

//	bitmapFontImageMetrics IM = {};
//	IM.FilePath = "../charmap-oldschool_white.png";
//	IM.Width = 128;
//	IM.Height = 64;
//	IM.CharsInRow = 18;
//	IM.CharWidth = 5;
//	IM.CharHeight = 7;
//	IM.HSpacing = 2;
//	IM.VSpacing = 2;
//	IM.LMargin = 1;
////	IM.TMargin = 1;

//	bitmapFontImageMetrics IM = {};
//	IM.FilePath = "../intrepid.png";
//	IM.Width = 128;
//	IM.Height = 48;
//	IM.CharsInRow = 16;
//	IM.CharWidth = 8;
//	IM.CharHeight = 8;
//	IM.HSpacing = 0;
//	IM.VSpacing = 0;
//	IM.LMargin = 0;
//	IM.TMargin = 0;

	bitmapFontImageMetrics IM = {};
	IM.FilePath = "../November(8x16).png";
	IM.Width = 760;
	IM.Height = 16;
	IM.CharsInRow = 95;
	IM.CharWidth = 8;
	IM.CharHeight = 16;
	IM.HSpacing = 0;
	IM.VSpacing = 0;
	IM.LMargin = 0;
	IM.TMargin = 0;

	fontConfig Config = {};
	Config.CharWidth = IM.CharWidth;
	Config.CharHeight = IM.CharHeight;
//	Config.CharWidth = IM.CharWidth * 2;
//	Config.CharHeight = IM.CharHeight * 2;
	Config.CharSpacing = 0;
	Config.LineSpacing = 0;
	Config.TabWidth = 3;

	bitmapFont *TheFont = make_bitmap_font(IM, Config);
	if(!TheFont)
	{
		printf("ERROR: FAILED TO MAKE BITMAP FONT!\n");
	}

	//@ dumbass way
	Config.CharWidth = 8;
	Config.CharHeight = 16;
	bitmapFont *Font8x16px = make_bitmap_font(IM, Config);
	Config.CharWidth = 16;
	Config.CharHeight = 32;
	bitmapFont *Font16x32px = make_bitmap_font(IM, Config);
	Config.CharWidth = 32;
	Config.CharHeight = 64;
	bitmapFont *Font32x64px = make_bitmap_font(IM, Config);
	Editor->Font8x16px = Font8x16px;
	Editor->Font16x32px = Font16x32px;
	Editor->Font32x64px = Font32x64px;

	InitTextBuffer(&Editor->TextBuffer);

	char *Contents;
//	const char *FilePath = "testfile.cpp";
	const char *FilePath = "../main.cpp";
	if(!ReadTextFile(FilePath, &Contents))
	{
		fprintf(stderr, "error: failed to read a dropped file: %s\n", FilePath);
		return;
	}
	printf("Read file \"%s\" successfully\n", FilePath);
	textBuffer *Buffer = &Editor->TextBuffer;
	int NumCharsInBuffer = Buffer->OneAfterLast;
	Delete(Buffer, GetStart(Buffer), NumCharsInBuffer);
	if(Insert(Buffer, Contents, GetStart(Buffer)))
	{
		printf("Loaded file successfully\n");
	}
	else
	{
		printf("File too large!\n");
	}

//	int X = 100;
//	int Y = 100;
//	int W = 300;
//	int H = 300;
	color BackgroundColor = {0.25f, 0.25f, 0.25f, 1.0f};
	rect PosAndSize;
	PosAndSize.X = 300;
	PosAndSize.Y = 300;
	PosAndSize.W = 300;
	PosAndSize.H = 300;
//	PosAndSize.W = Editor->WindowWidth;
//	PosAndSize.H = Editor->WindowHeight;
	init_editable_text(&Editor->EditableText, &Editor->TextBuffer, TheFont, BackgroundColor, PosAndSize);
}

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

void DisplayMessage(messageType MessageType, const char *MessageText, editor *Editor)
{
	Editor->Message = MessageType;
	strcpy(Editor->MessageText, MessageText); //@ bounds
	Editor->MessageStartTime = glfwGetTime();
}

// adjust viewport if CharWidth * CharHeight quad at (CharIndex, LineIndex) is not visible, so that it becomes visible again
// returns true if actually adjusted the viewport?
//void AdjustViewportIfNotVisible(editor *Editor, int CharIndex, int LineIndex)
//{
//	// Calculate the cursor's position relative to the viewport
//	int CursorX1 = CharIndex * (Editor->CharWidth + Editor->CharSpacing) - Editor->ViewportX;
//	int CursorY1 = LineIndex * (Editor->CharHeight + Editor->LineSpacing) - Editor->ViewportY;
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


