#ifndef _ENUM_IP_H_
#define _ENUM_IP_H_


#define max_name_len 64
#define ipv4_len 4

struct NetworkInterface {
	char name[max_name_len];
	unsigned char ip[4];
	unsigned char mask[4];
	NetworkInterface *pNext;
};

NetworkInterface *getNetworkInterfaces();

void freeNetworkInterfaces(NetworkInterface *pHead);

#endif
