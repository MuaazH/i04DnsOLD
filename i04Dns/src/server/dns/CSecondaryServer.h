#ifndef _CDNS_SERVER_H_
#define _CDNS_SERVER_H_

#include "../../../../mlib/system.h"
#include "../../util/util.h"

#define DELAY_HISTORY_SIZE 0x14

namespace dns {

	class CSecondaryServer
	{
		public:
			CSecondaryServer(NETADDR *addr);
			virtual ~CSecondaryServer();

			NETADDR *GetAddress();

			void MessageSent();
			void MessageReceived(DWord delay);

			DWord DelayScore();

			QWord GetSentCount();

			void ToString(char *str);

			static const DWord c_ToStrLen = 0x20;
			static const DWord c_TimeOut = 7000;

		private:

			NETADDR m_Address;
			DWord m_DelayHistory[DELAY_HISTORY_SIZE];
			DWord m_DelayHistoryIndex;
			QWord m_Sent;

			void SetNextDelay(DWord delay);
			void SetDelay(DWord delay);

	};

}

#endif
