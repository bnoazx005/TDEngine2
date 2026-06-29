/*!
	\file CUIElementMeshDataComponent.h
	\date 27.04.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../math/TVector2.h"
#include "../../math/TVector4.h"
#include "../../math/TRect.h"
#include "../../utils/Color.h"
#include <vector>


namespace TDEngine2
{
	enum class TResourceId : U32; 
	enum class E_UI_MATERIAL_TYPE : U8;


	typedef struct TUIElementsVertex
	{
		TVector4  mPosUV; // xy - position, zw - uv
		TColor32F mColor;
	} TUIElementsVertex, *TUIElementsVertexPtr;


	TDE2_API inline bool operator== (const TUIElementsVertex& left, const TUIElementsVertex& right) { return left.mPosUV == right.mPosUV && left.mColor == right.mColor; }


	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TUIElementMeshComponentData
	{
		typedef std::vector<TUIElementsVertex> TVertexArray;
		typedef std::vector<U16> TIndexArray;

		TVertexArray       mVertices;
		TIndexArray        mIndices;

		TVector2           mMinBounds;
		TVector2           mMaxBounds;

		bool               mIsFontMesh = false;
		E_UI_MATERIAL_TYPE mUIMaterialType;

		TResourceId        mTextureResourceId = TResourceId::Invalid;

		TDE2_DECLARE_COMPONENT_META(TUIElementMeshComponentData);
	};


	/*!
		\brief A factory function for creation objects of CUIElementMeshData's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CUIElementMeshData's implementation
	*/

	TDE2_API IComponent* CreateUIElementMeshData(E_RESULT_CODE& result);


	/*!
		class CUIElementMeshData
	*/

	class CUIElementMeshData : public CBaseComponentT<CUIElementMeshData, TUIElementMeshComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateUIElementMeshData(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CUIElementMeshData)

			TDE2_API void ResetMesh();

			TDE2_API void AddVertex(const TUIElementsVertex& vertex);
			TDE2_API void AddIndex(U16 value);

			TDE2_API void SetTextMeshFlag(bool value);
			TDE2_API void SetMaterialType(E_UI_MATERIAL_TYPE type);

			TDE2_API E_RESULT_CODE SetTextureResourceId(TResourceId resourceId);

			TDE2_API const TUIElementMeshComponentData::TVertexArray& GetVertices() const;
			TDE2_API const TUIElementMeshComponentData::TIndexArray& GetIndices() const;

			TDE2_API const TVector2& GetMinBound() const;
			TDE2_API const TVector2& GetMaxBound() const;

			TDE2_API TResourceId GetTextureResourceId() const;

			TDE2_API bool IsTextMesh() const;
			TDE2_API E_UI_MATERIAL_TYPE GetMaterialType() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CUIElementMeshData)
	};


	/*!
		struct TUIElementMeshDataParameters

		\brief The structure contains parameters for creation of CUIElementMeshData
	*/

	typedef struct TUIElementMeshDataParameters : public TBaseComponentParameters
	{
	} TUIElementMeshDataParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(UIElementMeshData, TUIElementMeshDataParameters);
}