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


	typedef struct TUIElementsVertex
	{
		TVector4  mPos;
		TVector2  mUV;
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

	class CUIElementMeshData : public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateUIElementMeshData(E_RESULT_CODE& result);
		public:
			typedef std::vector<TUIElementsVertex> TVertexArray;
			typedef std::vector<U16> TIndexArray;
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CUIElementMeshData)

			/*!
				\brief The method initializes an internal state of a quad sprite

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init();

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

			TDE2_API void AddVertex(const TUIElementsVertex& vertex);
			TDE2_API void AddIndex(U16 value);

			TDE2_API void SetFontMeshFlag(bool value);

			TDE2_API const TVertexArray& GetVertices() const;
			TDE2_API const TIndexArray& GetIndices() const;

			TDE2_API bool IsFontMesh() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CUIElementMeshData)
		protected:
			TVertexArray mVertices;
			TIndexArray  mIndices;

			bool mIsFontMesh;
	};


	/*!
		struct TUIElementMeshDataParameters

		\brief The structure contains parameters for creation of CUIElementMeshData
	*/

	typedef struct TUIElementMeshDataParameters : public TBaseComponentParameters
	{
	} TUIElementMeshDataParameters;


	/*!
		\brief A factory function for creation objects of CUIElementMeshDataFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CUIElementMeshDataFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateUIElementMeshDataFactory(E_RESULT_CODE& result);


	/*!
		class CUIElementMeshDataFactory

		\brief The class is factory facility to create a new objects of CUIElementMeshData type
	*/

	class CUIElementMeshDataFactory : public IComponentFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateUIElementMeshDataFactory(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init();

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* Create(const TBaseComponentParameters* pParams) const override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* CreateDefault(const TBaseComponentParameters& params) const override;

			/*!
				\brief The method returns an identifier of a component's type, which
				the factory serves

				\return The method returns an identifier of a component's type, which
				the factory serves
			*/

			TDE2_API TypeId GetComponentTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CUIElementMeshDataFactory)
	};
}