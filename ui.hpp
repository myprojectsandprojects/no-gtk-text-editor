#ifndef UI_HPP
#define UI_HPP

#include "text_buffer.hpp"
#include "drawing.hpp"
#include "text_drawing.hpp"

struct editableTextConfig
{
	int X, Y, W, H;
	color TextColor, BackgroundColor, CursorColor, BorderColor;
};

// oneliner
// multiliner
struct editableText
{
	int X, Y, W, H;

	textBuffer *TextBuffer;
	int Cursor;

	int OffsX, OffsY;

	color BackgroundColor, TextColor, CursorColor, BorderColor;

	font *Font;
	bitmapFont *BitmapFont;

	int TabWidth; // Number of characters, assuming we are using a monospace font.
};

void init_editable_text(editableText *EditableText, textBuffer *TextBuffer, font *Font, bitmapFont *BitmapFont, editableTextConfig Config);
void draw_editable_text(editableText *EditableText, shaders *Shaders, bool IsActive);
void adjust_viewport_if_not_visible(editableText *Editable, int Iter);
// void editable_text_on_key_event(editableText *EditableText)

#endif




