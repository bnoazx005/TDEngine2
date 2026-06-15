#include "../../include/scene/components/CLODStrategyComponent.h"
#include <algorithm>
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateLODStrategyComponentFactory)
	TDE2_DEFINE_COMPONENT_META(TLODStrategyComponentData)


	bool operator== (const TLODInstanceInfo& left, const TLODInstanceInfo& right)
	{
		return left.mActiveParams == right.mActiveParams && left.mMeshId == right.mMeshId && left.mSubMeshId == right.mSubMeshId && left.mMaterialId == right.mMaterialId;
	}


	CLODStrategyComponent::CLODStrategyComponent() :
		CBaseComponentT()
	{
	}

	E_RESULT_CODE CLODStrategyComponent::AddLODInstance(const TLODInstanceInfo& info)
	{
		mData.mLODInstances.emplace_back(info);
		mData.mIsDirty = true;

		return RC_OK;
	}

	E_RESULT_CODE CLODStrategyComponent::RemoveLODInstance(U32 index)
	{
		if (static_cast<USIZE>(index) >= mData.mLODInstances.size())
		{
			return RC_INVALID_ARGS;
		}

		mData.mLODInstances.erase(mData.mLODInstances.cbegin() + static_cast<USIZE>(index));
		mData.mIsDirty = true;

		return RC_OK;
	}

	void CLODStrategyComponent::Sort()
	{
		std::sort(mData.mLODInstances.begin(), mData.mLODInstances.end(), [](auto&& left, auto&& right)
		{
			return left.mSwitchDistance < right.mSwitchDistance;
		});

		mData.mIsDirty = true;
	}

	void CLODStrategyComponent::ForEachInstance(const std::function<bool(USIZE, TLODInstanceInfo&)>& action)
	{
		if (!action)
		{
			return;
		}

		for (USIZE i = 0; i < mData.mLODInstances.size(); ++i)
		{
			if (!action(i, mData.mLODInstances[i]))
			{
				return;
			}
		}
	}

	TLODInstanceInfo* CLODStrategyComponent::GetLODInfo(U32 index)
	{
		if (static_cast<USIZE>(index) >= mData.mLODInstances.size())
		{
			return nullptr;
		}

		return &mData.mLODInstances[index];
	}

	TLODInstanceInfo* CLODStrategyComponent::GetLODInfo(F32 distanceToCamera)
	{
		F32 prevSwitchDistance = 0.0f;

		for (USIZE i = 0; i < mData.mLODInstances.size(); ++i)
		{
			if (distanceToCamera > prevSwitchDistance && distanceToCamera < mData.mLODInstances[i].mSwitchDistance)
			{
				return &mData.mLODInstances[i];
			}
		}

		return nullptr;
	}
		

	IComponent* CreateLODStrategyComponent(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CLODStrategyComponent, result);
	}


	/*!
		\brief CLODStrategyComponentFactory's definition
	*/

	CLODStrategyComponentFactory::CLODStrategyComponentFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CLODStrategyComponentFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateLODStrategyComponent(result);
	}

	E_RESULT_CODE CLODStrategyComponentFactory::SetupComponent(CLODStrategyComponent* pComponent, const TLODStrategyComponentParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateLODStrategyComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CLODStrategyComponentFactory, result);
	}
}