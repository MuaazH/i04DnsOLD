#include "CBufferInputStream.h"
#include "../base/heap_dbg.h"

#define READ_8_AS_32 ((DWord) (m_pBuf[m_Pointer++]))
#define READ_8_AS_64 ((QWord) (m_pBuf[m_Pointer++]))

CBufferInputStream::CBufferInputStream(Byte *pBuf, DWord size)
{
    //ctor
    m_pBuf = pBuf;
    m_Size = size;
    m_Pointer = 0;
}

CBufferInputStream::~CBufferInputStream()
{
    //dtor
    m_pBuf = 0;
    m_Size = 0;
}

int CBufferInputStream::SizeLeft() {
    if (m_Pointer < 0) return -1;
    return (int) m_Size - m_Pointer;
}

bool CBufferInputStream::Skip(int n) {
	m_Pointer += n;
    if (m_Pointer < 0 || (DWord) m_Pointer >= m_Size) {
        m_Pointer -= n;
        return false;
    }
    return true;
}

int CBufferInputStream::GetPos() {
	return m_Pointer;
}

DWord CBufferInputStream::GetSize() {
	return m_Size;
}

Byte *CBufferInputStream::GetBuf() {
	return m_pBuf;
}

bool CBufferInputStream::ReadByte(Byte *b) {
    if (SizeLeft() < 1) return false;
    *b = m_pBuf[m_Pointer++];
    return true;
}

bool CBufferInputStream::ReadWord(Word *w) {
    if (SizeLeft() < 2) return false;
    DWord b0 = READ_8_AS_32 << 8;
    DWord b1 = READ_8_AS_32;
    *w = (Word) (b0 | b1);
    return true;
}

bool CBufferInputStream::ReadDWord(DWord *d) {
    if (SizeLeft() < 4) return false;
    DWord b0 = READ_8_AS_32 << 0x18;
    DWord b1 = READ_8_AS_32 << 0x10;
    DWord b2 = READ_8_AS_32 << 0x08;
    DWord b3 = READ_8_AS_32;
    *d = b0 | b1 | b2 | b3;
   return true;
}

bool CBufferInputStream::ReadQWord(QWord *q) {
    if (SizeLeft() < 8) return false;
    QWord b0 = READ_8_AS_64 << 0x38;
    QWord b1 = READ_8_AS_64 << 0x30;
    QWord b2 = READ_8_AS_64 << 0x28;
    QWord b3 = READ_8_AS_64 << 0x20;
    QWord b4 = READ_8_AS_64 << 0x18;
    QWord b5 = READ_8_AS_64 << 0x10;
    QWord b6 = READ_8_AS_64 << 0x08;
    QWord b7 = READ_8_AS_64;
    *q = b0 | b1 | b2 | b3 | b4 | b5 | b6 | b7;
    return true;
}

bool CBufferInputStream::ReadBuf(Byte *buf, DWord len) {
    if (SizeLeft() < (int) len) return false;
    for (DWord i = 0; i < len; i++) {
        buf[i] = m_pBuf[m_Pointer++];
    }
    return true;
}
