#include "ras/MeteredFunctions.hpp"

static const char *meteredFunctionName[] =
{
#define METERED_FUNCTION(Enum) #Enum
#include "ras/MeteredFunctions.ins"
#undef METERED_FUNCTION
};

