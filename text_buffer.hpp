#ifndef TEXT_BUFFER_HPP
#define TEXT_BUFFER_HPP

// Could keep an index of the last character instead of storing an actual '\0'?
// Iterator can be equal to OneAfterLast even though there is no character at that location
struct textBuffer
{
	int Size;
	char *Data;
	int OneAfterLast; // also size of contents
//	int Cursor; //@ probably shouldnt be part of the text buffer
};

void InitTextBuffer(textBuffer *Buffer);
char GetChar(textBuffer *Buffer, int At);
int GetStart(textBuffer *Buffer);
int GetEnd(textBuffer *Buffer);
bool IsStart(textBuffer *Buffer, int At);
bool IsEnd(textBuffer *Buffer, int At);
bool MoveForward(textBuffer *Buffer, int *Iter);
void MoveForwardFast(textBuffer *Buffer, int *Iter);
bool MoveBackward(textBuffer *Buffer, int *Iter);
//int GetCursor(textBuffer *Buffer);
//void SetCursor(textBuffer *Buffer, int At);
int GetCharsIntoLine(textBuffer *Buffer, int At);
int GetLinesIntoBuffer(textBuffer *Buffer, int At);
bool MoveToPrevLine(textBuffer *Buffer, int *Iter);
bool MoveToNextLine(textBuffer *Buffer, int *Iter);
bool Insert(textBuffer *Buffer, char Char, int At);
bool Insert(textBuffer *Buffer, const char *Text, int At);
bool Delete(textBuffer *Buffer, int At);
bool Delete(textBuffer *Buffer, int At, int NumChars);
bool MoveAtCharBackwards(textBuffer *Buffer, char Char, int *Iter);
//bool MoveAtCharForwards(textBuffer *Buffer, char Char, int *Iter);
//void CursorForward(textBuffer *Buffer);
//void CursorBackward(textBuffer *Buffer);
//void InsertAtCursor(textBuffer *Buffer, char Char, editor *Editor);
//int GetCursorCharIndex(textBuffer *Buffer);
//int GetCursorLineIndex(textBuffer *Buffer);

#endif



