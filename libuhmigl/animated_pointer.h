#ifndef __animated_pointer_h__
#define __animated_pointer_h__

// ISO C prohibits direct conversion between object and function pointers
#define ANIMATED_POINTER(_type, _ptr) \
	(*((_type *)(&(_ptr))))

#endif
