#ifndef _CLASS_REDBALCKBINARYST_H
#define _CLASS_REDBALCKBINARYST_H

#define IS_RBNODE_RED(x) (x != 0 && x->m_Color == NODE_COLOR_RED)
#include "ArrayDeque.h"

const bool NODE_COLOR_RED   = true;
const bool NODE_COLOR_BLACK = false;

template<typename K, typename V>
struct RBNode {
	
	K m_Key;           			// key
	V m_Val;         				// associated data
	RBNode<K, V> *m_pLeft, *m_pRight; // links to left and right subtrees
	bool m_Color;     			// color of parent link
	int m_N;             			// subtree count

	RBNode(K key, V val, bool color, int N) {
		this->m_Key = key;
		this->m_Val = val;
		this->m_Color = color;
		this->m_N = N;
		m_pRight = 0;
		m_pLeft = 0;
	}

};

template<typename Key, typename Val>
class RedBlackBST {

	private:

		RBNode<Key, Val> *m_pRoot;
		void (* m_Cleaner)(Key, Val);

		int (* compare)(Key, Key);
				
		unsigned int Size(RBNode<Key, Val> *x) {
			return x == 0? 0 : x->m_N;
		}
		
		Val Get(RBNode<Key, Val> *n, Key key) {
			//RBNode<Key, Val> *x = n;
			//while (x) {
			//	int cmp = compare(key, x->m_Key);
			//	if (cmp < 0) x = x->m_pLeft;
			//	else if (cmp > 0) x = x->m_pRight;
			//	else return x->m_Val;
			//}
			// return 0;
			RBNode<Key, Val> *x = GetNode(n, key);
			return x ? x->m_Val : 0;
		}
		
		RBNode<Key, Val> *GetNode(RBNode<Key, Val> *n, Key key) {
			RBNode<Key, Val> *x = n;
			while (x) {
				int cmp = compare(key, x->m_Key);
				if (cmp < 0) x = x->m_pLeft;
				else if (cmp > 0) x = x->m_pRight;
				else return x;
			}
			return 0;
		}

		bool Contains(RBNode<Key, Val> *n, Key key) {
			RBNode<Key, Val> *x = n;
			while (x) {
				int cmp = compare(key, x->m_Key);
				if (cmp < 0) x = x->m_pLeft;
				else if (cmp > 0) x = x->m_pRight;
				else return true;
			}
			return false;
		}

		void DeleteRBNode(RBNode<Key, Val> **n) {
			if (!*n) {
				return;
			}
			DeleteRBNode(&((*n)->m_pRight));
			DeleteRBNode(&((*n)->m_pLeft));
			delete *n;
			*n = 0;
		}

		RBNode<Key, Val> *PutInNode(RBNode<Key, Val> *h, Key key, Val val) {
			if (!h) {
				RBNode<Key, Val> *n = new RBNode<Key, Val>(key, val, NODE_COLOR_RED, 1);
				return n;
			}
			int cmp = compare(key, h->m_Key);
			if (cmp > 0) h->m_pRight = PutInNode(h->m_pRight, key, val);
			else if (cmp < 0) h->m_pLeft = PutInNode(h->m_pLeft, key, val);
			else h->m_Val = val;
			
			if (IS_RBNODE_RED(h->m_pRight) && !IS_RBNODE_RED(h->m_pLeft))
				h = RotateLeft(h);
			if (IS_RBNODE_RED(h->m_pLeft) && IS_RBNODE_RED(h->m_pLeft->m_pLeft))
				h = RotateRight(h);
			if (IS_RBNODE_RED(h->m_pLeft) && IS_RBNODE_RED(h->m_pRight))
				FlipColors(h);
				
			h->m_N = Size(h->m_pLeft) + Size(h->m_pRight) + 1;

			return h;
		}

		RBNode<Key, Val> *RotateRight(RBNode<Key, Val> *h) {
			// assert (h != 0) && IS_RBNODE_RED(h->m_pLeft);
			RBNode<Key, Val> *x = h->m_pLeft;
			h->m_pLeft = x->m_pRight;
			x->m_pRight = h;
			x->m_Color = x->m_pRight->m_Color;
			x->m_pRight->m_Color = NODE_COLOR_RED;
			x->m_N = h->m_N;
			h->m_N = Size(h->m_pLeft) + Size(h->m_pRight) + 1;
			return x;
		}

		RBNode<Key, Val> *RotateLeft(RBNode<Key, Val> *h) {
			RBNode<Key, Val> *x = h->m_pRight;
			h->m_pRight = x->m_pLeft;
			x->m_pLeft = h;
			x->m_Color = x->m_pLeft->m_Color;
			x->m_pLeft->m_Color =NODE_COLOR_RED;
			x->m_N = h->m_N;
			h->m_N = Size(h->m_pLeft) + Size(h->m_pRight) + 1;
			return x;
		}

		void FlipColors(RBNode<Key, Val> *h) {
			h->m_Color = !h->m_Color;
			h->m_pLeft->m_Color = !h->m_pLeft->m_Color;
			h->m_pRight->m_Color = !h->m_pRight->m_Color;
		}
		
		RBNode<Key, Val> *DeleteKeyFromNode(RBNode<Key, Val> *h, Key key) {
			if (compare(key, h->m_Key) < 0) {
				if (!IS_RBNODE_RED(h->m_pLeft) && !IS_RBNODE_RED(h->m_pLeft->m_pLeft))
					h = MoveRedLeft(h);
				h->m_pLeft = DeleteKeyFromNode(h->m_pLeft, key);
			} else {
				if (IS_RBNODE_RED(h->m_pLeft))
					h = RotateRight(h);
				if (compare(key, h->m_Key) == 0 && (h->m_pRight == 0)) {
					delete h;
					return 0;
				}
				if (!IS_RBNODE_RED(h->m_pRight) && !IS_RBNODE_RED(h->m_pRight->m_pLeft))
					h = MoveRedRight(h);
				if (compare(key, h->m_Key) == 0) {
					RBNode<Key, Val> *x = Min(h->m_pRight);
					h->m_Key = x->m_Key;
					h->m_Val = x->m_Val;
					h->m_pRight = DeleteMin(h->m_pRight);
				} else h->m_pRight = DeleteKeyFromNode(h->m_pRight, key);
			}
			return Balance(h);
		}
		
		RBNode<Key, Val> *MoveRedRight(RBNode<Key, Val> *h) {
			FlipColors(h);
			if (IS_RBNODE_RED(h->m_pLeft->m_pLeft)) {
				h = RotateRight(h);
				FlipColors(h);
			}
			return h;
		}

		RBNode<Key, Val> *MoveRedLeft(RBNode<Key, Val> *h) {
			FlipColors(h);
			if (IS_RBNODE_RED(h->m_pRight->m_pLeft)) {
				h->m_pRight = RotateRight(h->m_pRight);
				h = RotateLeft(h);
				FlipColors(h);
			}
			return h;
		}

		RBNode<Key, Val> *Balance(RBNode<Key, Val> *h) {
			if (IS_RBNODE_RED(h->m_pRight)) h = RotateLeft(h);
			if (IS_RBNODE_RED(h->m_pLeft) && IS_RBNODE_RED(h->m_pLeft->m_pLeft)) h = RotateRight(h);
			if (IS_RBNODE_RED(h->m_pLeft) && IS_RBNODE_RED(h->m_pRight)) FlipColors(h);
			
			h->m_N = Size(h->m_pLeft) + Size(h->m_pRight) + 1;
			return h;
		}
		
		RBNode<Key, Val> *DeleteMin(RBNode<Key, Val> *h) {
			if (h->m_pLeft == 0) {
				delete h;
				return 0;
			}

			if (!IS_RBNODE_RED(h->m_pLeft) && !IS_RBNODE_RED(h->m_pLeft->m_pLeft))
				h = MoveRedLeft(h);

			h->m_pLeft = DeleteMin(h->m_pLeft);
			return Balance(h);
		}

		RBNode<Key, Val> *Min(RBNode<Key, Val> *x) {
			RBNode<Key, Val> *n = x;
			while (n->m_pLeft != 0) {
				n = n->m_pLeft;
			}
			return n;
			// why recursive? you waste stack & time calling the method...
			// if (x->m_pLeft == 0) return x;
			// else return Min(x->m_pLeft);
		}
		
		void Consume(void (* NodeConsumer)(Key, Val), RBNode<Key, Val> *node) {
			if (node) {
				Consume(NodeConsumer, node->m_pLeft);
				NodeConsumer(node->m_Key, node->m_Val);
				Consume(NodeConsumer, node->m_pRight);
			}
		}

	public:
	
		RedBlackBST(int (* compare)(Key, Key)) {
			m_Cleaner = 0;
			m_pRoot = 0;
			this->compare = compare;
		}
		
		~RedBlackBST() {
			if (m_Cleaner) {
				ForEach(m_Cleaner);
			}
			Clear();
		}

		RBNode<Key, Val> *GetRoot() {
			return m_pRoot;
		}

		unsigned int Size() {
			return Size(m_pRoot);
		}

		bool IsEmpty() {
			return m_pRoot == 0;
		}
		
		Val Get(Key key) {
			return Get(m_pRoot, key);
		}
		
		RBNode<Key, Val> *GetNode(Key key) {
			return GetNode(m_pRoot, key);
		}
		
		bool Contains(Key key) {
			return Contains(m_pRoot, key);
		}
		
		void Put(Key key, Val val) {
			m_pRoot = PutInNode(m_pRoot, key, val);
			m_pRoot->m_Color = NODE_COLOR_BLACK;
		}

		void Clear(){
			if(m_pRoot)
				DeleteRBNode(&m_pRoot);
		}

		void ForEach(void (* NodeConsumer)(Key, Val)) {
			Consume(NodeConsumer, m_pRoot);
		}

		void SetCleaner(void (* NodeConsumer)(Key, Val)) {
			m_Cleaner = NodeConsumer;
		}

		void DeleteKey(Key key) {
			if (!Contains(key)) {
				return;
			}

			// if both children of root are black, set root to red
			if (!IS_RBNODE_RED(m_pRoot->m_pLeft) && !IS_RBNODE_RED(m_pRoot->m_pRight))
				m_pRoot->m_Color = NODE_COLOR_RED;

			m_pRoot = DeleteKeyFromNode(m_pRoot, key);
			if (!IsEmpty()) m_pRoot->m_Color = NODE_COLOR_BLACK;
		}
};

#endif
