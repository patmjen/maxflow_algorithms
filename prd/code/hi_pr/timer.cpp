#include "timer.h"

#ifndef WIN32
#include <time.h>
//#include <resource.h>

float timer ()
{
  clock_t tt = clock();
  return (float)(double(tt)/CLOCKS_PER_SEC);
}

#else

float timer (){
  return 0;
};

#endif
