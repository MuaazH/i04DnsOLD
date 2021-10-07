#ifndef _DNS_INTERNAL_MSG_
#define _DNS_INTERNAL_MSG_

namespace dns {

	class CMessage {

		public:

			CMessage() {
				mem_zero(this, sizeof(CMessage));
			}
			
			~CMessage() {
				FreeMemory();
			}
			
			DWord GetQueriesCount() { return m_QueriesCount; }

			CQuery **GetQueries() { return m_Queries; }

			void SetQueries(CQuery **queries, DWord count) { 
				FreeQueries();
				if (count) {
					NEW_ARR(m_Queries, CQuery *, count);
					mem_zero((void *) m_Queries, sizeof(CQuery *) * count);
					m_QueriesCount = count;
					if (queries) {
						for (DWord i = 0; i < count; i++) {
							m_Queries[i] = queries[i]->Copy();
						}
					}
				}
			}

			void SetAnswerRecordsCount(DWord count) {
				SetRecordsCount(&m_AnswerRecords, &m_AnswerRecordsCount, count);
			}
			
			void SetAuthorityRecordsCount(DWord count) {
				SetRecordsCount(&m_AuthorityRecords, &m_AuthorityRecordsCount, count);
			}
			
			void SetAdditionalRecordsCount(DWord count) {
				SetRecordsCount(&m_AdditionalRecords, &m_AdditionalRecordsCount, count);
			}

			bool Decode(void *buf, DWord size) {
				FreeMemory();
				CBufferInputStream in((Byte *) buf, size);
				// Header
				if (!in.ReadWord(&m_Id)) return false;
				if (!in.ReadWord(&m_Flags)) return false;
				Word count;
				if (!in.ReadWord(&count)) return false;
				m_QueriesCount = count;
				if (!in.ReadWord(&count)) return false;
				m_AnswerRecordsCount = count;
				if (!in.ReadWord(&count)) return false;
				m_AuthorityRecordsCount = count;
				if (!in.ReadWord(&count)) return false;
				m_AdditionalRecordsCount = count;
				// Queries
				if (!CQuery::DecodeQueries(&in, &m_Queries, m_QueriesCount)) return false;
				// Records
				if (!CResourceRecord::DecodeRecords(&in, &m_AnswerRecords, m_AnswerRecordsCount)) return false;
				if (!CResourceRecord::DecodeRecords(&in, &m_AuthorityRecords, m_AuthorityRecordsCount)) return false;
				if (!CResourceRecord::DecodeRecords(&in, &m_AdditionalRecords, m_AdditionalRecordsCount)) return false;
				return true;
			}

			DWord Encode(void *buf, DWord size) {
				CBufferOutputStream out((Byte *) buf, size);
				// Header
				if (!out.WriteWord(m_Id)) return 0;
				if (!out.WriteWord(m_Flags)) return 0;
				if (!out.WriteWord(m_QueriesCount)) return 0;
				if (!out.WriteWord((Word) m_AnswerRecordsCount)) return 0;
				if (!out.WriteWord((Word) m_AuthorityRecordsCount)) return 0;
				if (!out.WriteWord((Word) m_AdditionalRecordsCount)) return 0;
				RedBlackBST<char *, DWord> labelsOffsets(strCmp);
				// Queries
				if (!EncodeQueries(&out, &labelsOffsets)) return 0;
				// Records
				if (!EncodeRecords(&out, m_AnswerRecords, m_AnswerRecordsCount, &labelsOffsets)) return 0;
				if (!EncodeRecords(&out, m_AuthorityRecords, m_AuthorityRecordsCount, &labelsOffsets)) return 0;
				if (!EncodeRecords(&out, m_AdditionalRecords, m_AdditionalRecordsCount, &labelsOffsets)) return 0;
				return out.GetSize();
			}
			
			void SanitizeRecords(bool (* filterCallback)(CResourceRecord *)) {
				m_AnswerRecordsCount = SanitizeRecords(m_AnswerRecords, m_AnswerRecordsCount, filterCallback);
				m_AuthorityRecordsCount = SanitizeRecords(m_AuthorityRecords, m_AuthorityRecordsCount, filterCallback);
				m_AdditionalRecordsCount = SanitizeRecords(m_AdditionalRecords, m_AdditionalRecordsCount, filterCallback);
			}

		private:

			DWord SanitizeRecords(CResourceRecord **records, DWord count, bool (* filterCallback)(CResourceRecord *)) {
				DWord newCount = count;
				for (DWord i = 0; i < newCount; i++) {
					CResourceRecord *pRec = records[i];
					if (!filterCallback(records[i])) {	
						newCount--;
						if (newCount) {
							records[i] = records[newCount];
							records[newCount] = 0;
							i--;
						}
						DELETE_OBJ(pRec);
					}
				}
				return newCount;
			}

			bool EncodeQueries(CBufferOutputStream *out, RedBlackBST<char *, DWord> *labelsOffsets) {
				for (DWord i = 0; i < m_QueriesCount; i++) {
					if (!m_Queries[i]->Encode(out, labelsOffsets)) return false;
				}
				return true;
			}

			bool EncodeRecords(CBufferOutputStream *out, CResourceRecord **rrs, DWord count, RedBlackBST<char *, DWord> *labelsOffsets) {
				for (DWord i = 0; i < count; i++) {
					if (!rrs[i]->Encode(out, labelsOffsets)) return false;
				}
				return true;
			}

			void FreeQueries() {
				if (m_Queries) {
					for (DWord i = 0; i < m_QueriesCount; i++) {
						if (m_Queries[i]) {
							DELETE_OBJ(m_Queries[i]);
						}
					}
					DELETE_ARR(m_Queries);
					m_QueriesCount = 0;
				}
			}
			
			void FreeRecords(CResourceRecord ***prrs, DWord *pSize) {
				if (*prrs) {
					CResourceRecord **rrs = *prrs;
					DWord size = *pSize;
					for (DWord i = 0; i < size; i++) {
						if (rrs[i]) {
							DELETE_OBJ(rrs[i]);
						}
					}
					DELETE_ARR(rrs);
				}
				*pSize = 0;
				*prrs = 0;
			}

			void SetRecordsCount(CResourceRecord ***prrs, DWord *pCount, DWord recordsCount) {
				FreeRecords(prrs, pCount);
				if (recordsCount) {
					NEW_ARR(*prrs, CResourceRecord *, recordsCount);
					mem_zero((void *) *prrs, sizeof(void *) * recordsCount);
					*pCount = recordsCount;
				}
			}

			void FreeMemory() {
				FreeQueries();
				FreeRecords(&m_AnswerRecords, &m_AnswerRecordsCount);
				FreeRecords(&m_AuthorityRecords, &m_AuthorityRecordsCount);
				FreeRecords(&m_AdditionalRecords, &m_AdditionalRecordsCount);
			}
			
		public:
			
			Word m_Id;
			union {
				Word m_Flags;
				struct {
					// 8 bits
					Word responseCode : 4;
					Word zero : 3;
					Word recursionAvailable : 1;
					// 8 bits
					Word recursionDesired : 1;
					Word truncationFlag : 1;
					Word authoritativeAnswerFlag : 1;
					Word opcode : 4;
					Word isAnswer : 1;
				} m_Flag;
			};

			DWord m_QueriesCount;
			CQuery **m_Queries;

			DWord m_AnswerRecordsCount;
			CResourceRecord **m_AnswerRecords;

			DWord m_AuthorityRecordsCount;
			CResourceRecord **m_AuthorityRecords;

			DWord m_AdditionalRecordsCount;
			CResourceRecord **m_AdditionalRecords;

	};

}

#endif

