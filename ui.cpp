#include "ui.hpp"
#include <assert.h>

void init_editable_text(editableText *EditableText, textBuffer *TextBuffer, font *Font, bitmapFont *BitmapFont, editableTextConfig Config)
{
	EditableText->OffsX = 0;
	EditableText->OffsY = 0;

	EditableText->X = Config.X;
	EditableText->Y = Config.Y;
	EditableText->W = Config.W;
	EditableText->H = Config.H;
	EditableText->TextColor = Config.TextColor;
	EditableText->BackgroundColor = Config.BackgroundColor;
	EditableText->CursorColor = Config.CursorColor;
	EditableText->BorderColor = Config.BorderColor;

	EditableText->TextBuffer = TextBuffer;
	EditableText->Cursor = GetStart(TextBuffer);

	EditableText->Font = Font;
	EditableText->BitmapFont = BitmapFont;

	EditableText->TabWidth = 3; //@ hack
}

static void draw_text(editableText *EditableText, shaders *Shaders)
{
	textBuffer *TextBuffer = EditableText->TextBuffer;
	font *Font = EditableText->Font;
//	int OffsX = EditableText->OffsX;
//	int OffsY = EditableText->OffsY;
//	int X = EditableText->X;
//	int Y = EditableText->Y;
//	int W = EditableText->W;
//	int H = EditableText->H;
	color Color = {1.0f, 1.0f, 1.0f, 1.0f};

	int AdvanceX = (int)Font->Glyphs[0].Advance; // We assume it's a monospaced font. So all glyphs should have the same advance.
	float AbsAscent = (Font->Ascent < 0) ? -Font->Ascent : Font->Ascent;
	float AbsDescent = (Font->Descent < 0) ? -Font->Descent : Font->Descent;
	int AdvanceY = ((int)(AbsAscent + AbsDescent));

//	int PenX = -OffsX;
//	int PenY = -OffsY;

	// These are in pixels, relative to the text buffer (char's in the text buffer laid out in 2D).
	// We want the first char's left and top edges to be positioned at (0,0), but X and Y refer to current point and baseline.
	int X = 0;
	int Y = AbsAscent;

	int NumFloatsPerVertex = 8;
	int NumVerticesPerChar = 6;
	int NumFloatsPerChar = NumVerticesPerChar * NumFloatsPerVertex;
	int NumBytesPerChar = NumFloatsPerChar * sizeof(float);

	//@ Just assume we render all characters in the text buffer. This is clearly a no-no in the long run.
	// Maybe calculate some kind of estimate that is not going to be exceeded based on the area we render text on and character size?
	int InitiallyAllocatedMemory = TextBuffer->OneAfterLast * NumBytesPerChar;

	GLuint VB;
	glGenBuffers(1, &VB);
	glBindBuffer(GL_ARRAY_BUFFER, VB);
	glBufferData(GL_ARRAY_BUFFER, InitiallyAllocatedMemory, NULL, GL_STREAM_DRAW);

	float *Ptr = (float *) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	int VerticesCount = 0;

	for(int i = GetStart(TextBuffer); !IsEnd(TextBuffer, i); MoveForward(TextBuffer, &i))
	{
		char Ch = GetChar(TextBuffer, i);
//		printf("%c\n", Ch);

		if(Ch == '\n')
		{
			Y += AdvanceY;
			X = 0;
			continue;
		}

		if(Ch == '\t')
		{
			X += EditableText->TabWidth * AdvanceX;
			continue;
		}

//		if(Ch == '\n'/* || PenX > WindowWidth*/)
//		{
////			PenY += 16;
//			PenY += Font->Config.CharHeight + Font->Config.LineSpacing;
//			if(PenY > H)
//			{
//				break;
//			}
//			PenX = -OffsX;
//
//			continue;
//		}
//		if(Ch == '\t')
//		{
//			PenX += Font->Config.TabWidth * (Font->Config.CharWidth + Font->Config.CharSpacing);
//			continue;
//		}

		if(EditableText->OffsX <= X && X < EditableText->OffsX + EditableText->W
			&& EditableText->OffsY <= Y && Y < EditableText->OffsY + EditableText->H)
		{
			int Index = Ch - ' ';
			assert(0 <= Index && Index < 95);
//			textureCoordinates TexCoord = Font->TextureCoordinates[Index];
			glyphInfo Glyph = Font->Glyphs[Index];

//			int _X = X + PenX;
//			int _Y = Y + PenY;
//			int _W = Font->Config.CharWidth;
//			int _H = Font->Config.CharHeight;

			int _X = EditableText->X + X + Glyph.XOffs - EditableText->OffsX;
			int _Y = EditableText->Y + Y + Glyph.YOffs - EditableText->OffsY;
			int _W = Glyph.W;
			int _H = Glyph.H;

			float X0 = _X;
			float X1 = _X + _W;
			float Y0 = _Y;
			float Y1 = _Y + _H;

			// upper-left
			Ptr[0]=X0; Ptr[1]=Y0; Ptr[2]=Glyph.TX0; Ptr[3]=Glyph.TY0; Ptr[4]=Color.R; Ptr[5]=Color.G; Ptr[6]=Color.B; Ptr[7]=Color.A;
	
			// upper-right
			Ptr[8]=X1; Ptr[9]=Y0; Ptr[10]=Glyph.TX1; Ptr[11]=Glyph.TY0; Ptr[12]=Color.R; Ptr[13]=Color.G; Ptr[14]=Color.B; Ptr[15]=Color.A;
	
			// lower-left
			Ptr[16]=X0; Ptr[17]=Y1; Ptr[18]=Glyph.TX0; Ptr[19]=Glyph.TY1; Ptr[20]=Color.R; Ptr[21]=Color.G; Ptr[22]=Color.B; Ptr[23]=Color.A;
	
			// lower-left
			Ptr[24]=X0; Ptr[25]=Y1; Ptr[26]=Glyph.TX0; Ptr[27]=Glyph.TY1; Ptr[28]=Color.R; Ptr[29]=Color.G; Ptr[30]=Color.B; Ptr[31]=Color.A;
	
			// lower-right
			Ptr[32]=X1; Ptr[33]=Y1; Ptr[34]=Glyph.TX1; Ptr[35]=Glyph.TY1; Ptr[36]=Color.R; Ptr[37]=Color.G; Ptr[38]=Color.B;  Ptr[39]=Color.A;
	
			// upper-right
			Ptr[40]=X1; Ptr[41]=Y0; Ptr[42]=Glyph.TX1; Ptr[43]=Glyph.TY0; Ptr[44]=Color.R; Ptr[45]=Color.G; Ptr[46]=Color.B; Ptr[47]=Color.A;

////		memcpy(Ptr, Vertices, sizeof(Vertices));
			Ptr += NumFloatsPerChar;
			VerticesCount += NumVerticesPerChar;
		}
		
//		PenX += Font->Config.CharWidth;
		X += AdvanceX;
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);

	// Do OpenGL rendering stuff.

	glBindTexture(GL_TEXTURE_2D, Font->Tex);
	glUseProgram(Shaders->TextShader);
	pass_to_shader(Shaders->TextShader, "WindowWidth", (float)Shaders->WindowWidth);
	pass_to_shader(Shaders->TextShader, "WindowHeight", (float)Shaders->WindowHeight);

	GLuint VertexArray;
	glGenVertexArrays(1, &VertexArray);
	glBindVertexArray(VertexArray);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *) (sizeof(float) * 2));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *) (sizeof(float) * 4));
	glEnableVertexAttribArray(2);

	glDrawArrays(GL_TRIANGLES, 0, VerticesCount);

	glDeleteBuffers(1, &VB);
	glDeleteVertexArrays(1, &VertexArray);

//	glBindTexture(GL_TEXTURE_2D, 0); // bind default texture (?)
}

static void draw_text_bitmap_font(editableText *EditableText, shaders *Shaders)
{
	textBuffer *TextBuffer = EditableText->TextBuffer;
	bitmapFont *Font = EditableText->BitmapFont;
	int OffsX = EditableText->OffsX;
	int OffsY = EditableText->OffsY;
	int X = EditableText->X;
	int Y = EditableText->Y;
	int W = EditableText->W;
	int H = EditableText->H;


//	color Color = {1.0f, 1.0f, 1.0f, 1.0f};
	color Color = EditableText->TextColor;

	int PenX = -OffsX;
	int PenY = -OffsY;

	int NumFloatsPerVertex = 8;
	int NumVerticesPerChar = 6;
	int NumFloatsPerChar = NumVerticesPerChar * NumFloatsPerVertex;
	int NumBytesPerChar = NumFloatsPerChar * sizeof(float);

	//@ Just assume we render all characters in the text buffer. This is clearly a no-no in the long run.
	// Maybe calculate some kind of estimate that is not going to be exceeded based on the area we render text on and character size?
	int InitiallyAllocatedMemory = TextBuffer->OneAfterLast * NumBytesPerChar;

	GLuint VB;
	glGenBuffers(1, &VB);
	glBindBuffer(GL_ARRAY_BUFFER, VB);
	glBufferData(GL_ARRAY_BUFFER, InitiallyAllocatedMemory, 0, GL_DYNAMIC_DRAW);

	float *Ptr = (float *) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	int VerticesCount = 0;

	for(int i = GetStart(TextBuffer); !IsEnd(TextBuffer, i); MoveForward(TextBuffer, &i))
	{
		char Ch = GetChar(TextBuffer, i);
//		printf("%c\n", Ch);

		if(Ch == '\n'/* || PenX > WindowWidth*/)
		{
//			PenY += 16;
			PenY += Font->Config.CharHeight + Font->Config.LineSpacing;
			if(PenY > H)
			{
				break;
			}
			PenX = -OffsX;

			continue;
		}
		if(Ch == '\t')
		{
			PenX += EditableText->TabWidth * (Font->Config.CharWidth + Font->Config.CharSpacing);
			continue;
		}

		if(0 <= PenX && PenX <= W && 0 <= PenY && PenY + Font->Config.CharHeight <= H)
		{
			int Index = Ch - ' ';
			textureCoordinates TexCoord = Font->TextureCoordinates[Index];

			int _X = X + PenX;
			int _Y = Y + PenY;
			int _W = Font->Config.CharWidth;
			int _H = Font->Config.CharHeight;

			float X0 = _X;
			float X1 = _X + _W;
			float Y0 = _Y;
			float Y1 = _Y + _H;

			// flip the y-axis
			float Temp = TexCoord.Y0;
			TexCoord.Y0 = TexCoord.Y1;
			TexCoord.Y1 = Temp;

			// upper-left
			Ptr[0]=X0; Ptr[1]=Y0; Ptr[2]=TexCoord.X0; Ptr[3]=TexCoord.Y1; Ptr[4]=Color.R; Ptr[5]=Color.G; Ptr[6]=Color.B; Ptr[7]=Color.A;
	
			// upper-right
			Ptr[8]=X1; Ptr[9]=Y0; Ptr[10]=TexCoord.X1; Ptr[11]=TexCoord.Y1; Ptr[12]=Color.R; Ptr[13]=Color.G; Ptr[14]=Color.B; Ptr[15]=Color.A;
	
			// lower-left
			Ptr[16]=X0; Ptr[17]=Y1; Ptr[18]=TexCoord.X0; Ptr[19]=TexCoord.Y0; Ptr[20]=Color.R; Ptr[21]=Color.G; Ptr[22]=Color.B; Ptr[23]=Color.A;
	
			// lower-left
			Ptr[24]=X0; Ptr[25]=Y1; Ptr[26]=TexCoord.X0; Ptr[27]=TexCoord.Y0; Ptr[28]=Color.R; Ptr[29]=Color.G; Ptr[30]=Color.B; Ptr[31]=Color.A;
	
			// lower-right
			Ptr[32]=X1; Ptr[33]=Y1; Ptr[34]=TexCoord.X1; Ptr[35]=TexCoord.Y0; Ptr[36]=Color.R; Ptr[37]=Color.G; Ptr[38]=Color.B;  Ptr[39]=Color.A;
	
			// upper-right
			Ptr[40]=X1; Ptr[41]=Y0; Ptr[42]=TexCoord.X1; Ptr[43]=TexCoord.Y1; Ptr[44]=Color.R; Ptr[45]=Color.G; Ptr[46]=Color.B; Ptr[47]=Color.A;

////		memcpy(Ptr, Vertices, sizeof(Vertices));
			Ptr += NumFloatsPerChar;
			VerticesCount += NumVerticesPerChar;
		}
		
		PenX += Font->Config.CharWidth;
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);

	// Do OpenGL rendering stuff.

//	glBindTexture(GL_TEXTURE_2D, Font->TextureAtlas);
	glBindTexture(GL_TEXTURE_2D, Font->TextureAtlas->Tex);
	glUseProgram(Shaders->TextShader);
	pass_to_shader(Shaders->TextShader, "WindowWidth", (float)Shaders->WindowWidth);
	pass_to_shader(Shaders->TextShader, "WindowHeight", (float)Shaders->WindowHeight);

	GLuint VertexArray;
	glGenVertexArrays(1, &VertexArray);
	glBindVertexArray(VertexArray);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *) (sizeof(float) * 2));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *) (sizeof(float) * 4));
	glEnableVertexAttribArray(2);

	int NumVertices = VerticesCount;
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	glDeleteBuffers(1, &VB);
	glDeleteVertexArrays(1, &VertexArray);

//	glBindTexture(GL_TEXTURE_2D, 0); // bind default texture (?)
}

void draw_editable_text(editableText *EditableText, shaders *Shaders, bool IsActive)
{
	int X = EditableText->X;
	int Y = EditableText->Y;
	int W = EditableText->W;
	int H = EditableText->H;

	// BACKGROUND
	draw_quad(X, Y, W, H, EditableText->BackgroundColor);

	// BORDER
	int BorderThickness = 9;
	color ActiveColor = {1.0f, 0.0f, 0.0f, 1.0f};
	color BorderColor = ((IsActive) ? (ActiveColor) : (EditableText->BorderColor));
	draw_quad(X, Y - BorderThickness, W, BorderThickness, BorderColor);
	draw_quad(X, Y + H, W, BorderThickness, BorderColor);
	draw_quad(X - BorderThickness, Y - BorderThickness, BorderThickness, 2*BorderThickness + H, BorderColor);
	draw_quad(X + W, Y - BorderThickness, BorderThickness, 2*BorderThickness + H, BorderColor);

	// CURSOR

	// bitmap font:
//	int CharWidth = EditableText->BitmapFont->Config.CharWidth;
//	int CharHeight = EditableText->BitmapFont->Config.CharHeight;
//	int LineSpacing = EditableText->BitmapFont->Config.LineSpacing;
//	int CharSpacing = EditableText->BitmapFont->Config.CharSpacing;
//	int TabWidth = EditableText->BitmapFont->Config.TabWidth;

	// non-bitmap font:
	int CharWidth = EditableText->Font->Glyphs[0].Advance;
	int CharHeight = (int)(EditableText->Font->Ascent - EditableText->Font->Descent);
	int LineSpacing = 0;
	int CharSpacing = 0;
	int TabWidth = EditableText->TabWidth;

	textBuffer *Buffer = EditableText->TextBuffer;
	int Cursor = EditableText->Cursor;
	int CursorTBX = GetCharsIntoLine(Buffer, Cursor, TabWidth) * (CharWidth + CharSpacing);
	int CursorTBY = GetLinesIntoBuffer(Buffer, Cursor) * (CharHeight + LineSpacing);

	if(CursorTBX >= EditableText->OffsX && CursorTBX < EditableText->OffsX + W
		&& CursorTBY >= EditableText->OffsY && CursorTBY < EditableText->OffsY + H)
	{
		// Cursor visible
		int CursorX = X + CursorTBX - EditableText->OffsX;
		int CursorY = Y + CursorTBY - EditableText->OffsY;
		int CursorW = CharWidth;
		int CursorH = CharHeight;
		color CursorColor = EditableText->CursorColor;
		draw_quad(CursorX, CursorY, CursorW, CursorH, CursorColor);
	}

	// TEXT
//	draw_text_bitmap_font(EditableText, Shaders);
	draw_text(EditableText, Shaders);
}

void adjust_viewport_if_not_visible(editableText *Editable, int Iter)
{
	int Col = GetCharsIntoLine(Editable->TextBuffer, Iter, Editable->TabWidth);
	int Row = GetLinesIntoBuffer(Editable->TextBuffer, Iter);

	// Iter's position relative to the text buffer
	// bitmap font version:
//	int CursorX1 = Col * (Editable->BitmapFont->Config.CharWidth + Editable->BitmapFont->Config.CharSpacing);
//	int CursorY1 = Row * (Editable->BitmapFont->Config.CharHeight + Editable->BitmapFont->Config.LineSpacing);
	// bitmap non-font version:
	int CharWidth = Editable->Font->Glyphs[0].Advance;
	int CharHeight = (int)(Editable->Font->Ascent - Editable->Font->Descent);
	int LineSpacing = 0;
	int CharSpacing = 0;
	int CursorX1 = Col * (CharWidth + CharSpacing);
	int CursorY1 = Row * (CharHeight + LineSpacing);

	// Iter's position relative to the editable text widget
	CursorX1 = CursorX1 - Editable->OffsX;
	CursorY1 = CursorY1 - Editable->OffsY;

//	// bitmap font version:
//	int CursorX2 = CursorX1 + Editable->BitmapFont->Config.CharWidth;
//	int CursorY2 = CursorY1 + Editable->BitmapFont->Config.CharHeight;
	// non-bitmap font version:
	int CursorX2 = CursorX1 + CharWidth;
	int CursorY2 = CursorY1 + CharHeight;

	int W = Editable->W;
	int H = Editable->H;

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


