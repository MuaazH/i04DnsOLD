#ifdef DEBUG_CONF

#include "heap_dbg.h"

#include <stdio.h>

HeapDbgObj heap[HEAP_DBG_MAX_OBJECTS];

unsigned int objCnt = 0;

HeapDbgObj *heap_dbg_get(unsigned int i) {
	return i < objCnt ? &(heap[i]) : 0;
}

void heap_dbg_alloc(const char *pFileName, unsigned int lineNumber, void *ptr, unsigned int size, int type) {
	if (objCnt >= HEAP_DBG_MAX_OBJECTS) {
		printf("heap_dbg: overflow\n");
		return;
	}
	heap[objCnt].pFileName = pFileName;
	heap[objCnt].lineNumber = lineNumber;
	heap[objCnt].ptr = ptr;
	heap[objCnt].size = size;
	heap[objCnt].type = type;
	objCnt++;
}

int get_id(void *ptr) {
	unsigned int i = 0;
	while (i < objCnt) {
		if (ptr == heap[i].ptr) return i;
		i++;
	}
	return -1;
}

void heap_dbg_free(const char *pFileName, unsigned int lineNumber, void *ptr, int type) {
	int i = get_id(ptr);
	if (i >= 0) {
		objCnt--;
		if (heap[i].type != type) {
			printf("heap_dbg: memory %p of type %d released as type %d. (%s:%d)\n", ptr, heap[i].type, type, pFileName, lineNumber);
		}
		heap[i].pFileName = heap[objCnt].pFileName;
		heap[i].lineNumber = heap[objCnt].lineNumber;
		heap[i].ptr = heap[objCnt].ptr;
		heap[i].size = heap[objCnt].size;
		heap[i].type = heap[objCnt].type;
	} else {
		printf("heap_dbg: can't free unknown memory %p (%s:%d)\n", ptr, pFileName, lineNumber);
	}
}

void heap_dbg_alloc_obj(const char *pFileName, unsigned int lineNumber, void *ptr, unsigned int size) {
	heap_dbg_alloc(pFileName, lineNumber, ptr, size, DATATYPE_OBJECT);
}

void heap_dbg_alloc_arr(const char *pFileName, unsigned int lineNumber, void *ptr, unsigned int size) {
	heap_dbg_alloc(pFileName, lineNumber, ptr, size, DATATYPE_ARRAY);
}

void heap_dbg_free_obj(const char *pFileName, unsigned int lineNumber, void *ptr) {
	heap_dbg_free(pFileName, lineNumber, ptr, DATATYPE_OBJECT);
}

void heap_dbg_free_arr(const char *pFileName, unsigned int lineNumber, void *ptr) {
	heap_dbg_free(pFileName, lineNumber, ptr, DATATYPE_ARRAY);
}

unsigned int calc_size() {
	unsigned int size = 0;
	for (unsigned int i = 0; i < objCnt; i++) {
		size += heap[i].size;
	}
	return size;
}

unsigned int heap_dbg_get_size() {
	return calc_size();
}

unsigned int heap_dbg_objects_count() {
	return objCnt;
}


#endif
