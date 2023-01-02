#ifndef TR_METERED_FUNCTIONS
#define TR_METERED_FUNCTIONS

typedef enum meteredFunctionEnum {
#define METERED_FUNCTION(Enum) Enum
#include "ras/MeteredFunctions.ins"
   numMeteredFunctions
#undef METERED_FUNCTION
} meteredFunctionEnum;


namespace TR {

class FunctionMeter {

private:
   struct timespec _startTime;

   MeteredFunction _mf;
   TR::Compilation *_comp;

public:
   FunctionMeter(MeteredFunction f, TR::Compilation *comp) : _mf(f), _comp(comp) {
      comp->totalCalls[f]++;
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &_startTime);
   }

   ~FunctionMeter() {
      struct timespec end_time;
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
      int64_t diffInNanos = (end_time.tv_sec - _start_time.tv_sec) * (int64_t)1e9 + (end_time.tv_nsec - _start_time.tv_nsec);

      _comp->elapsedTimeInNanos[_mf] = diffInNanos;
   }

}

}



#define TIMER_FUNC(x) {}

#endif
