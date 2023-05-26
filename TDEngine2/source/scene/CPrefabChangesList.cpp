#include "../../include/scene/CPrefabChangesList.h"
#include "stringUtils.hpp"


namespace TDEngine2
{
	struct TPrefabChangesListArchiveKeys
	{
		static const std::string mChangesArrayGroupKey;
		static const std::string mChangesArrayElementGroupKey;
		static const std::string mTargetIdLey;
		static const std::string mPropertyBindingKey;
		static const std::string mValueKey;
	};

	const std::string TPrefabChangesListArchiveKeys::mChangesArrayGroupKey = "changes";
	const std::string TPrefabChangesListArchiveKeys::mChangesArrayElementGroupKey = "element";
	const std::string TPrefabChangesListArchiveKeys::mTargetIdLey = "target_id";
	const std::string TPrefabChangesListArchiveKeys::mPropertyBindingKey = "property_binding";
	const std::string TPrefabChangesListArchiveKeys::mValueKey = "value";


	CPrefabChangesList::CPrefabChangesList() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CPrefabChangesList::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}
		
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CPrefabChangesList::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		pWriter->BeginGroup(TPrefabChangesListArchiveKeys::mChangesArrayGroupKey);
		
		for (TChangeDesc& currDesc : mChanges)
		{
			pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
			{
				pWriter->BeginGroup(TPrefabChangesListArchiveKeys::mChangesArrayElementGroupKey);

				pWriter->SetUInt32(TPrefabChangesListArchiveKeys::mTargetIdLey, static_cast<U32>(currDesc.mTargetLinkEntityId));
				pWriter->SetString(TPrefabChangesListArchiveKeys::mPropertyBindingKey, currDesc.mPropertyBinding);

				pWriter->BeginGroup(TPrefabChangesListArchiveKeys::mValueKey);
				currDesc.mValue.Save(pWriter);
				pWriter->EndGroup();


				pWriter->EndGroup();
			}
			pWriter->EndGroup();
		}
		
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CPrefabChangesList::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		mChanges.clear();

		pReader->BeginGroup(TPrefabChangesListArchiveKeys::mChangesArrayGroupKey);

		TChangeDesc currDesc;

		while (pReader->HasNextItem())
		{
			pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
			{
				pReader->BeginGroup(TPrefabChangesListArchiveKeys::mChangesArrayElementGroupKey);

				currDesc.mTargetLinkEntityId = static_cast<TEntityId>(pReader->GetUInt32(TPrefabChangesListArchiveKeys::mTargetIdLey, static_cast<U32>(TEntityId::Invalid)));
				currDesc.mPropertyBinding = pReader->GetString(TPrefabChangesListArchiveKeys::mPropertyBindingKey);
				
				pReader->BeginGroup(TPrefabChangesListArchiveKeys::mValueKey);
				auto loadValueResult = DeserializeValue(pReader);
				pReader->EndGroup();

				if (loadValueResult.HasError())
				{
					result = result | loadValueResult.GetError();
				}
				else
				{
					currDesc.mValue = loadValueResult.Get();
				}

				pReader->EndGroup();

				mChanges.emplace_back(currDesc);
			}
			pReader->EndGroup();
		}

		pReader->EndGroup();

		return result;
	}

	E_RESULT_CODE CPrefabChangesList::ApplyChanges(TPtr<IWorld> pWorld)
	{
		if (!pWorld)
		{
			return RC_INVALID_ARGS;
		}

		TDE2_UNIMPLEMENTED();

		return RC_OK;
	}


	TDE2_API CPrefabChangesList* CreatePrefabChangesList(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CPrefabChangesList, CPrefabChangesList, result);
	}
}