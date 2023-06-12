#ifndef OPENGL_H
#define OPENGL_H

//@ opengl should probably be part of drawing and not a separate thing

#include <glad/glad.h>

//struct color
//{
//	float R, G, B, A;
//};

GLuint make_shader(const char *VertexSrc, const char *FragmentSrc, const char *GeometrySrc = NULL);
void pass_to_shader(GLuint Shader, const char *Name, int Value); //@ use long/int64_t?
void pass_to_shader(GLuint Shader, const char *Name, float Value); //@ use double?
GLuint make_color_shader();
GLuint make_texture_shader();
GLuint make_text_shader();
GLuint make_text_shader_geometry_style();
GLuint make_SDF_shader();

#endif