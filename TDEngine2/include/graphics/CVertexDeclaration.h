/*!
	\file CVertexDeclaration.h
	\date 27.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IVertexDeclaration.h"
#include "./../core/CBaseObject.h"
#include <vector>


namespace TDEngine2
{
	/*!
		class CVertexDeclaration

		\brief The class implements a common vertex declaration
	*/

	class CVertexDeclaration : public IVertexDeclaration, public CBaseObject
	{
		protected:
			typedef std::vector<TVertDeclElementDesc> TAttributesArray;
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

				\param[in] elementDesc A union object that contains element's parameters

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVertexDeclaration)

			TDE2_API U32 _getFormatSize(E_FORMAT_TYPE format) const;
		protected:
			TAttributesArray mElements;
	};
}