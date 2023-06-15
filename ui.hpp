#ifndef UI_HPP
#define UI_HPP

#include "text_buffer.hpp"
#include "drawing.hpp"
#include "text_drawing.hpp"

// oneliner
// multiliner
struct editableText
{
	int X, Y, W, H;
//	rect PosAndSize;

	textBuffer *TextBuffer;
	int Cursor;

	int OffsX, OffsY;

	color BackgroundColor, TextColor, CursorColor;

	bitmapFont *Font;
};

void init_editable_text(editableText *Editable, textBuffer *TextBuffer, bitmapFont *Font, color BackgroundColor, rect PosAndSize);
void draw_editable_text(editableText *EditableText, shaders *Shaders);

#endif




