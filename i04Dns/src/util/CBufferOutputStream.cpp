#include "CBufferOutputStream.h"
#include "../base/heap_dbg.h"

CBufferOutputStream::CBufferOutputStream(Byte *pBuf, int size)
{
    m_MaxSize = size;
    m_Size = 0;
    m_pBuf = pBuf;
}

CBufferOutputStream::~CBufferOutputStream()
{
	m_pBuf = 0;
	m_Size = 0;
	m_MaxSize = 0;
}

int CBufferOutputStream::SizeLeft() {
    if (m_Size < 0) {
        return -1;
    }
    return (int) m_MaxSize - m_Size;
}

int CBufferOutputStream::GetSize() {
    return m_Size;
}

bool CBufferOutputStream::WriteByte(Byte b) {
    if (SizeLeft() < 1) {
        return false;
    }
    m_pBuf[m_Size++] = b;
    return true;
}

bool CBufferOutputStream::WriteWord(Word w) {
    if (SizeLeft() < 2) {
        return false;
    }
    m_pBuf[m_Size++] = (Byte) ((w >> 8) & 0xFF);
    m_pBuf[m_Size++] = (Byte) (w & 0xFF);
    return true;
}

bool CBufferOutputStream::WriteDWord(DWord d) {
    if (SizeLeft() < 4) {
        return false;
    }
    m_pBuf[m_Size++] = (Byte) ((d >> 24) & 0xFF);
    m_pBuf[m_Size++] = (Byte) ((d >> 16) & 0xFF);
    m_pBuf[m_Size++] = (Byte) ((d >> 8) & 0xFF);
    m_pBuf[m_Size++] = (Byte) (d & 0xFF);
    return true;
}

bool CBufferOutputStream::WriteQWord(QWord q) {
    if (SizeLeft() < 8) {
        return false;
    }
    m_pBuf[m_Size++] = (Byte) ((q >> 56) & 0xFF);
    m_pBuf[m_Size++] = (Byte) ((q >> 48) & 0xFF);
    m_pBuf[m_Size++] = (Byte) ((q >> 40) & 0xFF);
    m_pBuf[m_Size++] = (Byte) ((q >> 32) & 0xFF);
    m_pBuf[m_Size++] = (Byte) ((q >> 24) & 0xFF);
    m_pBuf[m_Size++] = (Byte) ((q >> 16) & 0xFF);
    m_pBuf[m_Size++] = (Byte) ((q >> 8) & 0xFF);
    m_pBuf[m_Size++] = (Byte) (q & 0xFF);
    return true;
}

bool CBufferOutputStream::WriteBuf(Byte* buf, DWord len) {
    if (SizeLeft() < (int) len) {
        return false;
    }
    for (DWord i = 0; i < len; i++) {
        m_pBuf[m_Size++] = buf[i];
    }
    return true;
}

Byte *CBufferOutputStream::GetBuf() {
	return m_pBuf;
}

bool CBufferOutputStream::Skip(int nBytes) {
	m_Size += nBytes;
	if (m_Size < 0 || (DWord) m_Size >= m_MaxSize) {
		m_Size -= nBytes;
		return false;
	}
	return true;
}
