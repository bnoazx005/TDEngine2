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


	/*!
		\brief A factory function for creation objects of CUIElementMeshData's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CUIElementMeshData's implementation
	*/

	TDE2_API IComponent* CreateUIElementMeshData(E_RESULT_CODE& result);


	/*!
		class CUIElementMeshData
	*/

	class CUIElementMeshData : public CBaseComponent, public CPoolMemoryAllocPolicy<CUIElementMeshData, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateUIElementMeshData(E_RESULT_CODE& result);
		public:
			typedef std::vector<TUIElementsVertex> TVertexArray;
			typedef std::vector<U16> TIndexArray;
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CUIElementMeshData)

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

			/*!
				\brief The method creates a new deep copy of the instance and returns a smart pointer to it.
				The original state of the object stays the same

				\param[in] pDestObject A valid pointer to an object which the properties will be assigned into
			*/

			TDE2_API E_RESULT_CODE Clone(IComponent*& pDestObject) const override;

			TDE2_API void ResetMesh();

			TDE2_API void AddVertex(const TUIElementsVertex& vertex);
			TDE2_API void AddIndex(U16 value);

			TDE2_API void SetTextMeshFlag(bool value);
			TDE2_API void SetMaterialType(E_UI_MATERIAL_TYPE type);

			TDE2_API E_RESULT_CODE SetTextureResourceId(TResourceId resourceId);

			TDE2_API const TVertexArray& GetVertices() const;
			TDE2_API const TIndexArray& GetIndices() const;

			TDE2_API const TVector2& GetMinBound() const;
			TDE2_API const TVector2& GetMaxBound() const;

			TDE2_API TResourceId GetTextureResourceId() const;

			TDE2_API bool IsTextMesh() const;
			TDE2_API E_UI_MATERIAL_TYPE GetMaterialType() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CUIElementMeshData)
		protected:
			TVertexArray mVertices;
			TIndexArray  mIndices;

			TVector2     mMinBounds;
			TVector2     mMaxBounds;

			bool mIsFontMesh;
			E_UI_MATERIAL_TYPE mUIMaterialType;

			TResourceId mTextureResourceId;
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