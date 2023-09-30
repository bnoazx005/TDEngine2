#include "../../include/scene/components/CLODStrategyComponent.h"
#include <algorithm>


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateLODStrategyComponentFactory)


	CLODStrategyComponent::CLODStrategyComponent() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CLODStrategyComponent::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mIsDirty = true;

		return RC_OK;
	}

	E_RESULT_CODE CLODStrategyComponent::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CLODStrategyComponent::GetTypeId()));
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CLODStrategyComponent::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CLODStrategyComponent*>(pDestObject))
		{
			pComponent->mLODInstances.clear();
			std::copy(mLODInstances.begin(), mLODInstances.end(), std::back_inserter(pComponent->mLODInstances));

			pComponent->mIsDirty = true;

			return RC_OK;
		}

		return RC_FAIL;
	}

	E_RESULT_CODE CLODStrategyComponent::AddLODInstance(const TLODInstanceInfo& info)
	{
		mLODInstances.emplace_back(info);
		mIsDirty = true;

		return RC_OK;
	}

	E_RESULT_CODE CLODStrategyComponent::RemoveLODInstance(U32 index)
	{
		if (static_cast<USIZE>(index) >= mLODInstances.size())
		{
			return RC_INVALID_ARGS;
		}

		mLODInstances.erase(mLODInstances.cbegin() + static_cast<USIZE>(index));
		mIsDirty = true;

		return RC_OK;
	}

	void CLODStrategyComponent::Sort()
	{
		std::sort(mLODInstances.begin(), mLODInstances.end(), [](auto&& left, auto&& right)
		{
			return left.mSwitchDistance < right.mSwitchDistance;
		});

		mIsDirty = true;
	}

	void CLODStrategyComponent::ForEachInstance(const std::function<bool(USIZE, TLODInstanceInfo&)>& action)
	{
		if (!action)
		{
			return;
		}

		for (USIZE i = 0; i < mLODInstances.size(); ++i)
		{
			if (!action(i, mLODInstances[i]))
			{
				return;
			}
		}
	}

	TLODInstanceInfo* CLODStrategyComponent::GetLODInfo(U32 index)
	{
		if (static_cast<USIZE>(index) >= mLODInstances.size())
		{
			return nullptr;
		}

		return &mLODInstances[index];
	}

	TLODInstanceInfo* CLODStrategyComponent::GetLODInfo(F32 distanceToCamera)
	{
		F32 prevSwitchDistance = 0.0f;

		for (USIZE i = 0; i < mLODInstances.size(); ++i)
		{
			if (distanceToCamera > prevSwitchDistance && distanceToCamera < mLODInstances[i].mSwitchDistance)
			{
				return &mLODInstances[i];
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