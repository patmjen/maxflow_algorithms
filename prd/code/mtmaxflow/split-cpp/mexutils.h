//
// Written by Petter Strandmark 2009
//
// Contains a function for flushing the output buffer to
// MATLAB
//
#ifndef MEX_UTILS_PETTER
#define MEX_UTILS_PETTER

#include "mex.h"

#include <sstream>
#define ASSERT(cond) if (!(cond)) { std::stringstream sout; \
									sout << "Error (line " << __LINE__ << "): " << #cond; \
                                    mexErrMsgTxt(sout.str().c_str()); }

namespace {

	void flush_output()
	{
		//Hack
		mexEvalString("drawnow");
	}

}

#endif
