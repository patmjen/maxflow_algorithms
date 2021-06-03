#include "msvcdebug.h"

#ifdef WIN32
#include <afxwin.h>

namespace debug{
	CRTREPORTMODE::CRTREPORTMODE(){
#if defined(_DEBUG)
		//#error Requires Multythreaded Debug Runtime Library
#endif
		_CrtSetReportMode(_CRT_ERROR,_CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_ERROR,_CRTDBG_FILE_STDOUT);
		int tmp;
		tmp = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		tmp = tmp | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF;
		_CrtSetDbgFlag(tmp);
	};

	CRTREPORTMODE::~CRTREPORTMODE(){
		int tmp = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		if(!(tmp & _CRTDBG_LEAK_CHECK_DF))_CrtDumpMemoryLeaks();
	};

	CRTREPORTMODE CrtReportMode;
};
#else
namespace debug{
    CRTREPORTMODE::CRTREPORTMODE(){
    };
    CRTREPORTMODE::~CRTREPORTMODE(){
    };
};
#endif
