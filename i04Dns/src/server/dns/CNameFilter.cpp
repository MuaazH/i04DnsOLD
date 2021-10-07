#include "CNameFilter.h"
#include "../../../../mlib/system.h"
#include <stdio.h>
#include "../../base/heap_dbg.h"

#define ASTERISK '*'

using namespace dns;

CNameFilter::CNameFilter(char *str) {
	// init
	DWord len = (DWord) str_length(str);
	NEW_ARR(m_pText, char, len + 1);
	mem_copy((void *) m_pText, (void *) str, len + 1);
}

CNameFilter::~CNameFilter() {
	if (m_pText) {
		DELETE_ARR(m_pText);
	}
}

void CNameFilter::ToString(char *str) {
	if (m_pText) {
		sprintf(str, "(Name-Filter %s)", m_pText);
	} else {
		str[0] = 0;
	}
}

bool CNameFilter::Match(char *name) {
	if (!name || !m_pText) {
		return false;
	}
	DWord i = 0;
	DWord j = 0;
	DWord lastStar = 0xFFFFFFFF;
	while (name[i]) {
		if (!m_pText[j]) {
			if (!name[i]) { // name ended with the filter, they match
				return true;
			}
			if (j == lastStar) {
				return true; // filter ends with a star, the end of the name does not matter
			}
			return false; // the name is longer than the filter
		}
		// if star is found, mark the position
		// return to it when miss-match is found
		if (m_pText[j] == ASTERISK) {
			j++;
			lastStar = j;
			continue; // check if this is the end
		}
		if (m_pText[j] == name[i]) {
			j++;
		} else {
			if (lastStar > j) {
				break; // no star, and not the same text, leave!
			} else {
				j = lastStar;  // if a star was found, go back to it
			}
		}
		i++;
	}
	// if the end of the filter is stars, then it the text maches the filter
	while (m_pText[j]) {
		if (m_pText[j] != ASTERISK) {
			return false;
		}
		j++;
	}
	return true;
}

