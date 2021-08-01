#include "./../../include/core/CBaseObject.h"


namespace TDEngine2
{
	CBaseObject::CBaseObject() :
		mIsInitialized(false), mRefCounter(1)
	{
	}

	void CBaseObject::AddRef()
	{
		++mRefCounter;
	}

	U32 CBaseObject::GetRefCount() const
	{
		return mRefCounter;
	}
}