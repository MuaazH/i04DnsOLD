#include "enumip.h"
#include "detect.h"

// #include <stdio.h>

#if defined(CONF_FAMILY_UNIX)

#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>

NetworkInterface *getNetworkInterfaces() {
    struct ifaddrs *ifap, *ifa;

	NetworkInterface *head = 0;
	NetworkInterface *node = head;

    getifaddrs (&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family != AF_INET) {
			continue;
		}
	    struct sockaddr_in *sa = (struct sockaddr_in *) ifa->ifa_addr;
		unsigned char *addr = (unsigned char*) &sa->sin_addr;
		bool zero = true;
		for (int i = 0; i < ipv4_len; i++) {
			if(addr[i]) {
				zero = false;
				break;
			} 
		}
		if (zero) {
			continue;
		}
		if (!head) {
			head = new NetworkInterface();
			head->pNext = 0;
			node = head;
		} else {
			node->pNext = new NetworkInterface();
			node = node->pNext;
			node->pNext = 0;
		}
		// copy the name		
		char *name = ifa->ifa_name;
		for (int i = 0; i < max_name_len; i++) {
			node->name[i] = name[i];
			if (!name[i]){
				break;
			}
		}
		node->name[max_name_len - 1] = 0;
		// copy the ip
	    struct sockaddr_in *sm = (struct sockaddr_in *) ifa->ifa_netmask;
		unsigned char *mask = (unsigned char*) &sm->sin_addr;
		for (int i = 0; i < ipv4_len; i++) {
			node->ip[i] = addr[i];
			node->mask[i] = mask[i];
		}
    }
    freeifaddrs(ifap);
	return head;
}

#elif defined(CONF_FAMILY_WINDOWS)



 // Link to ws2_32.lib
#include <winsock2.h>
// Link to Iphlpapi.lib
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>

// Note: could also use malloc() and free()
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

// quick but not safe!
void strToIp(char *str, unsigned char *ip) {
	int j = 0;
	*((int *) ip) = 0;
	for (int i = 0; i < 16; i++) {
		if (!str[i]) {
			break;
		}
		if (str[i] == '.') {
			j++;
			continue;
		}
		ip[j] = ip[j] * 10 + (str[i] - '0');
	}
}

NetworkInterface *getNetworkInterfaces() {
    // Declare and initialize variables
	//
    // It is possible for an adapter to have multiple
    // IPv4 addresses, gateways, and secondary WINS servers assigned to the adapter
    //
    // Note that this sample code only prints out the
    // first entry for the IP address/mask, and gateway, and
    // the primary and secondary WINS server for each adapter
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;

	// variables used to print DHCP time info
	// char buffer[32];
	// errno_t error;

    ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
    pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(sizeof (IP_ADAPTER_INFO));
    if (pAdapterInfo == NULL) {
        return 0;
    }
 
    // Make an initial call to GetAdaptersInfo to get
    // the necessary size into the ulOutBufLen variable
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        FREE(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(ulOutBufLen);
        if (pAdapterInfo == NULL) {
            return 0;
        }
    }
	NetworkInterface *head = 0;
	NetworkInterface *node = 0;
    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        while (pAdapter) {
			char *name = (char *) &pAdapter->Description;

			PIP_ADDR_STRING list = &pAdapter->IpAddressList;
			
			while (list) {
				char *addrStr = (char *) &pAdapter->IpAddressList.IpAddress.String;
				char *maskStr = (char *) &pAdapter->IpAddressList.IpMask.String;
				unsigned char addr[4];
				unsigned char mask[4];
				strToIp(addrStr, (unsigned char *) &addr);
				strToIp(maskStr, (unsigned char *) &mask);
				if (*((int *) &addr) != 0) {
					if (!head) {
						head = new NetworkInterface();
						head->pNext = 0;
						node = head;
					} else {
						node->pNext = new NetworkInterface();
						node = node->pNext;
						node->pNext = 0;
					}
					
					// copy the name		
					for (int i = 0; i < max_name_len; i++) {
						node->name[i] = name[i];
						if (!name[i]){
							break;
						}
					}
					node->name[max_name_len - 1] = 0;
					// copy the ip
					*((int *) node->ip) = *((int *) &addr);
					*((int *) node->mask) = *((int *) &mask);
				}
				list = list->Next;
			}
            pAdapter = pAdapter->Next;
        }
	}
	
	if (pAdapterInfo)
		FREE(pAdapterInfo);

	return head;
}

#else
	#error NOT IMPLEMENTED
#endif


void freeNetworkInterfaces(NetworkInterface *pHead) {
	NetworkInterface *pNode = pHead;
	while (pNode) {
		NetworkInterface *pNext = pNode->pNext;
		delete pNode;
		pNode = pNext;
	}
}


