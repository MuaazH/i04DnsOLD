#ifndef _DNS_INTERNAL_QUERY_
#define _DNS_INTERNAL_QUERY_

namespace dns {

	class CQuery {

		public:
			CQuery(char *name, DWord nameLen, Word recordType, Word recordClass) {
				NEW_ARR(m_pName, char, nameLen + 1);
				mem_copy(m_pName, name, nameLen);
				m_pName[nameLen] = 0;
				m_RecordType = recordType;
				m_RecordClass = recordClass;
			}
			
			~CQuery() {
				if (m_pName) {
					DELETE_ARR(m_pName);
				}
			}
			
			CQuery *Copy() {
				CQuery *NEW_OBJ(pQuery, CQuery, (m_pName, str_length(m_pName), m_RecordType, m_RecordClass));
				return pQuery;
			}
			
			Word GetRecordType() { return m_RecordType; }
			
			Word GetRecordClass() { return m_RecordClass; }
			
			char *GetName() { return m_pName; };
			
			bool Encode(CBufferOutputStream *out, RedBlackBST<char *, DWord> *labelsOffsets) {
				DWord len = dns::EncodeSN((char *) out->GetBuf(), out->GetSize(), out->SizeLeft(), m_pName, labelsOffsets);
				if (!len) return false;
				if (!out->Skip(len)) return false;
				if (!out->WriteWord(m_RecordType)) return false;
				if (!out->WriteWord(m_RecordClass)) return false;
				return true;
			}

			static bool DecodeQueries(CBufferInputStream *in, CQuery*** queries, DWord qCount) {
				NEW_ARR(*queries, CQuery *, qCount);
				mem_zero((void *) *queries, qCount * sizeof(CQuery*));
				const DWord maxNameLen = 0x100;
				char buf[maxNameLen + 1];
				char *pBuf = (char *) &buf;
				int nameLen = 0;
				DWord endIndex = 0;
				Word qClass, qType;
				for (DWord i = 0; i < qCount; i++) {
					nameLen = dns::DecodeSN(pBuf, in->GetBuf(), in->GetPos(), in->GetSize(), maxNameLen, &endIndex);
					if (nameLen < 0) return false;
					in->Skip(endIndex - in->GetPos());
					if (!in->ReadWord(&qType)) return false;
					if (!in->ReadWord(&qClass)) return false;
					NEW_OBJ((*queries)[i], CQuery, (pBuf, nameLen, qType, qClass));
				}
				return true;
			}
			
		private:
			char *m_pName;
			Word m_RecordType;
			Word m_RecordClass;

	};

}

#endif

