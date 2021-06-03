
//
// Written by Petter Strandmark 2009
//
// Uses Windows API where avaliable for high precision, otherwise
// the standard C++ clock funtions.
//

#ifdef PRINT_TIMING
	#include <stdio.h>
	#include <stdarg.h>
#endif


#include "mex.h"

#ifdef TIMING
    #ifdef _WIN32
        #include <windows.h>
        LARGE_INTEGER timestamp;    
    #else
        #include <ctime>
        std::clock_t timestamp;
    #endif
#endif

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

		
		
void inline startTime()
{
    #ifdef TIMING
        #ifdef _WIN32
            QueryPerformanceCounter( &timestamp );
        #else
            timestamp = clock();
        #endif
    #endif
}

//Works like printf, but the resulting string must be shorter
//than 512 bytes, or else...
double inline endTime(const char* format="", ...)
{
    #ifdef TIMING
        #ifdef _WIN32
            LARGE_INTEGER time_taken;
            QueryPerformanceCounter( &time_taken );
            time_taken.QuadPart = time_taken.QuadPart - timestamp.QuadPart;
            LARGE_INTEGER frequency;
            QueryPerformanceFrequency( &frequency );
            double time = double(time_taken.QuadPart) / double(frequency.QuadPart);
        #else
            std::clock_t timeTaken = std::clock() - timestamp; 
            double time = double(timeTaken) / double(CLOCKS_PER_SEC);
        #endif
		#ifdef PRINT_TIMING
			char buffer[512]; 
			va_list args;
			va_start( args, format );
			vsnprintf(buffer, 512, format, args);
			va_end( args );
			mexPrintf("%s: %f \n", buffer,time);
		#endif
        startTime();    
        return time;
    #else
        return 0;
    #endif
}
