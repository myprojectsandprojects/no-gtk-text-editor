
#include "text_drawing.hpp"
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

//void make_font(font *Font, const char *FilePath)
font *make_font(const char *FilePath, int FontHeight)
{
	u8 *Contents;
	size_t NumBytes;
	if(!ReadFile(FilePath, &Contents, &NumBytes))
	{
		fprintf(stderr, "error: make_font(): ReadFile(): %s\n", FilePath);
		return NULL;
	}
	fprintf(stderr, "Successfully read %s (%ld)\n", FilePath, NumBytes);

	stbtt_fontinfo Font;
	if(!stbtt_InitFont(&Font, Contents, stbtt_GetFontOffsetForIndex(Contents, 0)))
	{
		fprintf(stderr, "error: stbtt_InitFont()\n");
		return NULL;
	}

	float Scale = stbtt_ScaleForPixelHeight(&Font, FontHeight);

	// What to display if we encounter a codepoint we have no glyph for?
	int NumCh = '~' - ' ' + 1;
	struct
	{
		unsigned char *Bitmap;
		int Width, Height, XOffs, YOffs;
		float Advance, LSB;
	} Glyphs[NumCh];

//	const char *GlyphsWeWant = "abc";
	
	int AtlasWidth = 0;
	int AtlasHeight = 0;
	for(int i = 0; i < NumCh; ++i)
	{
		char Ch = i + ' ';
		int GlyphWidth, GlyphHeight, XOffs, YOffs;
		unsigned char *GlyphBitmap =
			stbtt_GetCodepointBitmap(&Font, Scale, Scale, Ch, &GlyphWidth, &GlyphHeight, &XOffs, &YOffs);

		int Advance, LSB;
		stbtt_GetCodepointHMetrics(&Font, Ch, &Advance, &LSB);
// leftSideBearing is the offset from the current horizontal position to the left edge of the character
// advanceWidth is the offset from the current horizontal position to the next horizontal position
//   these are expressed in unscaled coordinates

		Glyphs[i].Bitmap = GlyphBitmap;
		Glyphs[i].Width = GlyphWidth;
		Glyphs[i].Height = GlyphHeight;
		Glyphs[i].XOffs = XOffs;
		Glyphs[i].YOffs = YOffs;
		Glyphs[i].Advance = Advance * Scale;
		Glyphs[i].LSB = LSB * Scale;

		if(GlyphHeight > AtlasHeight) AtlasHeight = GlyphHeight;
		AtlasWidth += GlyphWidth;
	}
//	for(int i = 0; i < 3; ++i)
//	{
//		printf("width: %d, height: %d\n", Glyphs[i].Width, Glyphs[i].Height);
//	}
//	printf("Atlas width: %d, height: %d\n", AtlasWidth, AtlasHeight);

	GLuint Tex;
	glGenTextures(1, &Tex);
	glBindTexture(GL_TEXTURE_2D, Tex);

	// What to do when texture coordinates are outside of the texture:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// What to do when the texture is minified/magnified:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//@ Does it have to be GL_RGB?
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, AtlasWidth, AtlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

	font *ResultFont = (font *)malloc(sizeof(font));

	int Ascent; int Descent; int LineGap;
	stbtt_GetFontVMetrics(&Font, &Ascent, &Descent, &LineGap);
	printf("ascent: %.1f, descent: %.1f, line gap: %.1f\n", Ascent*Scale, Descent*Scale, LineGap*Scale);

	ResultFont->Ascent = Ascent * Scale;
	ResultFont->Descent = Descent * Scale;
	ResultFont->LineGap = LineGap * Scale;

	int TexOffsetX = 0;
	for(int i = 0; i < NumCh; ++i)
	{
//		printf("width: %d, height: %d\n", Glyphs[i].Width, Glyphs[i].Height);
		glTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			TexOffsetX,                 // xoffset
//			Height - GlyphSlot->bitmap.rows,                       // yoffset
			0,                       // yoffset
			Glyphs[i].Width,  // width
			Glyphs[i].Height, // height
			GL_RED,           // format
			GL_UNSIGNED_BYTE, // type
			Glyphs[i].Bitmap  // data
		);

		free(Glyphs[i].Bitmap);

		ResultFont->Glyphs[i].W = Glyphs[i].Width;
		ResultFont->Glyphs[i].H = Glyphs[i].Height;
		ResultFont->Glyphs[i].TX0 = TexOffsetX / (float)AtlasWidth;
		ResultFont->Glyphs[i].TX1 = ResultFont->Glyphs[i].TX0 + Glyphs[i].Width / (float)AtlasWidth;
//		ResultFont->Glyphs[i].TY0 = 1.0f;
//		ResultFont->Glyphs[i].TY1 = ResultFont->Glyphs[i].TY0 - Glyphs[i].Height / (float)AtlasHeight;
		ResultFont->Glyphs[i].TY0 = 0.0f;
		ResultFont->Glyphs[i].TY1 = Glyphs[i].Height / (float)AtlasHeight;
		ResultFont->Glyphs[i].XOffs = Glyphs[i].XOffs;
		ResultFont->Glyphs[i].YOffs = Glyphs[i].YOffs;
		ResultFont->Glyphs[i].Advance = Glyphs[i].Advance;
		ResultFont->Glyphs[i].LSB = Glyphs[i].LSB;
//		printf("%d (%c): width: %d, height: %d, advance: %f, lsb: %f, xoffs: %d\n", i, (' ' + i),
//			ResultFont->Glyphs[i].W, ResultFont->Glyphs[i].H,
//			ResultFont->Glyphs[i].Advance, ResultFont->Glyphs[i].LSB, ResultFont->Glyphs[i].XOffs);

		TexOffsetX += Glyphs[i].Width;
	}

//	image *GlyphImage = make_image(GlyphBitmap, GlyphWidth, GlyphHeight, 1);

	ResultFont->Tex = Tex;
	ResultFont->W = AtlasWidth;
	ResultFont->H = AtlasHeight;

	free(Contents);
//	free(GlyphImage);

	return ResultFont;
}

void draw_font(font *Font)
{
	draw_quad(100, 100, Font->W, Font->H, Font->Tex);
}

void draw_text(const char *Text, int X, int Y, color Color, font *Font, shaders *Shaders)
{
//	int AdvanceX = 30;//Font->Config.CharWidth + Font->Config.CharSpacing;
//	int AdvanceY = 10;//Font->Config.CharHeight + Font->Config.LineSpacing;
//	int TabWidth = 10;//Font->Config.CharWidth * 3; //@

	int PenX = X;
	int PenY = Y;

	int TextLen = strlen(Text);
//	printf("TEXT LENGTH: %d\n", TextLen);

	int NumVerticesPerChar = 6;
	int NumFloatsPerVertex = 8;
	int NumFloatsPerChar = NumVerticesPerChar * NumFloatsPerVertex;
	int NumBytesPerChar = NumFloatsPerChar * sizeof(float);
//	int InitiallyAllocatedMemory = (42+6) * 14356 * sizeof(float); // avoid allocating memory, this is the exact amount we need
//	int InitiallyAllocatedMemory = (42+6) * 3 * sizeof(float); // avoid allocating memory, this is the exact amount we need
//	int InitiallyAllocatedMemory = (42+6) * TextLen * sizeof(float); // avoid allocating memory, this is the exact amount we need
	int InitiallyAllocatedMemory =
		TextLen * NumBytesPerChar; // avoid allocating memory, this is the exact amount we need
	// ' ', '\n', '\t' dont make vertices

	GLuint VB;
	glGenBuffers(1, &VB);
	glBindBuffer(GL_ARRAY_BUFFER, VB);
	glBufferData(GL_ARRAY_BUFFER, InitiallyAllocatedMemory, 0, GL_DYNAMIC_DRAW);

//	float *Ptr = (float *) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	int BufferOffset = 0;
	int VerticesCount = 0;

	for (int i = 0; Text[i]; ++i)
	{
////		if(Text[i] == '\n' || PenX > WindowWidth)
//		if(Text[i] == '\n')
//		{
////			PenY += 16;
//			PenY += AdvanceY;
////			if(PenY > WindowHeight)
////			{
////				break;
////			}
//			PenX = X;
//
//			continue;
//		}
//		if(Text[i] == '\t')
//		{
//			PenX += TabWidth;
//			continue;
//		}

		int Index = Text[i] - ' ';
////		textureCoordinates TexCoord = Font->TextureCoordinates[Index];
//		textureCoordinates TexCoord;
//		float GlyphWidthTex = 1.0f / 95;
//		TexCoord.X0 = Index * GlyphWidthTex;
//		TexCoord.X1 = TexCoord.X0 + GlyphWidthTex;
//		TexCoord.Y0 = 1.0f;
//		TexCoord.Y1 = 0.0f;
		glyphInfo Glyph = Font->Glyphs[Index];

		int X = PenX + Glyph.XOffs;
		int Y = PenY + Glyph.YOffs;
		int W = Glyph.W;//Font->Config.CharWidth;
		int H = Glyph.H;//Font->Config.CharHeight;
//		printf("[%c] tx0: %f, tx1: %f, ty0: %f, ty1: %f\n", Text[i], Glyph.TX0, Glyph.TX1, Glyph.TY0, Glyph.TY1);

		float X0 = X;
		float X1 = X + W;
		float Y0 = Y;
		float Y1 = Y + H;
//		printf("[%c] x0: %f, x1: %f, y0: %f, y1: %f\n", Text[i], X0, X1, Y0, Y1);
//		printf("[%c] tx0: %f, tx1: %f, ty0: %f, ty1: %f\n",
//			Text[i], Glyph.TexX0, Glyph.TexX1, Glyph.TexY0, Glyph.TexY1);

		float Vertices[] =
		{
//			// upper-left
//			X0, Y0, TexCoord.X0, TexCoord.Y1, Color.R, Color.G, Color.B, Color.A,
//	
//			// upper-right
//			X1, Y0, TexCoord.X1, TexCoord.Y1, Color.R, Color.G, Color.B, Color.A,
//	
//			// lower-left
//			X0, Y1, TexCoord.X0, TexCoord.Y0, Color.R, Color.G, Color.B, Color.A,
//	
//			// lower-left
//			X0, Y1, TexCoord.X0, TexCoord.Y0, Color.R, Color.G, Color.B, Color.A,
//	
//			// lower-right
//			X1, Y1, TexCoord.X1, TexCoord.Y0, Color.R, Color.G, Color.B, Color.A,
//	
//			// upper-right
//			X1, Y0, TexCoord.X1, TexCoord.Y1, Color.R, Color.G, Color.B, Color.A,

			// upper-left
			X0, Y0, Glyph.TX0, Glyph.TY0, Color.R, Color.G, Color.B, Color.A,
	
			// upper-right
			X1, Y0, Glyph.TX1, Glyph.TY0, Color.R, Color.G, Color.B, Color.A,
	
			// lower-left
			X0, Y1, Glyph.TX0, Glyph.TY1, Color.R, Color.G, Color.B, Color.A,
	
			// lower-left
			X0, Y1, Glyph.TX0, Glyph.TY1, Color.R, Color.G, Color.B, Color.A,
	
			// lower-right
			X1, Y1, Glyph.TX1, Glyph.TY1, Color.R, Color.G, Color.B, Color.A,
	
			// upper-right
			X1, Y0, Glyph.TX1, Glyph.TY0, Color.R, Color.G, Color.B, Color.A,
		};
//		int NumBytesPerChar = NumFloatsPerChar * sizeof(float);
		glBufferSubData(GL_ARRAY_BUFFER, BufferOffset, NumBytesPerChar, Vertices);
		BufferOffset += NumBytesPerChar;

//		// upper-left
//		Ptr[0]=X0; Ptr[1]=Y0; Ptr[2]=Glyph.TexX0; Ptr[3]=Glyph.TexY1; Ptr[4]=Color.R; Ptr[5]=Color.G; Ptr[6]=Color.B; Ptr[7]=Color.A;
//
//		// upper-right
//		Ptr[8]=X1; Ptr[9]=Y0; Ptr[10]=Glyph.TexX1; Ptr[11]=Glyph.TexY1; Ptr[12]=Color.R; Ptr[13]=Color.G; Ptr[14]=Color.B; Ptr[15]=Color.A;
//
//		// lower-left
//		Ptr[16]=X0; Ptr[17]=Y1; Ptr[18]=Glyph.TexX0; Ptr[19]=Glyph.TexY0; Ptr[20]=Color.R; Ptr[21]=Color.G; Ptr[22]=Color.B; Ptr[23]=Color.A;
//
//		// lower-left
//		Ptr[24]=X0; Ptr[25]=Y1; Ptr[26]=Glyph.TexX0; Ptr[27]=Glyph.TexY0; Ptr[28]=Color.R; Ptr[29]=Color.G; Ptr[30]=Color.B; Ptr[31]=Color.A;
//
//		// lower-right
//		Ptr[32]=X1; Ptr[33]=Y1; Ptr[34]=Glyph.TexX1; Ptr[35]=Glyph.TexY0; Ptr[36]=Color.R; Ptr[37]=Color.G; Ptr[38]=Color.B;  Ptr[39]=Color.A;
//
//		// upper-right
//		Ptr[40]=X1; Ptr[41]=Y0; Ptr[42]=Glyph.TexX1; Ptr[43]=Glyph.TexY1; Ptr[44]=Color.R; Ptr[45]=Color.G; Ptr[46]=Color.B; Ptr[47]=Color.A;
//
////		memcpy(Ptr, Vertices, sizeof(Vertices));
//		Ptr += NumFloatsPerChar;
		VerticesCount += NumVerticesPerChar;
		
//		PenX += AdvanceX;
		PenX += (int)Glyph.Advance; //@
	}

//	glUnmapBuffer(GL_ARRAY_BUFFER);


	// Do OpenGL rendering stuff.

//	glBindTexture(GL_TEXTURE_2D, Font->TextureAtlas);
//	glBindTexture(GL_TEXTURE_2D, Font->TextureAtlas->Tex);
	glBindTexture(GL_TEXTURE_2D, Font->Tex);
	glUseProgram(Shaders->TextShader);
	pass_to_shader(Shaders->TextShader, "WindowWidth", (float)Shaders->WindowWidth);
	pass_to_shader(Shaders->TextShader, "WindowHeight", (float)Shaders->WindowHeight);

//	GLuint VertexArray, VertexBuffer;
	GLuint VertexArray;
	glGenVertexArrays(1, &VertexArray);
//	glGenBuffers(1, &VertexBuffer);

	glBindVertexArray(VertexArray);
//	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);

	// position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *)0);
	glEnableVertexAttribArray(0);

	// texture coordinates
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *) (sizeof(float) * 2));
	glEnableVertexAttribArray(1);

	// color
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *) (sizeof(float) * 4));
	glEnableVertexAttribArray(2);

//	glBufferData(GL_ARRAY_BUFFER, VerticesToBeRendered.Count * sizeof(float), VerticesToBeRendered.Data, GL_STATIC_DRAW);
//	glBufferData(GL_ARRAY_BUFFER, VerticesToBeRendered.Count * sizeof(float), VerticesToBeRendered.Data, GL_STREAM_DRAW);

//	int NumVertices = VerticesToBeRendered.Count / 7;
//	int NumVertices = BufferOffset / (NumFloatsPerVertex * sizeof(float));
	int NumVertices = VerticesCount;
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

//	glDeleteBuffers(1, &VertexBuffer);
	glDeleteBuffers(1, &VB);
	glDeleteVertexArrays(1, &VertexArray);
//	free(VerticesToBeRendered.Data);

//	glBindTexture(GL_TEXTURE_2D, 0); // bind default texture (?)
}

bitmapFont *make_bitmap_font(bitmapFontImageMetrics IM, fontConfig Config)
{
	image *FontBitmap = make_image(IM.FilePath);

	const int NumChars = 127 - ' ';
//	textureCoordinates Chars[NumChars];
	textureCoordinates *Chars = (textureCoordinates *)malloc(NumChars * sizeof(textureCoordinates));
	{
		float PixelWidth = 1.0f / IM.Width;
		float PixelHeight = 1.0f / IM.Height;
		for(char Char = ' '; Char < 127; ++Char)
		{
			int Index = Char - ' ';
			int Row = Index / IM.CharsInRow;
			int Column = Index % IM.CharsInRow;
			int CharWidthAndSome = IM.CharWidth + IM.HSpacing;
			int CharHeightAndSome = IM.CharHeight + IM.VSpacing;

			Chars[Index].X0 = (IM.LMargin + Column * CharWidthAndSome) * PixelWidth;
			Chars[Index].X1 = Chars[Index].X0 + IM.CharWidth * PixelWidth;
			Chars[Index].Y0 = ((IM.Height - IM.TMargin) - Row * CharHeightAndSome) * PixelHeight;
			Chars[Index].Y1 = Chars[Index].Y0 - IM.CharHeight * PixelHeight;
		}
	}

	bitmapFont *Font = (bitmapFont *)malloc(sizeof(bitmapFont));
//	Font->TextureAtlas = myTexture;
	Font->TextureAtlas = FontBitmap;
	Font->TextureCoordinates = Chars;
	Font->Config = Config;

	return Font;
}

void draw_text(const char *Text, int X, int Y, color Color, bitmapFont *Font, shaders *Shaders)
{
	int AdvanceX = Font->Config.CharWidth + Font->Config.CharSpacing;
	int AdvanceY = Font->Config.CharHeight + Font->Config.LineSpacing;
	int TabWidth = Font->Config.CharWidth * 3; //@

	int PenX = X;
	int PenY = Y;

	int TextLen = strlen(Text);
//	printf("TEXT LENGTH: %d\n", TextLen);

	int NumVerticesPerChar = 6;
	int NumFloatsPerVertex = 8;
	int NumFloatsPerChar = NumVerticesPerChar * NumFloatsPerVertex;
	int NumBytesPerChar = NumFloatsPerChar * sizeof(float);
//	int InitiallyAllocatedMemory = (42+6) * 14356 * sizeof(float); // avoid allocating memory, this is the exact amount we need
//	int InitiallyAllocatedMemory = (42+6) * 3 * sizeof(float); // avoid allocating memory, this is the exact amount we need
//	int InitiallyAllocatedMemory = (42+6) * TextLen * sizeof(float); // avoid allocating memory, this is the exact amount we need
	int InitiallyAllocatedMemory =
		TextLen * NumBytesPerChar; // avoid allocating memory, this is the exact amount we need

	GLuint VB;
	glGenBuffers(1, &VB);
	glBindBuffer(GL_ARRAY_BUFFER, VB);
	glBufferData(GL_ARRAY_BUFFER, InitiallyAllocatedMemory, 0, GL_DYNAMIC_DRAW);

//	float *Ptr = (float *) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	int BufferOffset = 0;
	int VerticesCount = 0;

	for (int i = 0; Text[i]; ++i)
	{
//		if(Text[i] == '\n' || PenX > WindowWidth)
		if(Text[i] == '\n')
		{
//			PenY += 16;
			PenY += AdvanceY;
//			if(PenY > WindowHeight)
//			{
//				break;
//			}
			PenX = X;

			continue;
		}
		if(Text[i] == '\t')
		{
			PenX += TabWidth;
			continue;
		}

		int Index = Text[i] - ' ';
//		textureCoordinates TexCoord = Font->TextureCoordinates[Index];
		textureCoordinates TexCoord;
		float GlyphWidthTex = 1.0f / 95;
		TexCoord.X0 = Index * GlyphWidthTex;
		TexCoord.X1 = TexCoord.X0 + GlyphWidthTex;
		TexCoord.Y0 = 1.0f;
		TexCoord.Y1 = 0.0f;

		int X = PenX;
		int Y = PenY;
		int W = Font->Config.CharWidth;
		int H = Font->Config.CharHeight;

		float X0 = X;
		float X1 = X + W;
		float Y0 = Y;
		float Y1 = Y + H;
//		printf("[%c] x0: %f, x1: %f, y0: %f, y1: %f\n", Text[i], X0, X1, Y0, Y1);
//		printf("[%c] tx0: %f, tx1: %f, ty0: %f, ty1: %f\n",
//			Text[i], Glyph.TexX0, Glyph.TexX1, Glyph.TexY0, Glyph.TexY1);

		float Vertices[] =
		{
//			// upper-left
//			X0, Y0, TexCoord.X0, TexCoord.Y1, Color.R, Color.G, Color.B, Color.A,
//	
//			// upper-right
//			X1, Y0, TexCoord.X1, TexCoord.Y1, Color.R, Color.G, Color.B, Color.A,
//	
//			// lower-left
//			X0, Y1, TexCoord.X0, TexCoord.Y0, Color.R, Color.G, Color.B, Color.A,
//	
//			// lower-left
//			X0, Y1, TexCoord.X0, TexCoord.Y0, Color.R, Color.G, Color.B, Color.A,
//	
//			// lower-right
//			X1, Y1, TexCoord.X1, TexCoord.Y0, Color.R, Color.G, Color.B, Color.A,
//	
//			// upper-right
//			X1, Y0, TexCoord.X1, TexCoord.Y1, Color.R, Color.G, Color.B, Color.A,

			// upper-left
			X0, Y0, TexCoord.X0, TexCoord.Y0, Color.R, Color.G, Color.B, Color.A,
	
			// upper-right
			X1, Y0, TexCoord.X1, TexCoord.Y0, Color.R, Color.G, Color.B, Color.A,
	
			// lower-left
			X0, Y1, TexCoord.X0, TexCoord.Y1, Color.R, Color.G, Color.B, Color.A,
	
			// lower-left
			X0, Y1, TexCoord.X0, TexCoord.Y1, Color.R, Color.G, Color.B, Color.A,
	
			// lower-right
			X1, Y1, TexCoord.X1, TexCoord.Y1, Color.R, Color.G, Color.B, Color.A,
	
			// upper-right
			X1, Y0, TexCoord.X1, TexCoord.Y0, Color.R, Color.G, Color.B, Color.A,
		};
//		int NumBytesPerChar = NumFloatsPerChar * sizeof(float);
		glBufferSubData(GL_ARRAY_BUFFER, BufferOffset, NumBytesPerChar, Vertices);
		BufferOffset += NumBytesPerChar;

//		// upper-left
//		Ptr[0]=X0; Ptr[1]=Y0; Ptr[2]=Glyph.TexX0; Ptr[3]=Glyph.TexY1; Ptr[4]=Color.R; Ptr[5]=Color.G; Ptr[6]=Color.B; Ptr[7]=Color.A;
//
//		// upper-right
//		Ptr[8]=X1; Ptr[9]=Y0; Ptr[10]=Glyph.TexX1; Ptr[11]=Glyph.TexY1; Ptr[12]=Color.R; Ptr[13]=Color.G; Ptr[14]=Color.B; Ptr[15]=Color.A;
//
//		// lower-left
//		Ptr[16]=X0; Ptr[17]=Y1; Ptr[18]=Glyph.TexX0; Ptr[19]=Glyph.TexY0; Ptr[20]=Color.R; Ptr[21]=Color.G; Ptr[22]=Color.B; Ptr[23]=Color.A;
//
//		// lower-left
//		Ptr[24]=X0; Ptr[25]=Y1; Ptr[26]=Glyph.TexX0; Ptr[27]=Glyph.TexY0; Ptr[28]=Color.R; Ptr[29]=Color.G; Ptr[30]=Color.B; Ptr[31]=Color.A;
//
//		// lower-right
//		Ptr[32]=X1; Ptr[33]=Y1; Ptr[34]=Glyph.TexX1; Ptr[35]=Glyph.TexY0; Ptr[36]=Color.R; Ptr[37]=Color.G; Ptr[38]=Color.B;  Ptr[39]=Color.A;
//
//		// upper-right
//		Ptr[40]=X1; Ptr[41]=Y0; Ptr[42]=Glyph.TexX1; Ptr[43]=Glyph.TexY1; Ptr[44]=Color.R; Ptr[45]=Color.G; Ptr[46]=Color.B; Ptr[47]=Color.A;
//
////		memcpy(Ptr, Vertices, sizeof(Vertices));
//		Ptr += NumFloatsPerChar;
		VerticesCount += NumVerticesPerChar;
		
		PenX += AdvanceX;
	}

//	glUnmapBuffer(GL_ARRAY_BUFFER);


	// Do OpenGL rendering stuff.

//	glBindTexture(GL_TEXTURE_2D, Font->TextureAtlas);
	glBindTexture(GL_TEXTURE_2D, Font->TextureAtlas->Tex);
	glUseProgram(Shaders->TextShader);
	pass_to_shader(Shaders->TextShader, "WindowWidth", (float)Shaders->WindowWidth);
	pass_to_shader(Shaders->TextShader, "WindowHeight", (float)Shaders->WindowHeight);

//	GLuint VertexArray, VertexBuffer;
	GLuint VertexArray;
	glGenVertexArrays(1, &VertexArray);
//	glGenBuffers(1, &VertexBuffer);

	glBindVertexArray(VertexArray);
//	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);

	// position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *)0);
	glEnableVertexAttribArray(0);

	// texture coordinates
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *) (sizeof(float) * 2));
	glEnableVertexAttribArray(1);

	// color
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *) (sizeof(float) * 4));
	glEnableVertexAttribArray(2);

//	glBufferData(GL_ARRAY_BUFFER, VerticesToBeRendered.Count * sizeof(float), VerticesToBeRendered.Data, GL_STATIC_DRAW);
//	glBufferData(GL_ARRAY_BUFFER, VerticesToBeRendered.Count * sizeof(float), VerticesToBeRendered.Data, GL_STREAM_DRAW);

//	int NumVertices = VerticesToBeRendered.Count / 7;
//	int NumVertices = BufferOffset / (NumFloatsPerVertex * sizeof(float));
	int NumVertices = VerticesCount;
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

//	glDeleteBuffers(1, &VertexBuffer);
	glDeleteBuffers(1, &VB);
	glDeleteVertexArrays(1, &VertexArray);
//	free(VerticesToBeRendered.Data);

//	glBindTexture(GL_TEXTURE_2D, 0); // bind default texture (?)
}


