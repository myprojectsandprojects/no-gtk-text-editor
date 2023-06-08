
#include "text_drawing.hpp"
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bitmapFont *make_bitmap_font(bitmapFontImageMetrics IM, fontConfig Config)
{
	int ImageWidth, ImageHeight, ImageNChannels;
	stbi_set_flip_vertically_on_load(1);
	unsigned char *ImageData = stbi_load(IM.FilePath, &ImageWidth, &ImageHeight, &ImageNChannels, 0);
	if(!ImageData)
	{
		fprintf(stderr, "error: stbi_load()\n");
		return NULL;
	}
	else
	{
		printf("make_bitmap_font: width: %d, height: %d, num channels: %d\n", ImageWidth, ImageHeight, ImageNChannels);
	}

	GLuint myTexture;
	glGenTextures(1, &myTexture);
	glBindTexture(GL_TEXTURE_2D, myTexture);

	// What to do when texture coordinates are outside of the texture:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// What to do when the texture is minified/magnified:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ImageWidth, ImageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, ImageData);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ImageWidth, ImageHeight, 0, GL_RED, GL_UNSIGNED_BYTE, ImageData);
//	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(ImageData);

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
	Font->Shader = make_text_shader();
	Font->TextureAtlas = myTexture;
	Font->TextureCoordinates = Chars;
	Font->Config = Config;

	return Font;
}

void draw_text(const char *Text, int X, int Y, color Color, bitmapFont *Font, int WindowWidth, int WindowHeight)
{
	int PenX = X;
	int PenY = Y;

	float PixelWidth = 2.0f / WindowWidth;
	float PixelHeight = 2.0f / WindowHeight;

	int TextLen = strlen(Text);
//	printf("TEXT LENGTH: %d\n", TextLen);

	int NumFloatsPerVertex = 8;
	int NumVerticesPerChar = 6;
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
		if(Text[i] == '\n' || PenX > WindowWidth)
		{
//			PenY += 16;
			PenY += 10;
			if(PenY > WindowHeight)
			{
				break;
			}
			PenX = X;

			continue;
		}
		if(Text[i] == '\t')
		{
			PenX += 24;
			continue;
		}

		int Index = Text[i] - ' ';
		textureCoordinates TexCoord = Font->TextureCoordinates[Index];

		int X = PenX;
		int Y = PenY;
		int Width = Font->Config.CharWidth;
		int Height = Font->Config.CharHeight;

		float _X = X * PixelWidth - 1.0f;
		float _Y = 1.0f - Y * PixelHeight;
		float _Width = Width * PixelWidth;
		float _Height = Height * PixelHeight;

		float X0 = _X;
		float X1 = _X + _Width;
		float Y0 = _Y;
		float Y1 = _Y - _Height;
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
		
		PenX += Font->Config.CharWidth;
	}

//	glUnmapBuffer(GL_ARRAY_BUFFER);


	// Do OpenGL rendering stuff.

	glBindTexture(GL_TEXTURE_2D, Font->TextureAtlas);
	glUseProgram(Font->Shader);

//	GLuint VertexArray, VertexBuffer;
	GLuint VertexArray;
	glGenVertexArrays(1, &VertexArray);
//	glGenBuffers(1, &VertexBuffer);

	glBindVertexArray(VertexArray);
//	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *) (sizeof(float) * 2));
	glEnableVertexAttribArray(1);

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

void draw_text_buffer(
	textBuffer *TextBuffer,
	bitmapFont *Font,
	int X, int Y, int W, int H,
	int OffsH, int OffsV,
	int WindowWidth, int WindowHeight)
{
	color Color = {1.0f, 1.0f, 1.0f, 1.0f};

	// The offsets keep track of how much our "text box" has been moved downwards and to the left relative to the buffer.
	// So we start "laying out" characters from the beginning of the buffer, but we only actually draw them if they
	// land inside the box.
	// So this puts our box's top left corner at 0,0
	// Then we render char if: 0 < PenX && PenX < W && 0 < PenY && PenY < H
	int PenX = -OffsH;
	int PenY = -OffsV;

	float PixelWidth = 2.0f / WindowWidth;
	float PixelHeight = 2.0f / WindowHeight;

	int NumFloatsPerVertex = 8;
	int NumVerticesPerChar = 6;
	int NumFloatsPerChar = NumVerticesPerChar * NumFloatsPerVertex;
	int NumBytesPerChar = NumFloatsPerChar * sizeof(float);

	//@ Just assume we render all characters in the text buffer. This is clearly a no-no in the long run.
	int InitiallyAllocatedMemory = TextBuffer->OneAfterLast * NumBytesPerChar;

	GLuint VB;
	glGenBuffers(1, &VB);
	glBindBuffer(GL_ARRAY_BUFFER, VB);
	glBufferData(GL_ARRAY_BUFFER, InitiallyAllocatedMemory, 0, GL_DYNAMIC_DRAW);

//	float *Ptr = (float *) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	int BufferOffset = 0;
	int VerticesCount = 0;

	for(int i = GetStart(TextBuffer); !IsEnd(TextBuffer, i); MoveForward(TextBuffer, &i))
	{
		char Ch = GetChar(TextBuffer, i);
//		printf("%c\n", Ch);

		if(Ch == '\n'/* || PenX > WindowWidth*/)
		{
//			PenY += 16;
			PenY += Font->Config.CharHeight;
//			if(PenY > WindowHeight)
//			{
//				break;
//			}
			PenX = -OffsH;

			continue;
		}
		if(Ch == '\t')
		{
			PenX += 24;
			continue;
		}

		if(0 <= PenX && PenX <= W && 0 <= PenY && PenY <= H)
		{
			int Index = Ch - ' ';
			textureCoordinates TexCoord = Font->TextureCoordinates[Index];

			int WindowX = X + PenX;
			int WindowY = Y + PenY;
			int Width = Font->Config.CharWidth;
			int Height = Font->Config.CharHeight;

			float _X = WindowX * PixelWidth - 1.0f;
			float _Y = 1.0f - WindowY * PixelHeight;
			float _Width = Width * PixelWidth;
			float _Height = Height * PixelHeight;

			float X0 = _X;
			float X1 = _X + _Width;
			float Y0 = _Y;
			float Y1 = _Y - _Height;
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
		}
		
		PenX += Font->Config.CharWidth;
	}

//	glUnmapBuffer(GL_ARRAY_BUFFER);

	// Do OpenGL rendering stuff.

	glBindTexture(GL_TEXTURE_2D, Font->TextureAtlas);
	glUseProgram(Font->Shader);

//	GLuint VertexArray, VertexBuffer;
	GLuint VertexArray;
	glGenVertexArrays(1, &VertexArray);
//	glGenBuffers(1, &VertexBuffer);

	glBindVertexArray(VertexArray);
//	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *) (sizeof(float) * 2));
	glEnableVertexAttribArray(1);

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


