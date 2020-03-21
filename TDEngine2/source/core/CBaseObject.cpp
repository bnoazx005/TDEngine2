#include "./../../include/core/CBaseObject.h"


namespace TDEngine2
{
	CBaseObject::CBaseObject() :
		mIsInitialized(false)
	{
	}

	void CBaseObject::AddRef()
	{
		++mRefCounter;
	}
}