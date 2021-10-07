#ifndef _LOCAL_RECORD_
#define _LOCAL_RECORD_

#include "../../util/util.h"

namespace dns {

	class CLocalRecord
	{
		public:
			CLocalRecord(char *name, IPV4 *ips, DWord ipsCount);
			virtual ~CLocalRecord();
			DWord GetIPsCount() { return m_pIpsCount; };
			char *GetName() { return m_pName; };
			IPV4 GetIP(DWord i) { return m_pIps[i]; }
			void ToString(char *str);

		private:
			char *m_pName;
			DWord m_pIpsCount;
			IPV4 *m_pIps;

	};

}

#endif
