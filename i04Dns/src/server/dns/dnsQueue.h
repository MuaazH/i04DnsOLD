#ifndef _CLASS_DNS_QUEUE_
#define _CLASS_DNS_QUEUE_

#define INVALID_QUEUE_ID 0x0001
#define MIN_ID 0x00FF
#define MAX_ID 0x7FFF

#define MAX_QUEUE_SIZE 0x800

namespace dns {

	class PendingQuery {
		public:
			PendingQuery() {
				m_pName = 0;
				m_pServer = 0;
			}
			
			~PendingQuery() {
				if (m_pName) {
					DELETE_ARR(m_pName);
				}
				if (m_pServer) {
					DELETE_OBJ(m_pServer);
				}
			}

			long long           m_CreationTime;
			int                 m_SrcId;
			IPV4                m_SrcIp;
			int                 m_SrcPort;
			CSecondaryServer *m_pServer;
			char                *m_pName;
	};

	class CDnsQueue {

		public:
			CDnsQueue() {
				NEW_OBJ2(m_pQueue, RedBlackBST, DWord, PendingQuery *, (CDnsQueue::IntCmp));
				m_NextID = rand(MIN_ID, MIN_ID + 10);
			}

			~CDnsQueue() {
				Clear();
				DELETE_OBJ(m_pQueue);
			}

			DWord Enqueue(DWord id, IPV4 src, int port, CSecondaryServer *pSrv, CQuery *pQuery) {
				if (m_pQueue->Size() >= MAX_QUEUE_SIZE) {
					return INVALID_QUEUE_ID;
				}
				DWord localID = GetNextID();
				PendingQuery *NEW_OBJ(pPendingQuery, PendingQuery, ());
				pPendingQuery->m_CreationTime = hr_time_get();
				pPendingQuery->m_SrcId = id;
				pPendingQuery->m_SrcIp = src;
				pPendingQuery->m_SrcPort = port;
				pPendingQuery->m_pServer = pSrv;
				DWord len = str_length(pQuery->GetName()) + 1; // terminating zero
				NEW_ARR(pPendingQuery->m_pName, char, len);
				mem_copy((void *)pPendingQuery->m_pName, (void *) pQuery->GetName(), len);
				m_pQueue->Put(localID, pPendingQuery);
				return localID;
			}

			PendingQuery *Remove(DWord id, DWord *pWaitTime) {
				PendingQuery *pQuery = m_pQueue->Get(id);
				if (pQuery) {
					m_pQueue->DeleteKey(id);
					*pWaitTime = (DWord) ((hr_time_get() - pQuery->m_CreationTime) / (hr_time_freq() / 1000));
				}
				return pQuery;
			}

			void Release(PendingQuery *pQuery) {
				CDnsQueue::DeleteQuery(0, pQuery);
			}

			void ForEachExpired(int ageMs, void (* callback)(DWord, PendingQuery *)) {
				ArrayDeque<DWord> list;
				long long minTime = hr_time_get() - (ageMs * hr_time_freq() / 1000);
				ForEachExpired(m_pQueue->GetRoot(), minTime, callback, &list);
				while (list.Size()) {
					DWord id = list.Dequeue();
					DeleteQuery(0, m_pQueue->Get(id));
					m_pQueue->DeleteKey(id);
				}
			}

		private:

			void ForEachExpired(RBNode<DWord, PendingQuery *> *pNode,
								long long minTime,
								void (* callback)(DWord, PendingQuery *),
								ArrayDeque<DWord> *pList)
			{
				if (!pNode) return;
				ForEachExpired(pNode->m_pRight, minTime, callback, pList);
				if (pNode->m_Val->m_CreationTime < minTime) {
					callback(pNode->m_Key, pNode->m_Val);
					pList->Add(pNode->m_Key);
				}
				ForEachExpired(pNode->m_pLeft, minTime, callback, pList);
			}

			DWord GetNextID() {
				m_NextID += rand(7, 23);
				if (m_NextID > MAX_ID) {
					m_NextID += MIN_ID + rand(3, 7) - MAX_ID;
				}
				return m_NextID;
			}

			void Clear() {
				m_pQueue->ForEach(CDnsQueue::DeleteQuery);
				m_pQueue->Clear();
			}

			DWord m_NextID;
			RedBlackBST<DWord, PendingQuery *> *m_pQueue;

			// static
			static int IntCmp(DWord a, DWord b) {
				return a < b ? -1 : (a == b ? 0 : 1);
			}

			static void DeleteQuery(DWord id, PendingQuery *pQuery) {
				pQuery->m_pServer = 0;
				DELETE_OBJ(pQuery);
			}

	};

}

#endif
