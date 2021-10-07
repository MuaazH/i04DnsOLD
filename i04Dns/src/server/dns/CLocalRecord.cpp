#include "CLocalRecord.h"
#include "../../../../mlib/system.h"
#include "../../base/heap_dbg.h"
#include <stdio.h>

using namespace dns;

CLocalRecord::CLocalRecord(char *name, IPV4 *ips, DWord ipsCount) {
	DWord len = str_length(name) + 1;

	NEW_ARR(m_pName, char, len);
	mem_copy((void *) m_pName, (void *) name, len);

	NEW_ARR(m_pIps, IPV4, ipsCount);
	mem_copy((void *) m_pIps, (void *) ips, sizeof(IPV4) * ipsCount);

	m_pIpsCount = ipsCount;
}

CLocalRecord::~CLocalRecord() {
	if (m_pName) {
		DELETE_ARR(m_pName);
	}
	if (m_pIps) {
		DELETE_ARR(m_pIps);
	}
}

void CLocalRecord::ToString(char *str) {
	char *buf = str;
	sprintf(buf, "(Local-Record %s =", m_pName);
	while (*buf) buf++;
	for (DWord i = 0; i < m_pIpsCount; i++) {
		buf[0] = ' ';
		buf++;
		ip4ToString(m_pIps[i], buf);
		buf += IPV4_MAX_LENGTH;
	}
	*buf = ')';
	buf++;
	*buf = 0;
}

