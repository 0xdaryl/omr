#ifndef TR_METERED_FUNCTIONS
#define TR_METERED_FUNCTIONS

typedef enum MeteredFunction {
#define METERED_FUNCTION(Enum) Enum
#include "ras/MeteredFunctions.ins"
   numMeteredFunctions
#undef METERED_FUNCTION
} MeteredFunction;

namespace TR { class Compilation; }


namespace TR {

class FunctionMeter {

private:
   struct timespec _startTime;

   MeteredFunction _mf;
   TR::Compilation *_comp;

public:
   FunctionMeter(MeteredFunction f, TR::Compilation *comp);

   ~FunctionMeter();
};

}


#define TIMER_FUNC(x) { TR::FunctionMeter startMF(f, c); }

#endif
