#ifndef DRAWING_HPP
#define DRAWING_HPP

#include <glad/glad.h>

//#include "lib.h"
#include "lib/lib.hpp"

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
	GLuint TextureShader;
};

struct image
{
//	const char *FilePath;
//	unsigned char *Data;
	GLuint Tex;
	int W, H, NumChannels;
};
image *make_image(unsigned char *Data, int Width, int Height, int NumChannels);
image *make_image(const char *FilePath);

GLuint make_shader(const char *VertexSrc, const char *FragmentSrc, const char *GeometrySrc = NULL);
void pass_to_shader(GLuint Shader, const char *Name, int Value);
void pass_to_shader(GLuint Shader, const char *Name, float Value);
//GLuint make_color_shader();
GLuint make_color_shader_with_transform();
//GLuint make_text_shader();
GLuint make_text_shader_with_transform();
GLuint make_texture_shader();
GLuint make_texture_shader_with_transform();
GLuint make_text_shader_geometry_style();
GLuint make_SDF_shader();

void init_shaders(int WindowWidth, int WindowHeight);

//GLuint CreateShader(const char *VertexShaderFile, const char *FragmentShaderFile);
//void make_quad(array<float> *Vertices, openglXYWH PosAndSize, color Color); // pos/size opengl coordinates
//void make_quad(array<float> *Vertices, int X, int Y, int W, int H, color Color, windowWH WindowSize); // makes vertices
//void draw_2d_with_color(array<float> *Vertices);
//void draw_XYRGBA(array<float> *Vertices);

//void make_quad(int X, int Y, int W, int H, color Color, int WindowWidth, int WindowHeight);
void draw_quad(int X, int Y, int W, int H, color Color);
void draw_quad(int X, int Y, int W, int H, GLuint Texture, bool Flip = false);

#endif





