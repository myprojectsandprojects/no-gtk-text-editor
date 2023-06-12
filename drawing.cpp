#include "drawing.hpp"

#include <assert.h>

openglXYWH transform_window_to_opengl(windowXYWH WindowCoord, windowWH WindowSize)
{
	float PixelWidth = 2.0f / WindowSize.W;
	float PixelHeight = 2.0f / WindowSize.H;

	openglXYWH openglCoord;
	openglCoord.X = WindowCoord.X * PixelWidth - 1.0f;
	openglCoord.Y = 1.0f - WindowCoord.Y * PixelHeight;
	openglCoord.W = WindowCoord.W * PixelWidth;
	openglCoord.H = WindowCoord.H * PixelHeight;

	return openglCoord;
}

void make_quad(array<float> *Vertices, openglXYWH PosAndSize, color Color)
{
	float X0 = PosAndSize.X;
	float X1 = PosAndSize.X + PosAndSize.W;
	float Y0 = PosAndSize.Y;
	float Y1 = PosAndSize.Y - PosAndSize.H;

	float QuadVertices[] = {
		/* upper-left*/  X0, Y0, Color.R, Color.G, Color.B, Color.A,
		/* upper-right*/ X1, Y0, Color.R, Color.G, Color.B, Color.A,
		/* lower-right*/ X1, Y1, Color.R, Color.G, Color.B, Color.A,

		/* lower-right*/ X1, Y1, Color.R, Color.G, Color.B, Color.A,
		/* lower-left*/  X0, Y1, Color.R, Color.G, Color.B, Color.A,
		/* upper-left*/  X0, Y0, Color.R, Color.G, Color.B, Color.A,
	};
	ArrayAdd(Vertices, QuadVertices, COUNT(QuadVertices));
}

void draw_2d_with_color(array<float> *Vertices)
{
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0); // pos
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (2 * sizeof(float))); // color
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

//	glBindVertexArray(QuadVAO);
//	glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
	glBufferData(GL_ARRAY_BUFFER, Vertices->Count * sizeof(float), Vertices->Data, GL_STREAM_DRAW);

	assert((Vertices->Count % 6) == 0);
	int NumVertices = Vertices->Count / 6;
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

// Transformation of coordinates/dimensions to OpenGL coordinates/dimensions is done here, not in the shader.
void make_quad(array<float> *Vertices, int X, int Y, int W, int H, color Color, windowWH WindowSize)
{
	float PixelWidth = 2.0f / WindowSize.W;
	float PixelHeight = 2.0f / WindowSize.H;

	float _X = X * PixelWidth - 1.0f;
	float _Y = 1.0f - Y * PixelHeight;
	float _W = W * PixelWidth;
	float _H = H * PixelHeight;

	float X0 = _X;
	float X1 = _X + _W;
	float Y0 = _Y;
	float Y1 = _Y - _H;

	float QuadVertices[] = {
		/* upper-left*/  X0, Y0, Color.R, Color.G, Color.B, Color.A,
		/* upper-right*/ X1, Y0, Color.R, Color.G, Color.B, Color.A,
		/* lower-right*/ X1, Y1, Color.R, Color.G, Color.B, Color.A,

		/* lower-right*/ X1, Y1, Color.R, Color.G, Color.B, Color.A,
		/* lower-left*/  X0, Y1, Color.R, Color.G, Color.B, Color.A,
		/* upper-left*/  X0, Y0, Color.R, Color.G, Color.B, Color.A,
	};
	ArrayAdd(Vertices, QuadVertices, COUNT(QuadVertices));
}



