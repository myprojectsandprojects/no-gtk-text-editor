#ifndef UI_HPP
#define UI_HPP

#include "text_buffer.hpp"
#include "drawing.hpp"
#include "text_drawing.hpp"

#include "lib/array.hpp"


struct editableTextConfig
{
	int X, Y, W, H;
	int BorderThickness;
	color TextColor, BackgroundColor, CursorColor, BorderColor;
};

struct singleLineEditableTextConfig
{
	int X, Y;
	int BorderThickness;
	int W; // # of pixels/characters?
	color TextColor, BackgroundColor, CursorColor, BorderColor;
};

struct editableText
{
	int X, Y, W, H;
	int BorderThickness;

	textBuffer *TextBuffer;
	int Cursor;

	int OffsX, OffsY;

	color BackgroundColor, TextColor, CursorColor, BorderColor;

	font *Font;
	bitmapFont *BitmapFont;

	int TabWidth; // Number of characters, assuming we are using a monospace font.
};

//struct singleLineEditableText {
//	int X, Y, W;
//	int H; // depends on text height
//
//	textBuffer *TextBuffer;
//	int Cursor;
//
//	int OffsX;
//
//	color BackgroundColor, TextColor, CursorColor, BorderColor;
//
//	font *Font;
////	bitmapFont *BitmapFont;
//
//	int TabWidth; // Number of characters, assuming we are using a monospace font.
//};

//struct scrollableListItem
//{
//	const char *Label;
//	//image?
//};

struct scrollableList
{
	int X, Y, W, H;
	int MaxHeight;

	array<const char *> Items;
	int SelectedItem; // -1 -- invalid index (used when no items)

	int OffsY;

	color BGColor, TextColor, SelectedItemColor;

	font *Font;
//	bitmapFont *BitmapFont;
};

//enum fsNodeType {
enum dirEntryType
{
	dirEntryType_UNKNOWN,
	dirEntryType_REGULAR_FILE,
	dirEntryType_DIRECTORY,
};


void init(editableText *EditableText, textBuffer *TextBuffer, font *Font, bitmapFont *BitmapFont, editableTextConfig Config);
void init(editableText *EditableText, textBuffer *TextBuffer, font *Font, bitmapFont *BitmapFont, singleLineEditableTextConfig Config);
void draw(editableText *Editable, shaders *Shaders, bool IsActive); //or singleLineEditableText
//void draw(singleLineEditableText *EditableText, shaders *Shaders, bool IsActive);
//void adjust_viewport_if_not_visible(singleLineEditableText *Editable, int Iter);
void adjust_viewport_if_not_visible(editableText *Editable, int Iter);
// void editable_text_on_key_event(editableText *EditableText)

scrollableList make_scrollableList(int X, int Y, int W, int MaxHeight, font *Font, color BGColor, color SelectedItemColor, color TextColor);
void draw(scrollableList *List, array<dirEntryType> *DirEntryTypes, shaders *Shaders);
void append_item(scrollableList *List, const char *Item);
void removeAllItems(scrollableList *List);

#endif




