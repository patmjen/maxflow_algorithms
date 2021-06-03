#ifndef msvc_debug_h
#define msvc_debug_h

namespace debug{
	class CRTREPORTMODE{
	public:
		CRTREPORTMODE();
		~CRTREPORTMODE();
	};
	extern CRTREPORTMODE CrtReportMode;
};

#endif

