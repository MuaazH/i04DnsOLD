#ifndef _CLASS_BUFFEROUTPUTSTREAM_H
#define _CLASS_BUFFEROUTPUTSTREAM_H

#include "util.h"

class CBufferOutputStream
{
    public:
        CBufferOutputStream(Byte *pBuf, int size);
        virtual ~CBufferOutputStream();

		Byte *GetBuf();
        int GetSize();
        int SizeLeft();
		bool Skip(int nBytes);

        bool WriteByte(Byte c);
        bool WriteWord(Word w);
        bool WriteDWord(DWord d);
        bool WriteQWord(QWord q);
        bool WriteBuf(Byte* buf, DWord len);

    private:
        Byte *m_pBuf;
        int m_Size;
        DWord m_MaxSize;
};

#endif
