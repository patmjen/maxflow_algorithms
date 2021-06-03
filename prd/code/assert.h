#ifndef assert_h
#define assert_h

#if _MSC_VER > 1000
	#undef assert
	#ifndef NDEBUG
		#include <crtdbg.h>
		#define assert(expression) {_ASSERT(expression);}		
	#else
		#define assert(expression) ((void)0)
	#endif
#else

	#define assert(expression) ((void)0)

#endif

#define check_pointer(p, beg, end) {assert(p>=beg && p<end);}

#endif
