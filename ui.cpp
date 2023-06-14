#include "ui.hpp"

void init_editable_text(editableText *EditableText, textBuffer *TextBuffer, bitmapFont *Font, color BackgroundColor, windowXYWH PosAndSize)
{
//	EditableText->X = X;
//	EditableText->Y = Y;
//	EditableText->Width = Width;
//	EditableText->Height = Height;
	EditableText->PosAndSize = PosAndSize;

	EditableText->Cursor = GetStart(TextBuffer);
	EditableText->TextBuffer = TextBuffer;
	EditableText->OffsX = EditableText->OffsY = 0;
	EditableText->BackgroundColor = BackgroundColor;
	EditableText->Font = Font;
}

void draw_editable_text(editableText *EditableText, windowWH WindowSize, GLuint ColorShader)
{
	int X = EditableText->PosAndSize.X;
	int Y = EditableText->PosAndSize.Y;
	int W = EditableText->PosAndSize.W;
	int H = EditableText->PosAndSize.H;

//	array<float> Vertices; ArrayInit(&Vertices);
	//@ Vertices.Count is misleading.

	// MAKE BACKGROUND
//	openglXYWH PosAndSize = transform_window_to_opengl(EditableText->PosAndSize, WindowSize);
//	make_quad(&Vertices, PosAndSize, EditableText->BackgroundColor);
	make_quad(X, Y, W, H, EditableText->BackgroundColor, ColorShader, WindowSize.W, WindowSize.H);

	// MAKE CURSOR
	int CharWidth = EditableText->Font->Config.CharWidth;
	int CharHeight = EditableText->Font->Config.CharHeight;
	int LineSpacing = EditableText->Font->Config.LineSpacing;
	int CharSpacing = EditableText->Font->Config.CharSpacing;
	int TabWidth = EditableText->Font->Config.TabWidth;

	int CursorWidth = CharWidth;
	int CursorHeight = CharHeight;

	int Cursor = EditableText->Cursor;
	textBuffer *Buffer = EditableText->TextBuffer;
	int CursorTBX = GetCharsIntoLine(Buffer, Cursor, TabWidth) * (CharWidth + CharSpacing);
	int CursorTBY = GetLinesIntoBuffer(Buffer, Cursor) * (CharHeight + LineSpacing);

	if(CursorTBX >= EditableText->OffsX && CursorTBX < EditableText->OffsX + EditableText->PosAndSize.W
	&& CursorTBY >= EditableText->OffsY && CursorTBY < EditableText->OffsY + EditableText->PosAndSize.H)
	{
		// Cursor visible
		int CursorX = X + CursorTBX - EditableText->OffsX;
		int CursorY = Y + CursorTBY - EditableText->OffsY;
		color CursorColor = {1.0f, 0.0f, 0.0f, 1.0f};

//		windowXYWH WindowCoord;
//		WindowCoord.X = CursorX;
//		WindowCoord.Y = CursorY;
//		WindowCoord.W = CursorWidth;
//		WindowCoord.H = CursorHeight;
//		openglXYWH OpenglCoord = transform_window_to_opengl(WindowCoord, WindowSize);
//		make_quad(&Vertices, OpenglCoord, CursorColor);
		make_quad(CursorX, CursorY, CursorWidth, CursorHeight, CursorColor, ColorShader, WindowSize.W, WindowSize.H);
	}
//	draw_2d_with_color(&Vertices);


	// MAKE TEXT
	draw_text_buffer(EditableText->TextBuffer, EditableText->Font,
		X, Y, W, H, EditableText->OffsX, EditableText->OffsY, WindowSize.W, WindowSize.H);
}

void AdjustViewportIfNotVisible(editableText *Editable, int Iter)
{
//	int X = Editable->PosAndSize.X;
//	int Y = Editable->PosAndSize.Y;
	int W = Editable->PosAndSize.W;
	int H = Editable->PosAndSize.H;
	int Col = GetCharsIntoLine(Editable->TextBuffer, Iter, Editable->Font->Config.TabWidth);
	int Row = GetLinesIntoBuffer(Editable->TextBuffer, Iter);

	// Iter's position relative to the text buffer
	int CursorX1 = Col * (Editable->Font->Config.CharWidth + Editable->Font->Config.CharSpacing);
	int CursorY1 = Row * (Editable->Font->Config.CharHeight + Editable->Font->Config.LineSpacing);

	// Iter's position relative to the editable text widget
	CursorX1 = CursorX1 - Editable->OffsX;
	CursorY1 = CursorY1 - Editable->OffsY;
	int CursorX2 = CursorX1 + Editable->Font->Config.CharWidth;
	int CursorY2 = CursorY1 + Editable->Font->Config.CharHeight;

	//@ what if the cursor's height is greater than editable's height?
	if(CursorY2 > H)
	{
		Editable->OffsY += CursorY2 - H;
	}
	else if(CursorY1 < 0)
	{
		Editable->OffsY += CursorY1;
	}
	if(CursorX2 > W)
	{
		Editable->OffsX += CursorX2 - W;
	}
	else if(CursorX1 < 0)
	{
		Editable->OffsX += CursorX1;
	}
}


