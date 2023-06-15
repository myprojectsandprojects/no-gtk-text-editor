#ifndef DRAWING_HPP
#define DRAWING_HPP

#include <glad/glad.h>

#include "lib.h"

struct rect
{
	int X, Y, W, H;
};

struct color
{
	float R, G, B, A;
};

struct shaders
{
	int WindowWidth, WindowHeight;
	GLuint ColorShader;
	GLuint TextShader;
};

GLuint make_shader(const char *VertexSrc, const char *FragmentSrc, const char *GeometrySrc = NULL);
void pass_to_shader(GLuint Shader, const char *Name, int Value);
void pass_to_shader(GLuint Shader, const char *Name, float Value);
//GLuint make_color_shader();
GLuint make_color_shader_with_transform();
//GLuint make_text_shader();
GLuint make_text_shader_with_transform();
GLuint make_texture_shader();
GLuint make_text_shader_geometry_style();
GLuint make_SDF_shader();

void init_shaders(int WindowWidth, int WindowHeight);

//GLuint CreateShader(const char *VertexShaderFile, const char *FragmentShaderFile);
//void make_quad(array<float> *Vertices, openglXYWH PosAndSize, color Color); // pos/size opengl coordinates
//void make_quad(array<float> *Vertices, int X, int Y, int W, int H, color Color, windowWH WindowSize); // makes vertices
//void draw_2d_with_color(array<float> *Vertices);
//void draw_XYRGBA(array<float> *Vertices);

//void make_quad(int X, int Y, int W, int H, color Color, int WindowWidth, int WindowHeight);
void make_quad(int X, int Y, int W, int H, color Color);

#endif





