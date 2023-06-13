#ifndef DRAWING_HPP
#define DRAWING_HPP

#include <glad/glad.h>

#include "lib.h"

// OpenGL coordinates and dimensions
struct openglXYWH
{
	float X, Y, W, H;
};

// window coordinates and dimensions (in pixels)
struct windowXYWH
{
	int X, Y, W, H;
};

//// would like to have this (in window coordinates) and never think about things in terms of opengl coordinates
//struct box
//{
//	int X, Y, W, H;
//};

struct windowWH
{
	int W, H;
};

openglXYWH transform_window_to_opengl(windowXYWH WindowCoord, windowWH WindowSize);

struct color
{
	float R, G, B, A;
};

void make_quad(array<float> *Vertices, openglXYWH PosAndSize, color Color); // pos/size opengl coordinates
//todo: void make_quad(int X, int Y, int W, int H, color Color, editor *Editor); // makes vertices, also draws
void make_quad(array<float> *Vertices, int X, int Y, int W, int H, color Color, windowWH WindowSize); // makes vertices
void draw_2d_with_color(array<float> *Vertices);
void draw_XYRGBA(array<float> *Vertices);

#endif





