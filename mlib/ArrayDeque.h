#ifndef _CLASS_ARRAYQUEUE_H
#define _CLASS_ARRAYQUEUE_H

#define MIN_ARRAY_SIZE 4

template<typename T>
class ArrayDeque {
	public:
		ArrayDeque() {
			m_ArraySize = MIN_ARRAY_SIZE;
			m_pArray = new T[m_ArraySize];
			m_First = 0;
			m_Size = 0;
		}

		~ArrayDeque(){
			delete [] m_pArray;
		}

		bool IsEmpty() {
			return !m_Size;
		}

		unsigned int Size() {
			return m_Size;
		}

		// List

		void Add(T item) {
			Enqueue(item);
		}

		T At(unsigned int i) {
			if (i > m_Size) return 0;
			unsigned int p = i + m_First;
			if (p >= m_ArraySize) {
				p -= m_ArraySize;
			}
			return m_pArray[i];
		}

		// Queue

		void Clear() {
			m_First = 0;
			m_Size = 0;
			Resize(MIN_ARRAY_SIZE);
		}

		void Enqueue(T item) {
			unsigned int p = GetTail();
			m_pArray[p] = item;
			m_Size++;
			if (m_Size == m_ArraySize) {
				Resize(m_ArraySize << 1);
			}
		}

		T Dequeue() {
			if (m_Size) {
				m_Size--;
				T item = m_pArray[m_First];
				m_First++;
				if (m_First >= m_ArraySize) {
					m_First = 0;
				}
				if (m_Size < (m_ArraySize >> 2)) {
					Resize(m_ArraySize >> 1);
				}
				return item;
			}
			return 0;
		}

		// Stack

		void Push(T item) {
			Enqueue(item);
		}

		T Pop() {
			if (m_Size) {
				m_Size--;
				unsigned int p = GetTail();
				if (m_Size < (m_ArraySize >> 2)) {
					Resize(m_ArraySize >> 1);
				}
				return m_pArray[p];
			}
			return 0;
		}

	private:

		/** 
		 *	returns the index of the last item + 1
		 */
		inline unsigned int GetTail() {
			unsigned int p = m_First + m_Size;
			if (p >= m_ArraySize) {
				p -= m_ArraySize;
			}
			return p;
		}

		void Resize(unsigned int size) {
			if (size <= MIN_ARRAY_SIZE || size == m_ArraySize) {
				return;
			}
			T *buf = new T[size];
			unsigned int j = m_First;
			for (unsigned int i = 0; i < m_Size; i++) {
				buf[i] = m_pArray[j];
				j++;
				if (j >= m_ArraySize) {
					j = 0;
				}
			}
			m_First = 0;
			delete [] m_pArray;
			m_pArray = buf;
			m_ArraySize = size;
		}

		T *m_pArray;
		unsigned int m_ArraySize;
		unsigned int m_First;
		unsigned int m_Size;

};

#endif
