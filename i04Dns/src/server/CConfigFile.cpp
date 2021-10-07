#include "CConfigFile.h"
#include "../../../mlib/system.h"
#include "logger.h"
#include <quicksort.h>
#include "../base/heap_dbg.h"

#include <stdio.h>

enum {
	CMD_NULL = 0,
	CMD_DNS_SERVERS,
	CMD_DNS_IPV4_RECORDS,
	CMD_DNS_NAME_WHITELIST,
	CMD_DNS_NAME_BLACKLIST
};

// UTIL

int parsePort(char *pLine) {
	int val = 0;
	int dataFound = 0;
	int spaceAfterData = 0;
	for(int i = 0; pLine[i]; i++) {
		if (pLine[i] <= ' ') {
			if (dataFound) {
				spaceAfterData++;
			}
			continue;
		}
		if (spaceAfterData) {
			return 0;
		}
		char c = pLine[i];
		if ('0' <= c && c <= '9') {
			dataFound++;
			val *= 10;
			val += c - '0';
		} else {
			return 0;
		}
	}
	if (!dataFound) {
		return 0;
	}
	return val;
}

int filterCmp(dns::CNameFilter *pA, dns::CNameFilter *pB) {
	char *pAText = pA->GetText();
	char *pBText = pB->GetText();
	return strCmp(pAText, pBText);
}

void deleteLocalName(char *pName, dns::CLocalRecord *pRec) {
	DELETE_OBJ(pRec);
}

// CConfigFile

CConfigFile::CConfigFile(char *pFileName)
{
    DWord len = str_length(pFileName);
	NEW_ARR(m_pFileName, char, len + 1);
	mem_copy((void *) m_pFileName, (void *) pFileName, len + 1);
	// init
	m_ServersCount = 0;
	m_Servers = 0;
	//
	m_WhiteListSize = 0;
	m_WhiteList = 0;
	//
	m_BlackListSize = 0;
	m_BlackList = 0;
	//
	m_pLocalRecords = 0;
	//
	m_Lock = lock_create();
}

CConfigFile::~CConfigFile()
{
    if (m_pFileName) {
        DELETE_ARR(m_pFileName);
    }
	Clear();
	lock_destroy(m_Lock);
}

void CConfigFile::Clear() {
	if (m_Servers) {
		for (DWord i = 0; i  < m_ServersCount; i++) {
			if (m_Servers[i]) {
				DELETE_OBJ(m_Servers[i]);
			}
		}
		DELETE_ARR(m_Servers);
		m_Servers = 0;
	}
	m_ServersCount = 0;
	//
	if (m_WhiteList) {
		for (DWord i = 0; i < m_WhiteListSize; i++) {
			if (m_WhiteList[i]) {
				DELETE_OBJ(m_WhiteList[i]);
			}
		}
		DELETE_ARR(m_WhiteList);
		m_WhiteList = 0;
	}
	m_WhiteListSize = 0;
	//
	if (m_BlackList) {
		for (DWord i = 0; i < m_BlackListSize; i++) {
			if (m_BlackList[i]) {
				DELETE_OBJ(m_BlackList[i]);
			}
		}
		DELETE_ARR(m_BlackList);
		m_BlackList = 0;
	}
	m_BlackListSize = 0;
	//
	if (m_pLocalRecords) {
		m_pLocalRecords->ForEach(deleteLocalName);
		DELETE_OBJ(m_pLocalRecords);
		m_pLocalRecords = 0;
	}
}

dns::CSecondaryServer *CConfigFile::DnsServerFromString(char *pStr) {
	const int bufLen = 0x20;
	char *NEW_ARR(buf, char, bufLen);

	int len = str_length(pStr);
	mem_copy(buf, pStr, len + 1);

	str_append(buf, ":53", bufLen);

	NETADDR a;
	int error = net_addr_from_str(&a, buf);

	DELETE_ARR(buf);

	if (!error) {
		dns::CSecondaryServer *NEW_OBJ(pSrv, dns::CSecondaryServer, (&a));
		return pSrv;
	}
	return 0;
}

dns::CLocalRecord *CConfigFile::LocalRecordFromString(char *pStr) {
	dns::CLocalRecord *pLocalRecord = 0;
	DWord i = 0;
	while (pStr[i] && pStr[i] != ' ') {
		i++;
	}
	if (i > 0) {
		i++;
		char *NEW_ARR(name, char, i);
		mem_copy((void *) name, (void *) pStr, i);
		name[i - 1] = 0;
		if (pStr[i]) { // the string does not end here
			bool fail = false;
			ArrayDeque<IPV4> ips;
			IPV4 ip;
			DWord ipLen;
			while(pStr[i]) {
				if (pStr[i] == ' ') {
					i++;
					continue;
				}
				if (ip4FromString(pStr + i, &ip, &ipLen)) {
					fail = true;
					break;
				}
				i += ipLen;
				ips.Add(ip);
			}
			if (!fail) {
				DWord arraySize = ips.Size();
				IPV4 *NEW_ARR(ipArray, IPV4, arraySize);
				for (DWord a = 0; a < arraySize; a++) {
					ipArray[a] = ips.At(a);
				}
				NEW_OBJ(pLocalRecord, dns::CLocalRecord, (name, ipArray, arraySize));
				DELETE_ARR(ipArray);
				ips.Clear();
			}
		}
		DELETE_ARR(name);
	}
	return pLocalRecord;
}

bool CConfigFile::TestCmd(const char *pCmdStr, char *pLine) {
	int i = 0;
	const char *pShortCmdStr = pCmdStr + 4; // +4 = skip CMD_
	do {
		char x = CHAR_LOWERCASE(pShortCmdStr[i]);
		if (x != pLine[i]) {
			if (!x && pLine[i] == ':' && !pLine[i + 1]) {
				return true;
			}
			break;
		}
		if (!x) break;
		i++;
	} while (true);
	return false;
}

int CConfigFile::ExecLine(
							int cmd,
							char *pLine,
							ArrayDeque<dns::CSecondaryServer *> *pServers,
							ArrayDeque<dns::CNameFilter *> *pWhiteList,
							ArrayDeque<dns::CNameFilter *> *pBlackList,
							RedBlackBST<char *, dns::CLocalRecord *> *pLocalRecords
					 	  ) {

#define ExecLine_STR_(x) #x
#define ExecLine_STR(x) ExecLine_STR_(x)
#define ExecLine_TEST_CMD(CMD) if (TestCmd(ExecLine_STR(CMD), pLine)) { return CMD; }

	ExecLine_TEST_CMD(CMD_DNS_SERVERS)
	ExecLine_TEST_CMD(CMD_DNS_NAME_WHITELIST)
	ExecLine_TEST_CMD(CMD_DNS_NAME_BLACKLIST)
	ExecLine_TEST_CMD(CMD_DNS_IPV4_RECORDS)

	bool valid = cmd != CMD_NULL;
	int newCmd = cmd;
	if (valid) {
		// exec
		switch (cmd) {
			// TODO: check for duplicates
			case CMD_DNS_SERVERS:
			{
				dns::CSecondaryServer *srv = DnsServerFromString(pLine);
				if (srv) {
					pServers->Add(srv);
					break; // OK
				}
				valid = false;
			}

			// TODO: check for duplicates
			case CMD_DNS_NAME_WHITELIST:
			{
				dns::CNameFilter *NEW_OBJ(pWNameFilter, dns::CNameFilter, (pLine));
				pWhiteList->Add(pWNameFilter);
				break;
			}

			// TODO: check for duplicates
			case CMD_DNS_NAME_BLACKLIST:
			{
				dns::CNameFilter *NEW_OBJ(pBNameFilter, dns::CNameFilter, (pLine));
				pBlackList->Add(pBNameFilter);
				break;
			}

			case CMD_DNS_IPV4_RECORDS:
			{
				dns::CLocalRecord *record = LocalRecordFromString(pLine);
				if (record) {
					if (pLocalRecords->Contains(record->GetName())) {
						logWarn(LOG_CONF, 0x007, record->GetName()); // Duplicate Name Ignored
						DELETE_OBJ(record);
					} else {
						pLocalRecords->Put(record->GetName(), record);
					}
					break; // OK
				}
				valid = false;
				break; // Not Valid
			}

		}
	}
	if (!valid) {
		logError(LOG_CONF, 0x008, pLine); // Invalid Config Line
	}

	return newCmd;
}

void CConfigFile::ParseBuffer(char *buf, const DWord size) {
    ArrayDeque<char *> lines;
    DWord bufEnd = size - 1;
    DWord start = 0;
    for (DWord i = 0; i < size; i++) {
        char c = buf[i];
        if (i >= bufEnd || c == '\r' || c == '\n') {
			while (start < size && (buf[start] == '\r' || buf[start] == '\n')) {
				start++;
			}
            int lineSize = i - start; // don't count the last char
            if (i >= bufEnd) {
                lineSize += 1; // unless it is not a newline char
            }
            if (lineSize > 0) {
				char *NEW_ARR(line, char, lineSize + 1);
				DWord lineEnd = lineSize + start;
                line[lineSize] = 0;
                DWord k = 0;
			 	bool lastCharIsSpace = false;
			 	bool thisCharIsSpace;
			 	for (DWord j = start; j < lineEnd; j++) {
                    if (buf[j] == ';') { // no comments
						break;
                    }
                    if (buf[j] == '\t') { // tab must become space
                        buf[j] = ' ';
                    }
                    if (k > 0 || buf[j] != ' ') { // lines don't start with a space
						thisCharIsSpace = buf[j] == ' ';
						if (!(lastCharIsSpace && thisCharIsSpace)) {
							char chr = buf[j];
		                    line[k++] = CHAR_LOWERCASE(chr);
							lastCharIsSpace = thisCharIsSpace;
	                    }
                    }
                }
                if (k > 0) {
                    if (line[k - 1] == ' ') { // remove the last char if it is space
                        k--;
                    }
                    if (k > 0) {
                        line[k] = 0;
						char *NEW_ARR(shortLine, char, k + 1);
                        shortLine[k] = 0; // The end is 0
                        for (DWord a = 0; a < k; a++) {
                            shortLine[a] = line[a];
                        }
                        lines.Add(shortLine);
                    }
                }
				DELETE_ARR(line);
            }
            start = i + 1;
        }
    }
	// init collections to save the results
    ArrayDeque<dns::CSecondaryServer *> servers;
	ArrayDeque<dns::CNameFilter *> whiteList;
	ArrayDeque<dns::CNameFilter *> blackList;
	NEW_OBJ2(m_pLocalRecords, RedBlackBST, char *, dns::CLocalRecord *, (strCmp));
	// execute the lines
    DWord lc = lines.Size();
	int cmd = CMD_NULL;
    for (DWord h = 0; h < lc; h++) {
        char *pLine = lines.At(h);
		str_sanitize_strong(pLine);
        cmd = ExecLine(cmd, pLine, &servers, &whiteList, &blackList, m_pLocalRecords);
        DELETE_ARR(pLine); // clean shit up
    }
	// init the arrays
	m_ServersCount = servers.Size();
	NEW_ARR(m_Servers, dns::CSecondaryServer *, m_ServersCount);
	for (DWord x = 0; x < m_ServersCount; x++) {
		m_Servers[x] = servers.At(x);
	}
	//
	m_WhiteListSize = whiteList.Size();
	NEW_ARR(m_WhiteList, dns::CNameFilter *, m_WhiteListSize);
	for (DWord x = 0; x < m_WhiteListSize; x++) {
		m_WhiteList[x] = whiteList.At(x);
	}
	quickSort<dns::CNameFilter *>(m_WhiteList, 0, (int) (m_WhiteListSize - 1), filterCmp);
	//
	m_BlackListSize = blackList.Size();
	NEW_ARR(m_BlackList, dns::CNameFilter *, m_BlackListSize);
	for (DWord x = 0; x < m_BlackListSize; x++) {
		m_BlackList[x] = blackList.At(x);
	}
	quickSort<dns::CNameFilter *>(m_BlackList, 0, (int) (m_BlackListSize - 1), filterCmp);
}

bool CConfigFile::Load() {
	lock_wait(m_Lock);

	Clear();

    char *buf = 0;
    DWord size = 0;
    bool success = true;

 	IOHANDLE pFile = io_open(m_pFileName, (int) IOFLAG_READ);

	if (pFile) {
		size =  (DWord) io_length(pFile);
        if (size < 1) {
            success = false;
        } else {
			NEW_ARR(buf, char, size);
            DWord read = 0;
            while (read < size) {
                DWord count = io_read(pFile, (void *)(buf + read), size - read);
                if (count < 1) { // EOF
                    success = false;
                    break;
                }
                read += count;
            }
        }
		io_close(pFile);
        if (success) {
            ParseBuffer(buf, size);
			DELETE_ARR(buf);
        }
        // end
    } else {
        success = false; // file not found or some shit like that
    }
	lock_unlock(m_Lock);
    return success;
}

bool CConfigFile::IsBlackListed(char *name) {
	lock_wait(m_Lock);
	bool b = m_BlackList && FindName(name, m_BlackList, m_BlackListSize) >= 0;
	lock_unlock(m_Lock);
	return b;
}

bool CConfigFile::IsWhiteListed(char *name) {
	lock_wait(m_Lock);
	bool b = m_WhiteList && FindName(name, m_WhiteList, m_WhiteListSize) >= 0;
	lock_unlock(m_Lock);
	return b;
}

int CConfigFile::FindName(char *name, dns::CNameFilter **list, DWord listSize) {
	for (DWord i = 0; i < listSize; i++) {
		if (list[i] && list[i]->Match(name)) {
			return (int) i;
		}
	}
	return -1;
}

dns::CLocalRecord *CConfigFile::GetLocalRecord(char *name) {
	lock_wait(m_Lock);
	dns::CLocalRecord *rec = m_pLocalRecords ? m_pLocalRecords->Get(name) : 0;
	lock_unlock(m_Lock);
	return rec;
}

bool CConfigFile::IsRecursionAvailable() {
	lock_wait(m_Lock);
	bool b = m_Servers && m_ServersCount > 0;
	lock_unlock(m_Lock);
	return b;
}

dns::CSecondaryServer *CConfigFile::GetServer(DWord index) {
	lock_wait(m_Lock);
	dns::CSecondaryServer *s = index < m_ServersCount ? m_Servers[index] : 0;
	lock_unlock(m_Lock);
	return s;
}

DWord CConfigFile::GetServersCount() {
	lock_wait(m_Lock);
	DWord c = m_ServersCount;
	lock_unlock(m_Lock);
	return c;
}

#define GET_STR_FIELD(method, field) void CConfigFile:: method (char *pBuf) { \
	lock_wait(m_Lock);                                                        \
	if ( field ) {                                                            \
		int len = str_length( field ) + 1;                                    \
		mem_copy((void *) pBuf, (void *) field, len);                         \
	} else {                                                                  \
		pBuf[0] = 0;                                                          \
	}                                                                         \
	lock_unlock(m_Lock);                                                      \
}
