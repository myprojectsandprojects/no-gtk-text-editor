#ifndef TEXT_DRAWING_HPP
#define TEXT_DRAWING_HPP

#include "lib.h"
#include "text_buffer.hpp"
#include "drawing.hpp"

struct fontConfig
{
	int CharWidth, CharHeight;
	int CharSpacing, LineSpacing;
};

#define NUM_ASCII_CHARS 95

struct glyphInfo
{
	int W, H;
	int XOffs, YOffs;
	float Advance, LSB;
	float TX0, TX1, TY0, TY1;
};

struct font
{
	GLuint Tex;
	int W, H;
	glyphInfo Glyphs[NUM_ASCII_CHARS];
	float Ascent, Descent, LineGap;
};

font *make_font(const char *FilePath, int FontHeight);
//void free_font(font *Font);
void draw_font(font *Font);
void draw_text(const char *Text, int X, int Y, color Color, font *Font, shaders *Shaders);

struct textureCoordinates
{
	float X0, X1, Y0, Y1;
};

struct bitmapFont
{
	fontConfig Config;
//	GLuint TextureAtlas;
	image *TextureAtlas;
	textureCoordinates *TextureCoordinates;
//	GLuint Shader; // Maybe we want this later?
};

struct bitmapFontImageMetrics
{
	const char *FilePath;
	int Width, Height;
	int CharsInRow;
	int CharWidth, CharHeight;
	int HSpacing, VSpacing;
	int LMargin, TMargin;
};

bitmapFont *make_bitmap_font(bitmapFontImageMetrics ImageMetrics, fontConfig Config);
void draw_text(const char *Text, int X, int Y, color Color, bitmapFont *Font, shaders *Shaders);
// maybe draw_text() should take some kind of horizontal extent and once it reaches the end it wraps to a new line?
// also maybe draw_text() should be part of drawing and draw_text_buffer() is a very specialized function that should be part of editable_text?

//void make_text(
//	array<float> *TextVertices,
//	const char *Text,
//	int X, int Y,
//	color TextColor,
//	bitmapFont Font,
//	)
//	// need to render text separately
//
//void make_text(
//	array<float> *TextVertices,
//	textBuffer *Buffer,
//	editor *Editor
//	)
//	// need to render text separately
//
//void flush_text(
//	array<float> *TextVertices,
//)
//
//void draw_text(
//	const char *Text,
//	int X, int Y,
//	color Color,
//	fontAtlas *FontAtlas,
//	GLuint Shader
//	)
//
//struct editableText // textBox
//{
//	textBuffer *Buffer;
//	int Cursor;
//
//	int ViewportX, ViewportY;
//}

#endif


