#ifndef _HEAP_DEBUG_H_
#define _HEAP_DEBUG_H_

#ifdef DEBUG_CONF

	#define HEAP_DBG_MAX_OBJECTS 0x2000

	enum {
		DATATYPE_OBJECT = 1,
		DATATYPE_ARRAY = 2
	};

	typedef struct { 
		const char *pFileName;
		unsigned int lineNumber;
		void *ptr;
		unsigned int size;
		int type;
	} HeapDbgObj;

	#define NEW_OBJ(ptr, type, param) ptr = new type param; heap_dbg_alloc_obj(__FILE__, __LINE__, (void *) (ptr), sizeof(type))
	#define NEW_OBJ2(ptr, type, typeP1, typeP2, param) ptr = new type<typeP1, typeP2> param; heap_dbg_alloc_obj(__FILE__, __LINE__, (void *) (ptr), sizeof(type<typeP1, typeP2>))

	#define NEW_ARR(ptr, type, size) ptr = new type[size]; heap_dbg_alloc_arr(__FILE__, __LINE__, (void *) (ptr), sizeof(type) * size)

	#define DELETE_OBJ(ptr) heap_dbg_free_obj(__FILE__, __LINE__, ptr); delete ptr; (ptr) = 0

	#define DELETE_ARR(ptr) heap_dbg_free_arr(__FILE__, __LINE__, ptr); delete [] ptr; (ptr) = 0

	void heap_dbg_alloc_obj(const char *pFileName, unsigned int lineNumber, void *ptr, unsigned int size);

	void heap_dbg_alloc_arr(const char *pFileName, unsigned int lineNumber, void *ptr, unsigned int size);

	void heap_dbg_free_obj(const char *pFileName, unsigned int lineNumber, void *ptr);

	void heap_dbg_free_arr(const char *pFileName, unsigned int lineNumber, void *ptr);

	unsigned int heap_dbg_get_size();

	unsigned int heap_dbg_objects_count();

	HeapDbgObj *heap_dbg_get(unsigned int i);

#else

	#define NEW_OBJ(ptr, type, param) ptr = new type param
	#define NEW_OBJ2(ptr, type, typeP1, typeP2, param) ptr = new type<typeP1, typeP2> param

	#define NEW_ARR(ptr, type, size) ptr = new type[size]

	#define DELETE_OBJ(ptr) delete ptr; (ptr) = 0

	#define DELETE_ARR(ptr) delete [] ptr; (ptr) = 0

#endif


#endif

