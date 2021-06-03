#ifndef debug_logs_h
#define debug_logs_h

#include "streams/xstringstream.h"
//#include <stdio.h>
#include <iostream>


namespace debug{
	using namespace txt;

	//_________________stdout_________________________
	class stdoutStream : public TextStream{
	public:
		virtual TextStream & write(const char * x){
			std::cout<<x;
			return *this;
		};
		static stdoutStream the_stream;
	};
	extern pTextStream out1;
	extern pTextStream stream;
	extern pTextStream errstream;
	extern bool debug1;
};

#endif
