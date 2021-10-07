#ifndef _DOMAIN_NAME_SERVER_HEADER_
#define _DOMAIN_NAME_SERVER_HEADER_

#define MAX_NAME_LENGTH 255
#define MAX_DNS_LONG_ENTRIES 100

#include "../CConfigFile.h"

namespace dns {

	void run(CConfigFile *config);
	
}

#endif
