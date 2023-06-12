#include "editable_text.hpp"

void init_editable_text(editableText *EditableText, textBuffer *TextBuffer, bitmapFont *Font, color BackgroundColor,
	int X, int Y, int Width, int Height)
{
	EditableText->X = X;
	EditableText->Y = Y;
	EditableText->Width = Width;
	EditableText->Height = Height;

	EditableText->Cursor = GetStart(TextBuffer);
	EditableText->TextBuffer = TextBuffer;
	EditableText->OffsX = EditableText->OffsY = 0;
	EditableText->BackgroundColor = BackgroundColor;
	EditableText->Font = Font;
}

void draw_editable_text(editableText *EditableText, windowDimensions Window)
{
	int X = EditableText->X;
	int Y = EditableText->Y;
	int W = EditableText->Width;
	int H = EditableText->Height;

	array<float> Vertices; ArrayInit(&Vertices);
	//@ Vertices.Count is misleading.

	// MAKE BACKGROUND
	float _X, _Y, _W, _H;
	transform_window_to_opengl(X, Y, W, H, &_X, &_Y, &_W, &_H);
	make_quad(&Vertices, _X, _Y, _W, _H, EditableText->BackgroundColor);

	// MAKE CURSOR
	int CharWidth = EditableText->Font->Config.CharWidth;
	int CharHeight = EditableText->Font->Config.CharHeight;
	int LineSpacing = EditableText->Font->Config.LineSpacing;
	int CharSpacing = EditableText->Font->Config.CharSpacing;

	int CursorWidth = CharWidth;
	int CursorHeight = CharHeight;
	printf("cursor width: %d, cursor height: %d\n", CursorWidth, CursorHeight);

	int Cursor = EditableText->Cursor;
	textBuffer *Buffer = &Editor->TextBuffer;
	int CursorTBX = GetCharsIntoLine(Buffer, Cursor) * (CharWidth + CharSpacing);
	int CursorTBY = GetLinesIntoBuffer(Buffer, Cursor) * (CharHeight + LineSpacing);
	printf("cursor tbx: %d, cursor tby: %d\n", CursorTBX, CursorTBY);

	if(CursorTBX >= EditableText->OffsX && CursorTBX < EditableText->OffsX + W
	&& CursorTBY >= EditableText->OffsY && CursorTBY < EditableText->OffsY + H)
	{
		// Cursor visible
		int CursorX = X + CursorTBX - EditableText->OffsX;
		int CursorY = Y + CursorTBY - EditableText->OffsY;
		color CursorColor = {1.0f, 0.0f, 0.0f, 1.0f};
		make_quad(&Vertices, CursorX, CursorY, CursorWidth, CursorHeight, CursorColor, Editor);
	}

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
	glBufferData(GL_ARRAY_BUFFER, Vertices.Count * sizeof(float), Vertices.Data, GL_STREAM_DRAW);

	assert((Vertices.Count % 6) == 0);
	int NumVertices = Vertices.Count / 6;
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);


	// MAKE TEXT
	draw_text_buffer(EditableText->TextBuffer, EditableText->Font,
		X, Y, W, H, EditableText->OffsX, EditableText->OffsY, Editor->WindowWidth, Editor->WindowHeight);
}

void AdjustViewportIfNotVisible(editableText *Editable, int Iter)
{
	int Col = GetCharsIntoLine(Editable->TextBuffer, Iter);
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
	if(CursorY2 > Editable->Height)
	{
		Editable->OffsY += CursorY2 - Editable->Height;
	}
	else if(CursorY1 < 0)
	{
		Editable->OffsY += CursorY1;
	}
	if(CursorX2 > Editable->Width)
	{
		Editable->OffsX += CursorX2 - Editable->Width;
	}
	else if(CursorX1 < 0)
	{
		Editable->OffsX += CursorX1;
	}
}




