#include "compile/Compilation.hpp"
#include "ras/MeteredFunctions.hpp"
#include <time.h>

static const char *meteredFunctionName[] =
{
#define METERED_FUNCTION(Enum) #Enum
#include "ras/MeteredFunctions.ins"
#undef METERED_FUNCTION
};

TR::FunctionMeter::FunctionMeter(MeteredFunction f, TR::Compilation *comp) : _mf(f), _comp(comp) {
   comp->totalCalls[f]++;
   clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &_startTime);
}

TR::FunctionMeter::~FunctionMeter() {
   struct timespec end_time;
   clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
   int64_t diffInNanos = (end_time.tv_sec - _start_time.tv_sec) * (int64_t)1e9 + (end_time.tv_nsec - _start_time.tv_nsec);

   _comp->elapsedTimeInNanos[_mf] = diffInNanos;
}
