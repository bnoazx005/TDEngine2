#include "../../include/ecs/CBaseComponent.h"
#include "../../include/ecs/CComponentManager.h"
#include "../../include/utils/CFileLogger.h"
#include <limits>
#include <stringUtils.hpp>
#include <result.hpp>


namespace TDEngine2
{
	CBaseComponent::CBaseComponent() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseComponent::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseComponent::Load(IArchiveReader* pReader)
	{
		return RC_OK;
	}

	E_RESULT_CODE CBaseComponent::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CBaseComponent::PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper)
	{
		return RC_OK;
	}

	E_RESULT_CODE CBaseComponent::Clone(IComponent*& pDestObject) const
	{
		//TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}

	const std::string& CBaseComponent::GetTypeName() const
	{
		TDE2_UNIMPLEMENTED();
		return Wrench::StringUtils::GetEmptyStr();
	}

	IPropertyWrapperPtr CBaseComponent::GetProperty(const std::string& propertyName)
	{
		LOG_ERROR(Wrench::StringUtils::Format("[CBaseComponent] The property ({0}) wasn't found", propertyName));
		return IPropertyWrapperPtr(nullptr);
	}

	const std::vector<std::string>& CBaseComponent::GetAllProperties() const
	{
		static const std::vector<std::string> properties{};
		return properties;
	}

	bool CBaseComponent::IsRuntimeOnly() const
	{
		return false;
	}

	
	CComponentIterator CComponentIterator::mInvalidIterator = CComponentIterator();

	CComponentIterator::CComponentIterator():
		mCurrIndex((std::numeric_limits<U32>::max)())
	{
	}

	CComponentIterator::CComponentIterator(TComponentsArray& components, U32 index):
		mBegin(components.begin()), mEnd(components.end()), mCurrIndex(index), mIsValid(true)
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
		return mIsValid && (mBegin != mEnd) && (mBegin + mCurrIndex + 1) != mEnd;
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


	TDE2_DEFINE_FLAG_COMPONENT(DeactivatedComponent)
	TDE2_DEFINE_FLAG_COMPONENT(DeactivatedGroupComponent)



	/*!
		TComponentFactoryRegister's definition
	*/

	TComponentFactoryRegister::TComponentFactoryRegister(TComponentFactoryFunctor&& factory)
	{
		CComponentManager::RegisterComponentType(std::forward<TComponentFactoryFunctor>(factory));
	}
}