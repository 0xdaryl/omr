#include "ras/MeteredFunctions.hpp"

static const char *meteredFunctionName[] =
{
#define METERED_FUNCTION(Enum, name) #name
#include "ras/MeteredFunctions.ins"
#undef METERED_FUNCTION
};

