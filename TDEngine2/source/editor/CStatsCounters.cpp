#include "../../include/editor/CStatsCounters.h"

#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	U32 CStatsCounters::mDrawCallsCount = 0;
	U32 CStatsCounters::mLoadedResourcesCount = 0;
	U32 CStatsCounters::mTotalEntitiesCount = 0;
	U32 CStatsCounters::mTotalComponentsCount = 0;
}

#endif