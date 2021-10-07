#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#define Byte unsigned char
#define Word unsigned short
#define DWord unsigned int
#define QWord unsigned long long

#define SOCKET_CHECK_DELAY 10

#define IPV4 unsigned int
#define IPV4_MAX_LENGTH 15

#define MACADDR char[6]

#define PNETADDR_TO_IPV4(a) (IPV4) *((IPV4*) &((a)->ip))
#define IPV4_TO_NETADDR(_vn, _ip, _port) NETADDR _vn; _vn.type = NETTYPE_IPV4; *((IPV4 *) &(_vn.ip)) = _ip; _vn.port = _port;

#define IPV4_TO_STR(bufName, pBufName, ip) char bufName[IPV4_MAX_LENGTH + 1]; bufName[IPV4_MAX_LENGTH] = 0; char *pBufName = (char *) &bufName; ip4ToString(ip, pBufName);

#define CHAR_LOWERCASE(chr) ('A' <= chr && chr <= 'Z') ? ((char) (chr | 0x20)) : chr

int strCmp(char *a, char *b);

int rand(int a, int b);

/**
	returns 0 on success
*/
int ip4FromString(char *str, IPV4 *ip, DWord *ipStrLen = 0);

void ip4ToString(IPV4 ip, char *buf);


#ifdef DEBUG_CONF

void printBuf(Byte *pBuf, DWord size);

#endif


#endif // UTIL_H_INCLUDED
