#include "../../../include/graphics/UI/CUIElementMeshDataComponent.h"
#include "../../../include/ecs/CUIElementsRenderSystem.h"
#define META_EXPORT_ECS_SECTION
#include "../../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateUIElementMeshDataFactory)
	TDE2_DEFINE_COMPONENT_META(TUIElementMeshComponentData)


	CUIElementMeshData::CUIElementMeshData() :
		CBaseComponentT()
	{
	}

	void CUIElementMeshData::ResetMesh()
	{
		mData.mVertices.clear();
		mData.mIndices.clear();

		mData.mMinBounds = TVector2((std::numeric_limits<F32>::max)());
		mData.mMaxBounds = TVector2(-(std::numeric_limits<F32>::max)());
	}

	void CUIElementMeshData::AddVertex(const TUIElementsVertex& vertex)
	{
		const auto& pos = vertex.mPosUV;

		mData.mMinBounds = TVector2(CMathUtils::Min(pos.x, mData.mMinBounds.x), CMathUtils::Min(pos.y, mData.mMinBounds.y));
		mData.mMaxBounds = TVector2(CMathUtils::Max(pos.x, mData.mMaxBounds.x), CMathUtils::Max(pos.y, mData.mMaxBounds.y));

		mData.mVertices.emplace_back(vertex);
	}

	void CUIElementMeshData::AddIndex(U16 value)
	{
		mData.mIndices.push_back(value);
	}

	void CUIElementMeshData::SetTextMeshFlag(bool value)
	{
		mData.mIsFontMesh = value;
	}

	void CUIElementMeshData::SetMaterialType(E_UI_MATERIAL_TYPE type)
	{
		mData.mUIMaterialType = type;
	}

	E_RESULT_CODE CUIElementMeshData::SetTextureResourceId(TResourceId resourceId)
	{
		if (TResourceId::Invalid == resourceId)
		{
			return RC_INVALID_ARGS;
		}

		mData.mTextureResourceId = resourceId;

		return RC_OK;
	}

	const TUIElementMeshComponentData::TVertexArray& CUIElementMeshData::GetVertices() const
	{
		return mData.mVertices;
	}

	const TUIElementMeshComponentData::TIndexArray& CUIElementMeshData::GetIndices() const
	{
		return mData.mIndices;
	}

	const TVector2& CUIElementMeshData::GetMinBound() const
	{
		return mData.mMinBounds;
	}

	const TVector2& CUIElementMeshData::GetMaxBound() const
	{
		return mData.mMaxBounds;
	}

	bool CUIElementMeshData::IsTextMesh() const
	{
		return mData.mIsFontMesh;
	}

	E_UI_MATERIAL_TYPE CUIElementMeshData::GetMaterialType() const
	{
		return mData.mUIMaterialType;
	}

	TResourceId CUIElementMeshData::GetTextureResourceId() const
	{
		return mData.mTextureResourceId;
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