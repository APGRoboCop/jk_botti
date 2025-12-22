/*
 * Copyright (c) 2004-2006 Jussi Kivilinna
 */
#ifndef NEW_BASECLASS_H
#define NEW_BASECLASS_H

#include <malloc.h>

//new/delete operators with malloc/free to remove need for libstdc++

class class_new_baseclass {
public:
	// Construction
	class_new_baseclass() = default;

	// Operators
	void * operator new(size_t size) {
		if(size==0)
			return(calloc(1, 1));
		return(calloc(1, size));
	}

	void * operator new[](size_t size) {
		if(size==0)
			return(calloc(1, 1));
		return(calloc(1, size));
	}

	void operator delete(void *ptr) {
		if(ptr)
			free(ptr); 
	}

	void operator delete[](void *ptr) {
		if(ptr)
			free(ptr); 
	}
};

#endif /*NEW_BASECLASS_H*/
