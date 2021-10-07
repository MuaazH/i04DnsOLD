#ifndef _DNS_CNAME_FILTER_H_
#define _DNS_CNAME_FILTER_H_

#include "../../util/util.h"

namespace dns {

	class CNameFilter
	{
		public:
			CNameFilter(char *str);
			virtual ~CNameFilter();
			void ToString(char *str);
			bool Match(char *name);
			char* GetText() { return m_pText; };

		private:
			char *m_pText;
			
	};

}

#endif
