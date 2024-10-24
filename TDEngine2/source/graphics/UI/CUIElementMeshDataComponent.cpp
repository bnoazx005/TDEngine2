#include "../../../include/graphics/UI/CUIElementMeshDataComponent.h"


namespace TDEngine2
{
	struct TUIElementMeshDataArchiveKeys
	{
	};


	CUIElementMeshData::CUIElementMeshData() :
		CBaseComponent(), mTextureResourceId(TResourceId::Invalid), mIsFontMesh(false)
	{
	}

	E_RESULT_CODE CUIElementMeshData::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}
		
		return RC_OK;
	}

	E_RESULT_CODE CUIElementMeshData::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	void CUIElementMeshData::ResetMesh()
	{
		mVertices.clear();
		mIndices.clear();

		mMinBounds = TVector2((std::numeric_limits<F32>::max)());
		mMaxBounds = TVector2(-(std::numeric_limits<F32>::max)());
	}

	void CUIElementMeshData::AddVertex(const TUIElementsVertex& vertex)
	{
		const auto& pos = vertex.mPosUV;

		mMinBounds = TVector2(CMathUtils::Min(pos.x, mMinBounds.x), CMathUtils::Min(pos.y, mMinBounds.y));
		mMaxBounds = TVector2(CMathUtils::Max(pos.x, mMaxBounds.x), CMathUtils::Max(pos.y, mMaxBounds.y));

		mVertices.emplace_back(vertex);
	}

	void CUIElementMeshData::AddIndex(U16 value)
	{
		mIndices.push_back(value);
	}

	void CUIElementMeshData::SetTextMeshFlag(bool value)
	{
		mIsFontMesh = value;
	}

	E_RESULT_CODE CUIElementMeshData::SetTextureResourceId(TResourceId resourceId)
	{
		if (TResourceId::Invalid == resourceId)
		{
			return RC_INVALID_ARGS;
		}

		mTextureResourceId = resourceId;

		return RC_OK;
	}

	const CUIElementMeshData::TVertexArray& CUIElementMeshData::GetVertices() const
	{
		return mVertices;
	}

	const CUIElementMeshData::TIndexArray& CUIElementMeshData::GetIndices() const
	{
		return mIndices;
	}

	const TVector2& CUIElementMeshData::GetMinBound() const
	{
		return mMinBounds;
	}

	const TVector2& CUIElementMeshData::GetMaxBound() const
	{
		return mMaxBounds;
	}

	bool CUIElementMeshData::IsTextMesh() const
	{
		return mIsFontMesh;
	}

	TResourceId CUIElementMeshData::GetTextureResourceId() const
	{
		return mTextureResourceId;
	}


	IComponent* CreateUIElementMeshData(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CUIElementMeshData, result);
	}


	/*!
		\brief CUIElementsMeshDataFactory's definition
	*/

	CUIElementMeshDataFactory::CUIElementMeshDataFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CUIElementMeshDataFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateUIElementMeshData(result);
	}

	E_RESULT_CODE CUIElementMeshDataFactory::SetupComponent(CUIElementMeshData* pComponent, const TUIElementMeshDataParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateUIElementMeshDataFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CUIElementMeshDataFactory, result);
	}
}