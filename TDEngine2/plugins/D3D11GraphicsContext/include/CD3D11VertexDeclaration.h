/*!
	\file CD3D11VertexDeclaration.h
	\date 11.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/IVertexDeclaration.h>
#include <core/IBaseObject.h>
#include <vector>



#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CD3D11VertexDeclaration's type

		\return A pointer to CD3D11VertexDeclaration's implementation
	*/

	TDE2_API IVertexDeclaration* CreateD3D11VertexDeclaration(E_RESULT_CODE& result);


	/*!
		class CD3D11VertexDeclaration

		\brief The class implements a vertex declaration for D3D11
	*/

	class CD3D11VertexDeclaration : public IVertexDeclaration, public virtual IBaseObject
	{
		public:
			friend TDE2_API IVertexDeclaration* CreateD3D11VertexDeclaration(E_RESULT_CODE& result);
		protected:
			typedef std::vector<TVertDeclElementDesc::TD3D11VertDeclElementDesc> TAttributesArray;
		public:
			/*!
				\brief The method initializes a vertex declaration's object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method adds a new element within existing declaration

				\param[in] A union object that contains element's parameters

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE AddElement(const TVertDeclElementDesc& elementDesc) override;

			/*!
				\brief The method removes specified entity, which placed at the index-th slot

				\param[in] index A slot's index

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RemoveElement(U32 index) override;

			/*!
				\brief The method returns current number of elements within the declaration

				\return  The method returns current number of elements within the declaration
			*/

			TDE2_API U32 GetElementsCount() const override;

			/*!
				\brief The method returns a size of current declaration in bytes

				\return The method returns a size of current declaration in bytes
			*/

			TDE2_API U32 GetStrideSize() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11VertexDeclaration)
		protected:
			bool             mIsInitialized;

			TAttributesArray mElements;
	};
}

#endif