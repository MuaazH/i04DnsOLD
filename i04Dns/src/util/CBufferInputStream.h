#ifndef _CLASS_BUFFERINPUTSTREAM_H
#define _CLASS_BUFFERINPUTSTREAM_H

#include "util.h"

class CBufferInputStream
{
    public:
        CBufferInputStream(Byte *pBuf, DWord size);
        virtual ~CBufferInputStream();

        DWord GetSize();
        int GetPos();
        int SizeLeft();
		Byte *GetBuf();
		bool Skip(int bytes);

        bool ReadByte(Byte *b);
        bool ReadWord(Word *w);
        bool ReadDWord(DWord *d);
        bool ReadQWord(QWord *q);
        bool ReadBuf(Byte *buf, DWord size);

    private:
        Byte *m_pBuf;
        DWord m_Size;
        int m_Pointer;
};

#endif
