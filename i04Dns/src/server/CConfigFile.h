#ifndef CConfigFile_H
#define CConfigFile_H

#include "../util/util.h"
#include <ArrayDeque.h>
#include "dns/CSecondaryServer.h"
#include "dns/CNameFilter.h"
#include "dns/CLocalRecord.h"
#include <RedBlackBST.h>

#define CONFIGFILE_MAX_STR_LEN 128

class CConfigFile
{
    public:
        CConfigFile(char *pFileName);
        virtual ~CConfigFile();

        bool Load();
		// DNS
		bool IsRecursionAvailable();
		bool IsBlackListed(char *pName);
		bool IsWhiteListed(char *pName);
		dns::CLocalRecord *GetLocalRecord(char *pName);
		dns::CSecondaryServer *GetServer(DWord index);
		DWord GetServersCount();
		void GetWebInterfaceFileName(char *pBuf);
		void GetWebInterfaceAdmin(char *pBuf);
		void GetWebInterfaceAdminPassword(char *pBuf);

    private:

		void Clear();
		dns::CSecondaryServer *DnsServerFromString(char *pStr);
		dns::CLocalRecord *LocalRecordFromString(char *pStr);
		//
		bool TestCmd(const char *pCmdStr, char *pLine);
		int ExecLine(
						int cmd,
						char *line,
						ArrayDeque<dns::CSecondaryServer *> *pServers,
						ArrayDeque<dns::CNameFilter *> *pWhiteList,
						ArrayDeque<dns::CNameFilter *> *pBlackList,
						RedBlackBST<char *, dns::CLocalRecord *> *pLocalRecords
					 );
        void ParseBuffer(char *buf, DWord size);
		int FindName(char *name, dns::CNameFilter **list, DWord listSize);

        char					*m_pFileName;
		//
		DWord               	m_ServersCount;
		dns::CSecondaryServer	**m_Servers;
		//
		DWord					m_WhiteListSize;
		dns::CNameFilter		**m_WhiteList;
		//
		DWord					m_BlackListSize;
		dns::CNameFilter		**m_BlackList;
		//
		RedBlackBST<char *, dns::CLocalRecord *> *m_pLocalRecords;
		//
		LOCK                	m_Lock;
};

#endif // CConfigFile_H

