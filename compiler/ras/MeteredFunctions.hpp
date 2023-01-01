#define TIMER_FUNC(x) {}

typedef enum meteredFunctionEnum {
#define METERED_FUNCTION(Enum) Enum
#include "ras/MeteredFunctions.ins"
#undef METERED_FUNCTION
} meteredFunctionEnum;
