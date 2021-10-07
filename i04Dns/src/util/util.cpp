#include "util.h"
#include "../../../mlib/system.h"
#include <stdio.h>
#include <stdlib.h>
#define MAX_BYTE_VALUE 255
#define IP_BYTES_SEPARATOR '.'
#define PORT_SEPARATOR ':'

#ifdef DEBUG_CONF

#include <iostream>

#endif

static bool rndinit = false;

int rand(int a, int b) {
	if (!rndinit) {
		srand(time_timestamp());
		rndinit = true;
	}
	int range = (b - a);
	int num = rand();
	if (range > RAND_MAX) {
		return num * ((float) RAND_MAX / range);
	}
	return a + (num % range);
}

int strCmp(char *a, char *b) {
	int aLen = 0;
	int bLen = 0;
	while (a[aLen]) aLen++;
	while (b[bLen]) bLen++;
	if (aLen < bLen) return -1;
	if (bLen < aLen) return 1;
	for (int i = 0; i < aLen; i++) {
		if (a[i] < b[i])
			return -1;
		if (a[i] > b[i])
			return 1;
	}
	return 0;
}

int ip4FromString(char *str, IPV4 *ip, DWord *ipStrLen) {
	Byte bytes[4];
	*((IPV4 *) &bytes) = 0;	
	DWord b = 0;
	DWord digitsFound = 0;
	int error = 1;
	DWord i = 0;
	for (;; i++) {
		if (!str[i] || str[i] == ' ' || str[i] == PORT_SEPARATOR) {
			if (b == 3 && digitsFound) {
				error = 0;
			}
			break;
		}
		if (str[i] == IP_BYTES_SEPARATOR) {
			if (!digitsFound) {
				break; // starts with dot, or has tow dots in a row
			}
			digitsFound = 0; // reset
			b++; // go to the next byte
			continue; // go the next char
		}
		if (str[i] < '0' || '9' < str[i]) {
			break; // Bullshit!
		}
		DWord tmp = bytes[b];
		tmp = tmp * 10 +  ((DWord) (str[i] - '0'));
		if (tmp > MAX_BYTE_VALUE) {
			break;
		}
		bytes[b] = (Byte) tmp;
		digitsFound++;
	}
	if (!error) {
		*ip = *((IPV4 *) &bytes);
		if(ipStrLen) {
			*ipStrLen = i;
		}
	}
	return error;
}

void ip4ToString(IPV4 ip, char *buf) {
	Byte *bytes = (Byte *) &ip;
//	sprintf(buf, "%03d.%03d.%03d.%03d", bytes[0], bytes[1], bytes[2], bytes[3]);
	sprintf(buf, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
}


#ifdef DEBUG_CONF

void printBuf(Byte *pBuf, DWord size) {
	const char *digits = "0123456789ABCDEF";
	for (DWord i = 0; i < size; i++) {
		int c = pBuf[i];
		int l = c & 0x0F;
		int h = (c & 0xF0) >> 4;
		std::cout << digits[h] << digits[l] << " ";
	}
	std::cout << std::endl;
}

#endif
