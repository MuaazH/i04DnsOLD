#include "dnsServer.h"
#include "../../../../mlib/system.h"
#include "../../../../mlib/enumip.h"
#include "../../util/util.h"
#include "../logger.h"
#include "CLocalRecord.h"
#include "../../util/CBufferInputStream.h"
#include "../../util/CBufferOutputStream.h"
#include <RedBlackBST.h>
#include "../../base/heap_dbg.h"

#define SERVER_PORT         53
#define DNS_MSG_MAX_SIZE    512
#define CLEAN_UP_DELAY      2000

#include "protocol.h"
#include "CQuery.h"
#include "CResourceRecord.h"
#include "CMessage.h"
#include "dnsCache.h"
#include "dnsQueue.h"

#include <stdio.h>

// ##############################################################
// ##############################################################
// ##############################################################
// ##############################################################

using namespace dns;

static NETSOCKET socket = {0, 0, 0};
static CConfigFile *pConfig = 0;
static CCache *pCache = 0;
static CDnsQueue *pQueue = 0;
static int running = 0;
//

inline bool isBlackListed(char *pName) {
	if (pConfig->IsWhiteListed(pName)) return false;
	return pConfig->IsBlackListed(pName);
}

inline bool isRecordTypeSupported(Word t) {
	switch (t)
	{
		case RECORD_TYPE_IPv4:
		case RECORD_TYPE_CNAME:
			return true;
		default:
			return false;
	}
}

inline bool recordFilter(CResourceRecord *pRec) {
	if (!isRecordTypeSupported(pRec->GetRecordType())) {
		char recTypStr[16];
		sprintf(recTypStr, "%d", pRec->GetRecordType());
		logInfo(LOG_DNS, 0x009, recTypStr); // Dropping
		return false;
	}
	if (isBlackListed(pRec->GetName())) {
		logInfo(LOG_DNS, 0x00A, pRec->GetName()); // Dropping
		return false;
	}
	return true;
}

inline int evalMask(Byte *a, Byte *b) {
	for (int i = 0; i < 4; i++) {
		if (a[i] != b[i]) {
			DWord x = a[i];
			DWord y = b[i];
			DWord mask = i * 8;
			for (int j = 1; j < 8; j++) {
				if ((x >> j) == (y >> j)) {
					mask += j;
					break;
				}
			}
			return mask;
		}
	}
	return 32;
}

inline IPV4 findBestSubnet(IPV4 src) {
	NetworkInterface *pHead = getNetworkInterfaces();
	NetworkInterface *pNode = pHead;
	int maxMask = 0;
	IPV4 best = src;
	Byte *pSrc = (Byte *) &src;
	while (pNode) {
		Byte *pAddr = (Byte *) &(pNode->ip);
		int mask = evalMask(pSrc, pAddr);
		if (mask > maxMask) {
			maxMask = mask;
			best = *((IPV4 *) pAddr);
		}
		pNode = pNode->pNext;
	}
	freeNetworkInterfaces(pHead);
	return best;
}

inline bool isLoopBack(IPV4 ip) {
	Byte *pBytes = (Byte *) &ip;
	return pBytes[0] == 0x7F;
}

bool sendMsg(CMessage *pMsg, NETADDR *pTo) {
	const DWord bufSize = DNS_MSG_MAX_SIZE * 4;
	Byte buf[bufSize];
	int size = pMsg->Encode((Byte *) &buf, bufSize);
	// printBuf(buf, size);
	if (size > DNS_MSG_MAX_SIZE) {
		return false;
	}
	if (size > 1) {
		int n = net_udp_send(socket, pTo, (void *) &buf, size);
		return n == size;
	}
	return false;
}

bool answerWithError(CMessage *qMsg, NETADDR *sender, int responseCode) {
	// create a msg
	CMessage rMsg;
	// same id a the query
	rMsg.m_Id = qMsg->m_Id;
	// copy all the flags
	rMsg.m_Flags = qMsg->m_Flags;
	// edit some flags
	rMsg.m_Flag.responseCode = responseCode;
	rMsg.m_Flag.recursionAvailable = pConfig->IsRecursionAvailable() ? 1 : 0;
	rMsg.m_Flag.truncationFlag = 0;
	rMsg.m_Flag.authoritativeAnswerFlag = 1;
	rMsg.m_Flag.isAnswer = 1;
	// point to the queries instead of copying them
	rMsg.m_QueriesCount = qMsg->m_QueriesCount;
	rMsg.m_Queries = qMsg->m_Queries;
	// send the msg
	bool ok = sendMsg(&rMsg, sender);
	// Clear the pointer. Not ours to delete.
	rMsg.m_QueriesCount = 0;
	rMsg.m_Queries = 0;
	return ok;
}

bool answerWithLocalData(CMessage *qMsg, NETADDR *client, CLocalRecord *pRec) {
	logInfo(LOG_DNS, 0x00B); // Answering From Local Records
	// create a msg
	CMessage rMsg;
	// same id a the query
	rMsg.m_Id = qMsg->m_Id;
	// edit some flags
	rMsg.m_Flag.responseCode = RESPONSE_CODE_NO_ERROR;
	rMsg.m_Flag.recursionAvailable = pConfig->IsRecursionAvailable() ? 1 : 0;
	rMsg.m_Flag.authoritativeAnswerFlag = 1;
	rMsg.m_Flag.isAnswer = 1;
	// point to the queries instead of copying them
	rMsg.m_QueriesCount = qMsg->m_QueriesCount;
	rMsg.m_Queries = qMsg->m_Queries;
	// build the records
	if (pRec) {
		DWord count = pRec->GetIPsCount();
		rMsg.m_AnswerRecordsCount = (Word) count;
		NEW_ARR(rMsg.m_AnswerRecords, CResourceRecord *, count);
		for (DWord i = 0; i < count; i++) {
			NEW_OBJ(rMsg.m_AnswerRecords[i], CResourceRecord, (pRec, i));
		}
		if (count == 1 && isLoopBack(pRec->GetIP(0))) {
			*((IPV4*) (rMsg.m_AnswerRecords[0]->GetData())) = findBestSubnet(PNETADDR_TO_IPV4(client));
		}
	}
	// send the msg
	// printMsg(&rMsg);
	bool ok = sendMsg(&rMsg, client);
	// Clean our shit
	// the msg will delete itself
	// Clear the pointer. Not ours to delete.
	rMsg.m_QueriesCount = 0;
	rMsg.m_Queries = 0;
	return ok;
}

bool answerFromCache(CMessage *qMsg, NETADDR *client, CCacheNode *pNode) {
	logInfo(LOG_DNS, 0x00C); // Answering From Cache
	// create a msg
	CMessage rMsg;
	// same id a the query
	rMsg.m_Id = qMsg->m_Id;
	// edit some flags
	rMsg.m_Flag.responseCode = RESPONSE_CODE_NO_ERROR;
	rMsg.m_Flag.recursionAvailable = pConfig->IsRecursionAvailable() ? 1 : 0;
	rMsg.m_Flag.authoritativeAnswerFlag = 1;
	rMsg.m_Flag.isAnswer = 1;
	// point to the queries instead of copying them
	rMsg.m_QueriesCount = qMsg->m_QueriesCount;
	rMsg.m_Queries = qMsg->m_Queries;
	// build the records
	rMsg.m_AnswerRecordsCount = pNode->m_Size1;
	rMsg.m_AuthorityRecordsCount = pNode->m_Size2;
	rMsg.m_AdditionalRecordsCount = pNode->m_Size3;
	rMsg.m_AnswerRecords = pNode->m_Size1 ? (pNode->m_ppRecords) : 0;
	rMsg.m_AuthorityRecords = pNode->m_Size2 ? (pNode->m_ppRecords + pNode->m_Size1) : 0;
	rMsg.m_AdditionalRecords = pNode->m_Size3 ? (pNode->m_ppRecords + pNode->m_Size1 + pNode->m_Size2) : 0;
	// send msg
	bool ok = sendMsg(&rMsg, client);
	// Clean our shit
	// the msg will delete itself
	// Clear the pointer. Not ours to delete.
	rMsg.m_QueriesCount = 0;
	rMsg.m_Queries = 0;
	rMsg.m_AnswerRecordsCount = 0;
	rMsg.m_AuthorityRecordsCount = 0;
	rMsg.m_AdditionalRecordsCount = 0;
	rMsg.m_AnswerRecords = 0;
	rMsg.m_AuthorityRecords = 0;
	rMsg.m_AdditionalRecords = 0;
	return ok;
}

void handleQueryTimeout(DWord id, PendingQuery *pQuery) {
	// Query Timedout: id
	char idStr[8];
	sprintf(idStr, "%d", id);
	logInfo(LOG_DNS, 0x00D, idStr); // Query timedout
}

bool forwardMsg(CMessage *pMsg, NETADDR *pFrom, CSecondaryServer *pSrv) {
	IPV4 from = PNETADDR_TO_IPV4(pFrom);
	DWord id = pQueue->Enqueue(pMsg->m_Id, from, pFrom->port, pSrv, pMsg->m_Queries[0]);
	char newIdStr[8];
	sprintf(newIdStr, "%d", id);
	if (id != INVALID_QUEUE_ID) {
		IPV4 ip4To = PNETADDR_TO_IPV4(pSrv->GetAddress());
		IPV4_TO_STR(toStr, pToStr, ip4To);
		logInfo(LOG_DNS, 0x00E, pMsg->m_Queries[0]->GetName(), pToStr, newIdStr); // Forwarding
		DWord oldId = pMsg->m_Id;
		pMsg->m_Id = id;
		bool sent = sendMsg(pMsg, pSrv->GetAddress());
		pMsg->m_Id = oldId;
		pSrv->MessageSent();
		return sent;
	} else {
		logInfo(LOG_DNS, 0x00F); // Server Overloaded
		return false;
	}
}

CSecondaryServer *getBestServer() {
	DWord bestScore = 0xFFFFFFFF;
	QWord bestSent = 0xFFFFFFFFFFFFFFFFL;
	CSecondaryServer *bestSrv = 0;
	//
	DWord count = pConfig->GetServersCount();
	//
	DWord score;
	CSecondaryServer *srv;
	for (DWord i = 0; i < count; i++) {
		srv = pConfig->GetServer(i);
		score = srv->DelayScore();
		if (score < bestScore) {
			bestScore = score;
			bestSrv = srv;
			bestSent = srv->GetSentCount();
		} else if (score == bestScore) {
			QWord sent = srv->GetSentCount();
			if (sent < bestSent) {
				bestScore = score;
				bestSrv = srv;
				bestSent = sent;
			}
		}
	}
	return bestSrv;
}

void cache(CMessage *pMsg) {
	DWord count = pMsg->m_AnswerRecordsCount + pMsg->m_AuthorityRecordsCount + pMsg->m_AdditionalRecordsCount;
	if (!count) return;
	CResourceRecord **NEW_ARR(rrs, CResourceRecord *, count);
	DWord i = 0;
#define _CACHE_COPY_RR(array, size) for (DWord j = 0; j < size; j++) { rrs[i++] = array[j]; }
	_CACHE_COPY_RR(pMsg->m_AnswerRecords, pMsg->m_AnswerRecordsCount)
	_CACHE_COPY_RR(pMsg->m_AuthorityRecords, pMsg->m_AuthorityRecordsCount)
	_CACHE_COPY_RR(pMsg->m_AdditionalRecords, pMsg->m_AdditionalRecordsCount)
	pCache->Save(pMsg->m_Queries[0]->GetName(), rrs, pMsg->m_AnswerRecordsCount, pMsg->m_AuthorityRecordsCount, pMsg->m_AdditionalRecordsCount);
	DELETE_ARR(rrs);
}

inline bool handleIPv4Query(CMessage *pMsg, NETADDR *pFrom) {
	IPV4 ipv4From = PNETADDR_TO_IPV4(pFrom);
	IPV4_TO_STR(fromStr, pFromStr, ipv4From);
	logInfo(LOG_DNS, 0x010, pFromStr, pMsg->m_Queries[0]->GetName()); // Query
	// try locally, else forward
	CLocalRecord *pLocalRecord = pConfig->GetLocalRecord(
		pMsg->m_Queries[0]->GetName()
	);
	//
	if (pLocalRecord || !pMsg->m_Flag.recursionDesired || !pConfig->IsRecursionAvailable()) {
		return answerWithLocalData(pMsg, pFrom, pLocalRecord);
	}
	CCacheNode *pNode = pCache->Get(pMsg->m_Queries[0]->GetName());
	if (pNode) {
		return answerFromCache(pMsg, pFrom, pNode);
	}
	CSecondaryServer *pSrv = getBestServer();
	if (pSrv) {
		return forwardMsg(pMsg, pFrom, pSrv);
	} else {
		// should never happen
	}
	return false;
}

bool handleQuery(CMessage *pMsg, NETADDR *pFrom) {
	if (pMsg->m_Flag.responseCode != RESPONSE_CODE_NO_ERROR) {
		// [bad msg] Query with error response code
		return false;
	}
	switch (pMsg->m_Queries[0]->GetRecordType()) {
		case RECORD_TYPE_IPv4:
			return handleIPv4Query(pMsg, pFrom);
	}
	return false;
}

bool handleAnswer(CMessage *pMsg, NETADDR *pFrom) {
	DWord waitTimeMs = 0;
	IPV4 from = PNETADDR_TO_IPV4(pFrom);
	PendingQuery *pQuery = pQueue->Remove(pMsg->m_Id, &waitTimeMs);
	char msgIdStr[8];
	sprintf(msgIdStr, "%d", pMsg->m_Id);
	if (pQuery && from == PNETADDR_TO_IPV4(pQuery->m_pServer->GetAddress()) && str_comp(pQuery->m_pName, pMsg->m_Queries[0]->GetName()) == 0) {
		pQuery->m_pServer->MessageReceived(waitTimeMs);
		//
		IPV4_TO_STR(fromStr, pFromStr, from);
		char delayStr[16];
		sprintf(delayStr, "%d", waitTimeMs);
		logInfo(LOG_DNS, 0x011, pFromStr, msgIdStr, delayStr); // Answer
		//
		IPV4_TO_NETADDR(client, pQuery->m_SrcIp, pQuery->m_SrcPort);
		bool result;
		pMsg->SanitizeRecords(recordFilter);
		pMsg->m_Id = pQuery->m_SrcId;
		result = sendMsg(pMsg, &client);
		if (pMsg->m_Flag.responseCode == RESPONSE_CODE_NO_ERROR) {
			cache(pMsg);
		}
		pQueue->Release(pQuery);
		return result;
	} else {
		logDebug(LOG_DNS, 0x012, msgIdStr); // Unknown Answer
		return true;
	}
}

static void handleMsg(void *msgBuf, DWord size, NETADDR *pFrom) {
	// todo: if 2 msg with the same id came from the same client, drop the 2nd.
	CMessage msg;
	if (!msg.Decode(msgBuf, size)) {
		logDebug(LOG_DNS, 0x013); // Failed To Decode Msg
	#ifdef DEBUG_CONF
		printBuf((Byte *) msgBuf, size);
	#endif
		return;
	}
	if (msg.m_Flag.truncationFlag){
		// [bad msg] Msg Truncated
		return;
	}
	if (msg.m_Flag.opcode != OPERATION_CODE_DEFAULT) {
		// [bad msg] Msg Opcode Not Supported
		answerWithError(&msg, pFrom, RESPONSE_CODE_NOT_IMPLEMENTED);
		return;
	}
	if (msg.m_QueriesCount != 1) {
		answerWithError(&msg, pFrom, RESPONSE_CODE_REFUSED);
		char cntStr[8];
		sprintf(cntStr, "%d", msg.m_QueriesCount);
		logInfo(LOG_DNS, 0x014, cntStr); // Refusing Because Queries Count Is Not One
		return;
	}
	if (msg.m_Queries[0]->GetRecordClass() != RECORD_CLASS_TCP_IP) {
		// "[bad msg] Query class not supported"
		answerWithError(&msg, pFrom, RESPONSE_CODE_NOT_IMPLEMENTED);
		return;
	}
	if (!isRecordTypeSupported(msg.m_Queries[0]->GetRecordType())) {
		answerWithError(&msg, pFrom, RESPONSE_CODE_NOT_IMPLEMENTED);
		char typStr[8];
		sprintf(typStr, "%d", msg.m_Queries[0]->GetRecordType());
		logDebug(LOG_DNS, 0x015, typStr); // Refusing
		return;
	}
	if (isBlackListed(msg.m_Queries[0]->GetName())) {
		answerWithError(&msg, pFrom, RESPONSE_CODE_REFUSED);
		logInfo(LOG_DNS, 0x016, msg.m_Queries[0]->GetName()); // Refusing
		return;
	}
	if (msg.m_Flag.isAnswer) {
		handleAnswer(&msg, pFrom);
	} else {
		handleQuery(&msg, pFrom);
	}
}

void dns::run(CConfigFile *pConfigFile) {
	running = 1;
	logInfo(LOG_DNS, 0x017); // Starting

	pConfig = pConfigFile;
	NEW_OBJ(pCache, CCache, ());
	NEW_OBJ(pQueue, CDnsQueue, ());

	// shit i need
	NETADDR address;
	mem_zero((void *) &address, sizeof(NETADDR));

	address.type = NETTYPE_IPV4;
	address.port = SERVER_PORT;

	char portStr[16];
	sprintf(portStr, "%d", SERVER_PORT);

	while (running) {
		socket = net_udp_create(address, 0);
		if (socket.type) {
			break;
		}
		logError(LOG_DNS, 0x018, portStr); // Failed To Open Port
		thread_sleep(5000); // Retry in 5 seconds...
	}
	logInfo(LOG_DNS, 0x019, portStr); // Port Open

	// init the recv buffer
	const DWord bufSize = DNS_MSG_MAX_SIZE * 4;
	Byte buf[bufSize];
	void *pBuf = (void *) &buf;

	int count; // bytes received
	NETADDR from; // sender address

	DWord timer = 0;

	while (running) {
		timer += SOCKET_CHECK_DELAY;
		count = net_udp_recv(socket, &from, pBuf, bufSize);
		if (count <= 0) {
			// Clean up & sleep if you find the time.
			// If the server does not stop, not much time
			// will pass for shit to expire anyway
			if (timer > CLEAN_UP_DELAY) {
				timer = 0;
				pCache->RemoveExpired();
				pQueue->ForEachExpired(CSecondaryServer::c_TimeOut, handleQueryTimeout);
			}
			thread_sleep(SOCKET_CHECK_DELAY);
			continue;
		}
		if (count > DNS_MSG_MAX_SIZE) {
			// msg too large
			continue;
		}
		handleMsg(pBuf, (DWord) count, &from);
	}
	pConfig = 0;
	delete pCache;
	delete pQueue;
	running = 0;
}

