#include "text_buffer.hpp"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

void InitTextBuffer(textBuffer *Buffer)
{
//	Buffer->Size = 3;
	Buffer->Size = 1024;	
	Buffer->Data = (char *) malloc(Buffer->Size);
	assert(Buffer->Data);
	Buffer->OneAfterLast = 0;
//	Buffer->Cursor = 0;

//	memset(Buffer->Data, 'x', Buffer->Size);

//	Buffer->Data[0] = '1';
//	Buffer->Data[1] = '2';
//	Buffer->Data[2] = '3';
//	Buffer->Data[3] = '4';
//	Buffer->Data[4] = '5';
//	Buffer->Data[5] = '6';
//	Buffer->Data[6] = '7';
//	Buffer->Data[7] = '8';
//	Buffer->Data[8] = '9';
//	Buffer->Data[9] = '0';
//	Buffer->OneAfterLast = 10;
}

char GetChar(textBuffer *Buffer, int At)
{
	// validate iterator
	assert(0 <= At && At < Buffer->OneAfterLast+1);

	if(At == Buffer->OneAfterLast)
	{
		return '\0'; //@ ?
	}
	else
	{
		return Buffer->Data[At];
	}
}

int GetStart(textBuffer *Buffer)
{
	return 0;
}

int GetEnd(textBuffer *Buffer)
{
//	int i = 0;
//	while(true)	
//	{
//		assert(i < Buffer->Size);
//		if(Buffer->Data[i] == '\0')
//		{
//			return i;
//		}
//		++i;
//	}
	return Buffer->OneAfterLast;
}

bool IsStart(textBuffer *Buffer, int At)
{
	return (At == 0);
}

bool IsEnd(textBuffer *Buffer, int At)
{
	return (At == Buffer->OneAfterLast);
}

bool MoveForward(textBuffer *Buffer, int *Iter)
{
	// validate iterator
	assert(0 <= *Iter && *Iter < Buffer->OneAfterLast+1);

	if(*Iter == Buffer->OneAfterLast)
	{
		return false; // at the end
	}
	else
	{
		*Iter += 1;
		return true;
	}
}

void MoveForwardFast(textBuffer *Buffer, int *Iter)
{
	*Iter += 1;
}

bool MoveBackward(textBuffer *Buffer, int *Iter)
{
	// validate iterator
	assert(0 <= *Iter && *Iter < Buffer->OneAfterLast+1);

	if(*Iter == 0)
	{
		return false; // at the beginning
	}
	else
	{
		*Iter -= 1;
		return true;
	}
}

//int GetCursor(textBuffer *Buffer)
//{
//	return Buffer->Cursor;
//}
//
//void SetCursor(textBuffer *Buffer, int At)
//{
//	assert(0 <= At && At < Buffer->OneAfterLast+1);
//
//	Buffer->Cursor = At;
//}

int GetCharsIntoLine(textBuffer *Buffer, int At)
{
	assert(0 <= At && At < Buffer->OneAfterLast+1);

	int i = At;
	while(i > 0)
	{
		--i;
		if(Buffer->Data[i] == '\n')
		{
			i += 1;
			break;
		}
	}
	return At - i;
}

int GetLinesIntoBuffer(textBuffer *Buffer, int At)
{
	assert(0 <= At && At < Buffer->OneAfterLast+1);

	int LineCount = 0;
	for(int i = At - 1; i >= 0; --i)
	{
		if(Buffer->Data[i] == '\n')
		{
			++LineCount;
		}
	}
	return LineCount;
}

// moves Iter to the beginning of previous line and returns true
// if no previous line, returns false
bool MoveToPrevLine(textBuffer *Buffer, int *Iter)
{
	assert(0 <= *Iter && *Iter < Buffer->OneAfterLast+1);

//	bool BrokeOut = false;
//	bool OnPreviousLine = false;
//	int i = *Iter;
////	while(i > 0)
//	while(!IsStart(Buffer, i))
//	{
//		--i;
//		if(GetChar(Buffer, i) == '\n')
//		{
//			if(OnPreviousLine)
//			{
//				BrokeOut = true;
//				break;
//			}
//			else
//			{
//				OnPreviousLine = true;
//			}
//		}
//	}
//	bool Result = (BrokeOut || OnPreviousLine);
//	if(Result)
//	{
//		*Iter = i;
//		*Iter += BrokeOut ? 1 : 0;
//	}
//	return Result;

	bool Result = false;
	if(MoveAtCharBackwards(Buffer, '\n', Iter))
	{
		// go to the beginning of the line
		while(!IsStart(Buffer, *Iter))
		{
			MoveBackward(Buffer, Iter);
			if(GetChar(Buffer, *Iter) == '\n')
			{
				MoveForward(Buffer, Iter);
				break;
			}
		}
		Result = true;
	}
	return Result;
}

// moves Iter to the beginning of next line and returns true
// if no next line, returns false
bool MoveToNextLine(textBuffer *Buffer, int *Iter)
{
	assert(0 <= *Iter && *Iter < Buffer->OneAfterLast+1);

	int i = *Iter;
	bool FoundLine = false;
//	while(GetChar(Buffer, i) != '\0')
	while(i < Buffer->OneAfterLast)
	{
		if(GetChar(Buffer, i) == '\n')
		{
			FoundLine = true;
			i += 1;
			break;
		}
		++i;
	}
	if(FoundLine)
	{
		*Iter = i;
	}
	return FoundLine;
}

bool Insert(textBuffer *Buffer, char Char, int At)
{
	assert(0 <= At && At <= Buffer->OneAfterLast);

	bool Result = false; // Assume buffer is full

	if(Buffer->OneAfterLast < Buffer->Size)
	{
		int i = Buffer->OneAfterLast;
		while(i > At)
		{
			Buffer->Data[i] = Buffer->Data[i-1];
			--i;
		}
		Buffer->Data[At] = Char;
		Buffer->OneAfterLast += 1;
		Result = true;
	}

	return Result;
}

bool Insert(textBuffer *Buffer, const char *Text, int At)
{
	assert(0 <= At && At <= Buffer->OneAfterLast);

	bool Result = false; // Assume buffer is full
	int TextLength = strlen(Text);

	if(!((Buffer->OneAfterLast + TextLength) <= Buffer->Size))
	{
		int NewDataSize = 2 * (Buffer->OneAfterLast + TextLength);
		char *NewData = (char *) malloc(NewDataSize);
		assert(NewData);
		for(int i = 0; i < Buffer->OneAfterLast; ++i)
		{
			NewData[i] = Buffer->Data[i];
		}
		free(Buffer->Data);
		Buffer->Data = NewData;
		Buffer->Size = NewDataSize;
		printf("ALLOCATED NEW MEMORY: %d\n", Buffer->Size);
	}

	int i = Buffer->OneAfterLast-1;
	while(i >= At)
	{
		Buffer->Data[i+TextLength] = Buffer->Data[i];
		--i;
	}
	for(int i = 0; i < TextLength; ++i)
	{
		Buffer->Data[At+i] = Text[i];
	}
	Buffer->OneAfterLast += TextLength;
	Result = true;

	return Result;
}

/*
we dont expect the caller to keep track of whether the buffer is empty or not (well return false if the buffer is empty)
we do expect the caller to not give us invalid[1] iterators though (when we see one well fail an assertion)

[1] invalid iterator is an iterator which is either outside of the buffer bounds or after the '\0' that marks the end of the buffer contents.
*/
bool Delete(textBuffer *Buffer, int At)
{
	// validate an iterator
	assert(0 <= At && At <= Buffer->OneAfterLast);

	if(Buffer->OneAfterLast > 0)
	{
//		do
//		{
//			++At;
//			Buffer->Data[At-1] = Buffer->Data[At];
//		}
//		while(At < Buffer->OneAfterLast-1);
		while(At < Buffer->OneAfterLast-1)
		{
			Buffer->Data[At] = Buffer->Data[At+1];
			++At;
		}
		Buffer->OneAfterLast -= 1;
		return true;
	}
	else
	{
		// buffer is empty or At is at the end of the buffer
		return false;
	}
}

bool Delete(textBuffer *Buffer, int At, int NumChars)
{
	// validate an iterator
	assert(0 <= At && At <= Buffer->OneAfterLast);

	// cant delete more characters than there are
	assert(NumChars <= Buffer->OneAfterLast);

	//@ if the caller wants to delete more chars than can be deleted... we dont check for that

	if(NumChars <= Buffer->OneAfterLast) //@ because of the assertion above else-block never executes
	{
		while(At+NumChars < Buffer->OneAfterLast)
		{
			Buffer->Data[At] = Buffer->Data[At+NumChars];
			++At;
		}
		Buffer->OneAfterLast -= NumChars;
		return true;
	}
	else
	{
		return false;
	}
	return false;
}

//bool MoveAtCharForwards(textBuffer *Buffer, char Char, int *Iter)
//{}
bool MoveAtCharBackwards(textBuffer *Buffer, char Char, int *Iter)
{
	assert(0 <= *Iter && *Iter < Buffer->OneAfterLast+1);

	bool Result = false;
	int i = *Iter;
	do
	{
		if(MoveBackward(Buffer, &i))
		{
			if(GetChar(Buffer, i) == '\n')
			{
				*Iter = i;
				Result = true;
				break;
			}
		}
		else
		{
			break;
		}
	}
	while(!IsStart(Buffer, *Iter));
	return Result;
}

//void CursorForward(textBuffer *Buffer)
//{
//	assert(Buffer->Data[Buffer->Cursor] != '\0');
//	Buffer->Cursor += 1;
//}
//
//void CursorBackward(textBuffer *Buffer)
//{
//	assert(Buffer->Cursor != 0);
//	Buffer->Cursor -= 1;
//}

//void InsertAtCursor(textBuffer *Buffer, char Char, editor *Editor)
//{
//	// find 0-char at the end
//	int i = Buffer->Cursor;
//	while(Buffer->Data[i] != '\0') ++i;
//
//	assert(i < Buffer->Size);
//	if(i == Buffer->Size-1)
//	{
//		// Dont insert if the buffer is full
//		return;
//	}
//
//	// copy characters
//	while(i >= Buffer->Cursor)
//	{
//		Buffer->Data[i+1] = Buffer->Data[i];
//		--i;
//	}
//
//	Buffer->Data[Buffer->Cursor] = Char;
//	SetCursor(Buffer, Buffer->Cursor+1, Editor);
//}

//int GetCursorCharIndex(textBuffer *Buffer)
//{
//	int i = Buffer->Cursor;
//	while(i > 0)
//	{
//		--i;
//		if(Buffer->Data[i] == '\n')
//		{
//			i += 1;
//			break;
//		}
//	}
//	return Buffer->Cursor - i;
//}

//// Figure out on which line the cursor is at
//int GetCursorLineIndex(textBuffer *Buffer)
//{
//	int LineCount = 0; // number of lines before cursor
//	for(int i = Buffer->Cursor - 1; i >= 0; --i)
//	{
//		if(Buffer->Data[i] == '\n')
//		{
//			++LineCount;
//		}
//	}
//	return LineCount;
//}



