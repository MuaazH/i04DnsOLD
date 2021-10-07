#ifndef _DNS_INTERNAL_REOURCE_RECORD_
#define _DNS_INTERNAL_REOURCE_RECORD_

#define DEFAULT_TIME_TO_LIVE 60

namespace dns {

	class CResourceRecord {

		public:

	#ifdef _LOCAL_RECORD_
			CResourceRecord(CLocalRecord *rec, DWord index) {
				Init(rec->GetName(), str_length(rec->GetName()), RECORD_TYPE_IPv4, RECORD_CLASS_TCP_IP);
				m_DataSize = 4;
				NEW_ARR(m_pData, Byte, m_DataSize);
				*((IPV4 *) m_pData) = rec->GetIP(index);
			}
	#endif

			CResourceRecord(char *name, DWord nameLen, Word recordType, Word recordClass) {
				Init(name, nameLen, recordType, recordClass);
			}

			~CResourceRecord() {
				if (m_pName) {
					DELETE_ARR(m_pName);
				}
				if (m_pData) {
					DELETE_ARR(m_pData);
				}
			}

			CResourceRecord *Copy() {
				CResourceRecord *NEW_OBJ(pCopy, CResourceRecord, (m_pName, str_length(m_pName), m_RecordType, m_RecordClass));
				pCopy->m_expirationTime = m_expirationTime;
				pCopy->SetDataSize(m_DataSize);
				mem_copy(pCopy->m_pData, m_pData, m_DataSize);
				return pCopy;
			}
			
			Word GetRecordType() { return m_RecordType; }

			Word GetRecordClass() { return m_RecordClass; }

			char *GetName() { return m_pName; }

			Byte *GetData() { return m_pData; }

			void SetDataSize(DWord size) { 
				if (m_pData) {
					DELETE_ARR(m_pData);
				}
				m_DataSize = size;
				NEW_ARR(m_pData, Byte, size);
			}

			DWord GetDataSize() { return m_DataSize; }

			void SetTimeToLive(DWord ttl) {
				m_expirationTime = time_timestamp() + (int) ttl;
			}

			int GetTimeToLive() {
				int t = m_expirationTime - time_timestamp();
				return t < 0? 0 : t;
			}

			bool isExpired() {
				return !GetTimeToLive();
			}

			bool Encode(CBufferOutputStream *out, RedBlackBST<char *, DWord> *labelsOffsets) {
				DWord len = dns::EncodeSN((char *) out->GetBuf(), out->GetSize(), out->SizeLeft(), m_pName, labelsOffsets);
				if (!len) return false;
				if (!out->Skip(len)) return false;
				if (!out->WriteWord(m_RecordType)) return false;
				if (!out->WriteWord(m_RecordClass)) return false;
				if (!out->WriteDWord((DWord) GetTimeToLive())) return false;
				if (m_RecordType == RECORD_TYPE_CNAME) {
					if (!out->Skip(2)) return false;
					len = dns::EncodeSN((char *) out->GetBuf(), out->GetSize(), out->SizeLeft(), (char *) m_pData, labelsOffsets);
					if (!len) return false;
					if (!out->Skip(-2)) return false;
					if (!out->WriteWord(len)) return false;
					if (!out->Skip(len)) return false;
				} else {
					if (!out->WriteWord(m_DataSize)) return false;
					if (!out->WriteBuf(m_pData, m_DataSize)) return false;
				}
				return true;
			}

			static bool DecodeRecords(CBufferInputStream *in, CResourceRecord ***prrs, DWord count) {
				if (!count) return true;
				NEW_ARR(*prrs, CResourceRecord *, count);
				mem_zero((void *) *prrs, count * sizeof(CResourceRecord *));
				const DWord maxNameLen = 0x100;
				char buf[maxNameLen + 1]; char *pBuf = (char *) &buf;
				int nameLen = 0;
				DWord endIndex = 0;
				Word qClass, qType;
				Word dataSize;
				DWord timeToLive;
				for (DWord i = 0; i < count; i++) {
					nameLen = dns::DecodeSN(pBuf, in->GetBuf(), in->GetPos(), in->GetSize(), maxNameLen, &endIndex);
					if (nameLen < 0) return false;
					in->Skip(endIndex - in->GetPos());
					if (!in->ReadWord(&qType)) return false;
					if (!in->ReadWord(&qClass)) return false;
					if (!in->ReadDWord(&timeToLive)) return false;
					if (!in->ReadWord(&dataSize)) return false;
					CResourceRecord *NEW_OBJ(pRec, CResourceRecord, (pBuf, nameLen, qType, qClass));
					(*prrs)[i] = pRec;
					pRec->SetTimeToLive(timeToLive);
					if (qType == RECORD_TYPE_CNAME) {
						nameLen = dns::DecodeSN(pBuf, in->GetBuf(), in->GetPos(), in->GetSize(), maxNameLen, &endIndex);
						if (nameLen < 0) return false;
						if (dataSize != endIndex - in->GetPos()) return false;
						in->Skip(dataSize);
 						pRec->SetDataSize(nameLen + 1);
						mem_copy(pRec->GetData(), pBuf, nameLen + 1);
					} else {
						// other record types, just read the data as it is
						pRec->SetDataSize(dataSize);
						if (!in->ReadBuf(pRec->GetData(), dataSize)) return false;
					}
				}
				return true;
			}
			
		private:
			void Init(char *name, DWord nameLen, Word recordType, Word recordClass) {
				NEW_ARR(m_pName, char, nameLen + 1);
				mem_copy(m_pName, name, nameLen);
				m_pName[nameLen] = 0;
				m_RecordType = recordType;
				m_RecordClass = recordClass;
				m_DataSize = 0;
				m_pData = 0;
				m_expirationTime = time_timestamp() + DEFAULT_TIME_TO_LIVE;
			}

			char *m_pName;
			Word m_RecordType;
			Word m_RecordClass;
			int m_expirationTime;
			Word m_DataSize;
			Byte *m_pData;
			
	};

}

#endif

