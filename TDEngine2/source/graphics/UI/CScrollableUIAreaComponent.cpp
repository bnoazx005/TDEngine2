#include "../../../include/graphics/UI/CScrollableUIAreaComponent.h"
#include "../../../include/math/MathUtils.h"


namespace TDEngine2
{
	CScrollableUIArea::CScrollableUIArea() :
		CBaseComponent()
	{
	}


	struct TScrollableUIAreaArchiveKeys
	{
		static const std::string mContentEntityRefKeyId;
		static const std::string mScrollSpeedKeyId;
		static const std::string mIsHorizontalKeyId;
		static const std::string mIsVerticalKeyId;
		static const std::string mNormalizedScrollPositionKeyId;
	};


	const std::string TScrollableUIAreaArchiveKeys::mContentEntityRefKeyId = "content_entity_ref";
	const std::string TScrollableUIAreaArchiveKeys::mScrollSpeedKeyId = "scroll_speed";
	const std::string TScrollableUIAreaArchiveKeys::mIsHorizontalKeyId = "is_horizontal";
	const std::string TScrollableUIAreaArchiveKeys::mIsVerticalKeyId = "is_vertical";
	const std::string TScrollableUIAreaArchiveKeys::mNormalizedScrollPositionKeyId = "normalized_scroll_pos";

	E_RESULT_CODE CScrollableUIArea::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mContentEntityRef = static_cast<TEntityId>(pReader->GetUInt32(TScrollableUIAreaArchiveKeys::mContentEntityRefKeyId));

		mScrollSpeedFactor = pReader->GetFloat(TScrollableUIAreaArchiveKeys::mScrollSpeedKeyId, 100.0f);
		mIsHorizontal = pReader->GetBool(TScrollableUIAreaArchiveKeys::mIsHorizontalKeyId, true);
		mIsVertical = pReader->GetBool(TScrollableUIAreaArchiveKeys::mIsVerticalKeyId, true);

		pReader->BeginGroup(TScrollableUIAreaArchiveKeys::mNormalizedScrollPositionKeyId);
		
		auto posResult = LoadVector2(pReader);
		if (posResult.HasError())
		{
			return posResult.GetError();
		}

		mNormalizedScrollPosition = posResult.Get();

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
			pWriter->SetFloat(TScrollableUIAreaArchiveKeys::mScrollSpeedKeyId, mScrollSpeedFactor);

			pWriter->SetBool(TScrollableUIAreaArchiveKeys::mIsHorizontalKeyId, mIsHorizontal);
			pWriter->SetBool(TScrollableUIAreaArchiveKeys::mIsVerticalKeyId, mIsVertical);

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
			pComponent->mScrollSpeedFactor = mScrollSpeedFactor;
			pComponent->mIsHorizontal = mIsHorizontal;
			pComponent->mIsVertical = mIsVertical;

			return RC_OK;
		}

		return RC_FAIL;
	}

	void CScrollableUIArea::SetContentEntityId(TEntityId cursorId)
	{
		mContentEntityRef = cursorId;
	}

	void CScrollableUIArea::SetScrollSpeedFactor(F32 value)
	{
		mScrollSpeedFactor = value;
	}

	void CScrollableUIArea::SetLayoutPrepared(bool value)
	{
		mIsLayoutPrepared = value;
	}

	void CScrollableUIArea::SetHorizontal(bool state)
	{
		mIsHorizontal = state;
	}

	void CScrollableUIArea::SetVertical(bool state)
	{
		mIsVertical = state;
	}

	void CScrollableUIArea::SetNormalizedScrollPosition(const TVector2& value)
	{
		mNormalizedScrollPosition = TVector2(CMathUtils::Clamp01(value.x), CMathUtils::Clamp01(value.y));
	}
	
	TEntityId CScrollableUIArea::GetContentEntityId() const
	{
		return mContentEntityRef;
	}

	F32 CScrollableUIArea::GetScrollSpeedFactor() const
	{
		return mScrollSpeedFactor;
	}

	bool CScrollableUIArea::IsLayoutInitialized() const
	{
		return mIsLayoutPrepared;
	}

	bool CScrollableUIArea::IsHorizontal() const
	{
		return mIsHorizontal;
	}

	bool CScrollableUIArea::IsVertical() const
	{
		return mIsVertical;
	}

	const TVector2& CScrollableUIArea::GetNormalizedScrollPosition() const
	{
		return mNormalizedScrollPosition;
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