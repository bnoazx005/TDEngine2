#include "./../../include/ecs/CBaseComponent.h"
#include <limits>


namespace TDEngine2
{
	CBaseComponent::CBaseComponent() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseComponent::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	
	CComponentIterator CComponentIterator::mInvalidIterator = CComponentIterator();

	CComponentIterator::CComponentIterator():
		mCurrIndex((std::numeric_limits<U32>::max)())
	{
	}

	CComponentIterator::CComponentIterator(TComponentsArray& components, U32 index):
		mBegin(components.begin()), mEnd(components.end()), mCurrIndex(index)
	{
	}

	CComponentIterator::~CComponentIterator()
	{
	}

	CComponentIterator& CComponentIterator::Next()
	{
		++mCurrIndex;

		return *this;
	}

	bool CComponentIterator::HasNext() const
	{
		return (mBegin + mCurrIndex + 1) != mEnd;
	}

	void CComponentIterator::Reset()
	{
		mCurrIndex = 0;
	}

	IComponent* CComponentIterator::Get() const
	{
		if ((mBegin + mCurrIndex) == mEnd)
		{
			return nullptr;
		}

		return *(mBegin + mCurrIndex);
	}

	IComponent* CComponentIterator::operator* () const
	{
		return Get();
	}

	CComponentIterator& CComponentIterator::operator++ ()
	{
		++mCurrIndex;

		return *this;
	}

	bool CComponentIterator::operator== (const CComponentIterator& iter) const
	{
		if (mCurrIndex != iter.mCurrIndex ||
			mBegin != iter.mBegin ||
			mEnd != iter.mEnd)
		{
			return false;
		}

		return true;
	}

	bool CComponentIterator::operator!= (const CComponentIterator& iter) const
	{
		if (mCurrIndex != iter.mCurrIndex ||
			mBegin != iter.mBegin ||
			mEnd != iter.mEnd)
		{
			return true;
		}

		return false;
	}
}