#ifndef _DNS_INTERNAL_PROTOCOL_
#define _DNS_INTERNAL_PROTOCOL_

enum
{
	RECORD_CLASS_TCP_IP = 1,
	RECORD_CLASS_UNKNOWN = -1
};

enum
{
	RECORD_TYPE_IPv4 = 1,
	RECORD_TYPE_NS = 2,
	RECORD_TYPE_CNAME = 5,
	RECORD_TYPE_SOA = 6,
	RECORD_TYPE_PTR = 12,
	RECORD_TYPE_MX = 15,
	RECORD_TYPE_TXT = 16,
	RECORD_TYPE_RP = 17,
	RECORD_TYPE_AFSDB = 18,
	RECORD_TYPE_SIG = 24,
	RECORD_TYPE_KEY = 25,
	RECORD_TYPE_IPV6 = 28,
	RECORD_TYPE_LOC = 29,
	RECORD_TYPE_SRV = 33,
	RECORD_TYPE_NAPTR = 35,
	RECORD_TYPE_KX = 36,
	RECORD_TYPE_CERT = 37,
	RECORD_TYPE_DNAME = 39,
	RECORD_TYPE_APL = 42,
	RECORD_TYPE_DS = 43,
	RECORD_TYPE_SSHFP = 44,
	RECORD_TYPE_IPSECKEY = 45,
	RECORD_TYPE_RRSIG = 46,
	RECORD_TYPE_NSEC = 47,
	RECORD_TYPE_DNSEKEY = 48,
	RECORD_TYPE_DHCID = 49,
	RECORD_TYPE_NSEC3 = 50,
	RECORD_TYPE_NSEC3PARAM = 51,
	RECORD_TYPE_HIP = 55,
	RECORD_TYPE_SPF = 99,
	RECORD_TYPE_TKEY = 249,
	RECORD_TYPE_TSIG = 250,
	RECORD_TYPE_ALL_CHACHED = 255,
	RECORD_TYPE_TA = 32768,
	RECORD_TYPE_DLV = 32769,
	RECORD_TYPE_UNKNOWN = -1
};

enum
{
    OPERATION_CODE_DEFAULT = 0,
    OPERATION_CODE_UNKNOWN = -1
};

enum
{
	RESPONSE_CODE_NO_ERROR = 0,
	RESPONSE_CODE_FORMAT_ERROR = 1,
	RESPONSE_CODE_SERVER_ERROR = 2,
	RESPONSE_CODE_NAME_ERROR = 3,
	RESPONSE_CODE_NOT_IMPLEMENTED = 4,
	RESPONSE_CODE_REFUSED = 5,
	RESPONSE_CODE_NOT_AUTH = 8,
	RESPONSE_CODE_NOT_IN_ZONE = 9,
	RESPONSE_CODE_UNKNOWN = -1
};

namespace dns {

	// returns the length of the decoded name, -1 = error
	DWord DecodeSN(char *dst, Byte *src, DWord srcOffset, DWord srcLen, DWord maxLen, DWord *endIndex) {
		DWord len = 0;
		DWord index = srcOffset;
		DWord labelLen;
		bool jmpFound = false;
		while (true) {
			if (index > srcLen) {
				return -1; // index out of bounds exception
			}
			labelLen = src[index];
			index++;
			if (!labelLen) {
				if (!jmpFound) {
					*endIndex = index;
				}
				break; // string end
			}
			if (labelLen < 0xC0 && 0x3F < labelLen) {
				return -1; // not a valid label length | not a valid pointer byte
			}
			if (labelLen >= 0xC0) {
				labelLen = (labelLen & 0x3F) << 8;
				labelLen |= src[index];
				if (!jmpFound) {
					*endIndex = index + 1;
					jmpFound = true;
				}
				index = labelLen; // it is not length, it is index in the array.
				continue;
			}
			if (index + labelLen >= srcLen || len + labelLen >= maxLen) {
				return -1; // index out of bounds exception
			}
			if (len) { // names don't start with a dot
				dst[len++] = '.';
			}
			for (DWord i = 0; i < labelLen; i++) {
				char chr = (char) src[index++];
				dst[len++] = CHAR_LOWERCASE(chr);
			}
		}
		dst[len] = 0; // terminating null
		return len;
	}

	// returns the length of the compressed name, 0 = error
	DWord EncodeSN(char *dst, DWord dOff, DWord dLen, char *src, RedBlackBST<char *, DWord> *labelsOffsets) {
		DWord len = 0;
		char *ddst = dst + dOff; // direct dest pointer
		char *name = src;
		while (true) {
			DWord offset = labelsOffsets->Get(name);
			if (!offset || offset >= dOff) { // offset < dOff, you can't use your own label (infinite loop)
				// calculate the len of the label
				DWord labelLen = 0;
				while (name[labelLen] && name[labelLen] != '.' && labelLen < 0x40)
					labelLen++;
				// check the len
				if (labelLen > 0x3F) {
					return 0; // label too long
				}
				// add it to the map
				labelsOffsets->Put(name, dOff + len);
				// check the bounds
				len += labelLen + 1;
				if (len + 1 > dLen) {
					return 0; // dst index out of bounds
				}
				// write label to ddst
				ddst[0] = (char) labelLen;
				ddst++;
				mem_copy((void *) ddst, (void *) name, labelLen);
				ddst += labelLen;
				name += labelLen;
				//
				if (!name[0]) { // if the name ended
					len++;
					if (len > dLen) {
						return 0;
					}
					ddst[0] = 0; // the end
					break;
				} else { // name does not end here
					name++; // skip the '.'
				}
			} else {
				len += 2;
				if (len > dLen) {
					return 0; // dst index out of bounds
				}
				Word ptr = (Word) offset | 0xC000;
				ddst[0] = (char) ((ptr >> 8) & 0xFF);
				ddst[1] = (char) (ptr & 0xFF);
				break;
			}
		}
		return len;
	}

}

#endif

