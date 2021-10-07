#ifndef _CLASS_DNS_CACHE_
#define _CLASS_DNS_CACHE_

#define MAX_CACHE_TTL 300

namespace dns {

	class CCacheNode {

		public:

			CCacheNode() {
				m_ppRecords = 0;
			}
			
			~CCacheNode() {
				if (m_ppRecords) {
					DELETE_ARR(m_ppRecords);
				}
			}
		
			DWord m_Size1;
			DWord m_Size2;
			DWord m_Size3;
			CResourceRecord **m_ppRecords;
	};

	static void deleteCacheNode(char *name, CCacheNode *node) {
		if (name) {
			DELETE_ARR(name);
		}
		DWord size = node->m_Size1 + node->m_Size2 + node->m_Size3;
		for (DWord i = 0; i < size; i++) {
			DELETE_OBJ(node->m_ppRecords[i]);
		}
		DELETE_OBJ(node);
	}
	
	class CCache {

		public:
			CCache() {
				m_pRecordTree = 0;
			}

			~CCache() {
				Clear();
			}

			// assumes all records are IPV4
			void Save(char *name, CResourceRecord **rrs, DWord size1, DWord size2, DWord size3) {
				DWord size = size1 + size2 + size3;
				if (!size || !str_length(name)) {
					return;
				}
				// find the min ttl
				int min = MAX_CACHE_TTL;
				for (DWord i = 0; i < size; i++) {
					int t = rrs[i]->GetTimeToLive();
					if (t < min)
						min = t;
				}
				if (min <= 0) {
					return; // don't save shit.
				}
				CCacheNode *NEW_OBJ(node, CCacheNode, ());
				node->m_Size1 = size1;
				node->m_Size2 = size2;
				node->m_Size3 = size3;
				NEW_ARR(node->m_ppRecords, CResourceRecord *, size);
				for (DWord i = 0; i < size; i++) {
					node->m_ppRecords[i] = rrs[i]->Copy();
					node->m_ppRecords[i]->SetTimeToLive(min);
				}
				// add shit to the tree of crap
				if (!m_pRecordTree) {
					NEW_OBJ2(m_pRecordTree, RedBlackBST, char *, CCacheNode *, (strCmp));
				}
				// if the record already exist, replace shit only
				RBNode<char *, CCacheNode *> *old = m_pRecordTree->GetNode(name);
				if (old) {
					deleteCacheNode(0, old->m_Val);
					old->m_Val = node;
				} else {
					// copy the name
					DWord nameLen = str_length(name) + 1; // +1 to include the 0
					char *NEW_ARR(nameCopy, char, nameLen);
					mem_copy(nameCopy, name, nameLen);
					// add to the tree
					m_pRecordTree->Put(nameCopy, node);
				}
			}

			void RemoveExpired() {
				if (!m_pRecordTree) {
					return;
				}
				ArrayDeque<char *> list;
				DeleteExpiredValues(m_pRecordTree->GetRoot(), &list);
				DWord size = list.Size();
				for (DWord i = 0; i < size; i++) {
					char *name = list.At(i);
					m_pRecordTree->DeleteKey(name);
					DELETE_ARR(name);
				}
				if (!m_pRecordTree->Size()) {
					DELETE_OBJ(m_pRecordTree);
				}
			}

			DWord Size() {
				return m_pRecordTree ? m_pRecordTree->Size() : 0;
			}

			// Deletes the shit out of everything
			void Clear() {
				if (m_pRecordTree) {
					m_pRecordTree->ForEach(deleteCacheNode);
					DELETE_OBJ(m_pRecordTree);
				}
			}

			CCacheNode *Get(char *name) {
				return m_pRecordTree ? m_pRecordTree->Get(name) : 0;
			}

		private:
		
			void DeleteExpiredValues(RBNode<char *, CCacheNode *> *node, ArrayDeque<char *> *list) {
				if (!node) 
					return;
				DeleteExpiredValues(node->m_pRight, list);
				if (node->m_Val->m_ppRecords[0]->isExpired()) {
					list->Add(node->m_Key);
					deleteCacheNode(0, node->m_Val);
				}
				DeleteExpiredValues(node->m_pLeft, list);
			}

			RedBlackBST<char *, CCacheNode *> *m_pRecordTree;
	};

}

#endif
