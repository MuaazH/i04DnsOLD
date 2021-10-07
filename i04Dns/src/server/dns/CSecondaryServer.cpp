#include "CSecondaryServer.h"
#include <stdio.h>
#include "../../base/heap_dbg.h"

using namespace dns;

CSecondaryServer::CSecondaryServer(NETADDR *addr) {
	mem_copy((void *) &m_Address, (void *) addr, sizeof(NETADDR));
	m_Sent = 0;
}

CSecondaryServer::~CSecondaryServer() {
}

NETADDR *CSecondaryServer::GetAddress() {
	return &m_Address;
}

void CSecondaryServer::MessageSent() {
	m_Sent++;
	// assume the server won't answer
	SetNextDelay(CSecondaryServer::c_TimeOut);
}

void CSecondaryServer::MessageReceived(DWord delay) {
	SetDelay(delay);
}

DWord CSecondaryServer::DelayScore() {
	if (m_Sent < DELAY_HISTORY_SIZE) {
		// give new shits a chance.
		return m_Sent;
	}
	DWord sum = DELAY_HISTORY_SIZE;
	for (DWord i = 0; i < DELAY_HISTORY_SIZE; i++) {
		sum += m_DelayHistory[i];
	}
	return sum;
}

void CSecondaryServer::SetNextDelay(DWord delay) {
	m_DelayHistoryIndex++;
	m_DelayHistoryIndex %= DELAY_HISTORY_SIZE;
	m_DelayHistory[m_DelayHistoryIndex] = delay;
}

void CSecondaryServer::SetDelay(DWord delay) {
	m_DelayHistory[m_DelayHistoryIndex] = delay;
}

void CSecondaryServer::ToString(char *str) {
	// addr max length = 22 = 15 (ip) + 6 (port) + 1 (null)
	const int maxIpLen = 22;
	char ipStr[maxIpLen];
	net_addr_str(&m_Address, (char *) &ipStr, maxIpLen, 1);
	sprintf(str, "(DNS-SRV %s)", ipStr);
}

QWord CSecondaryServer::GetSentCount() {
	return m_Sent;
}


