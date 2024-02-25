/*!
	\file CStatsCounters.h
	\date 25.02.2024
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	/*!
		\brief The class is a scope for global statistics counters
	*/

	class CStatsCounters
	{
		public:
			// Renderer
			TDE2_API static U32 mDrawCallsCount;

			// Resources
			TDE2_API static U32 mLoadedResourcesCount;

			// ECS
			TDE2_API static U32 mTotalEntitiesCount;
			TDE2_API static U32 mTotalComponentsCount;
	};
}

#define TDE2_STATS_COUNTER_ADD(Counter, Value) do { CStatsCounters::Counter += Value; } while(false)
#define TDE2_STATS_COUNTER_SUBT(Counter, Value) do { CStatsCounters::Counter -= Value; } while(false)
#define TDE2_STATS_COUNTER_SET(Counter, Value) do { CStatsCounters::Counter = Value; } while(false)
#define TDE2_STATS_COUNTER_INCREMENT(Counter) do { ++CStatsCounters::Counter; } while(false)
#define TDE2_STATS_COUNTER_DECREMENT(Counter) do { --CStatsCounters::Counter; } while(false)
#define TDE2_STATS_COUNTER_RESET(Counter) do { CStatsCounters::Counter = 0; } while(false)

#else

#define TDE2_STATS_COUNTER_ADD(Counter, Value)
#define TDE2_STATS_COUNTER_SUBT(Counter, Value)
#define TDE2_STATS_COUNTER_SET(Counter, Value) 
#define TDE2_STATS_COUNTER_INCREMENT(Counter)
#define TDE2_STATS_COUNTER_DECREMENT(Counter)
#define TDE2_STATS_COUNTER_RESET(Counter)

#endif
