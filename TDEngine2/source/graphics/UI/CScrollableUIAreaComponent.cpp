#include "../../../include/graphics/UI/CScrollableUIAreaComponent.h"


namespace TDEngine2
{
	CScrollableUIArea::CScrollableUIArea() :
		CBaseComponent()
	{
	}


	struct TScrollableUIAreaArchiveKeys
	{
		static const std::string mContentEntityRefKeyId;
		static const std::string mNormalizedScrollPositionKeyId;
	};


	const std::string TScrollableUIAreaArchiveKeys::mContentEntityRefKeyId = "content_entity_ref";
	const std::string TScrollableUIAreaArchiveKeys::mNormalizedScrollPositionKeyId = "normalized_scroll_pos";

	E_RESULT_CODE CScrollableUIArea::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mContentEntityRef = static_cast<TEntityId>(pReader->GetUInt32(TScrollableUIAreaArchiveKeys::mContentEntityRefKeyId));

		pReader->BeginGroup(TScrollableUIAreaArchiveKeys::mNormalizedScrollPositionKeyId);
		if (auto posResult = LoadVector2(pReader))
		{
			mNormalizedScrollPosition = Normalize(posResult.Get());
		}
		pReader->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CScrollableUIArea::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CScrollableUIArea::GetTypeId()));

			pWriter->SetUInt32(TScrollableUIAreaArchiveKeys::mContentEntityRefKeyId, static_cast<U32>(mContentEntityRef));	

			pWriter->BeginGroup(TScrollableUIAreaArchiveKeys::mNormalizedScrollPositionKeyId);
			SaveVector2(pWriter, mNormalizedScrollPosition);
			pWriter->EndGroup();
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CScrollableUIArea::PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper)
	{
		mContentEntityRef = entitiesIdentifiersRemapper.Resolve(mContentEntityRef);
		
		return CBaseComponent::PostLoad(pEntityManager, entitiesIdentifiersRemapper);
	}

	E_RESULT_CODE CScrollableUIArea::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CScrollableUIArea*>(pDestObject))
		{
			pComponent->mContentEntityRef = mContentEntityRef;
			pComponent->mNormalizedScrollPosition = mNormalizedScrollPosition;

			return RC_OK;
		}

		return RC_FAIL;
	}

	void CScrollableUIArea::SetContentEntityId(TEntityId cursorId)
	{
		mContentEntityRef = cursorId;
	}

	void CScrollableUIArea::SetNormalizedScrollPosition(const TVector2& pos)
	{
		mNormalizedScrollPosition = pos;
	}

	void CScrollableUIArea::SetLayoutPrepared(bool value)
	{
		mIsLayoutPrepared = value;
	}
	
	TEntityId CScrollableUIArea::GetContentEntityId() const
	{
		return mContentEntityRef;
	}

	const TVector2& CScrollableUIArea::GetNormalizedScrollPosition() const
	{
		return mNormalizedScrollPosition;
	}

	bool CScrollableUIArea::IsLayoutInitialized() const
	{
		return mIsLayoutPrepared;
	}


	IComponent* CreateScrollableUIArea(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CScrollableUIArea, result);
	}


	/*!
		\brief CScrollableUIAreaFactory's definition
	*/

	CScrollableUIAreaFactory::CScrollableUIAreaFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CScrollableUIAreaFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateScrollableUIArea(result);
	}

	E_RESULT_CODE CScrollableUIAreaFactory::SetupComponent(CScrollableUIArea* pComponent, const TScrollableUIAreaParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateScrollableUIAreaFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CScrollableUIAreaFactory, result);
	}
}