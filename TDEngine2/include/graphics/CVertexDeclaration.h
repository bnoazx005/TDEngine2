/*!
	\file CVertexDeclaration.h
	\date 27.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IVertexDeclaration.h"
#include "./../core/CBaseObject.h"
#include <vector>
#include <tuple>


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

			typedef std::vector<std::tuple<U32, U32>> TInstancingInfoArray;
		public:
			/*!
				\brief The method initializes a vertex declaration's object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method adds a new element within existing declaration

				\param[in] elementDesc A union object that contains element's parameters

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE AddElement(const TVertDeclElementDesc& elementDesc) override;

			/*!
				\brief The method adds a divisor into a vertex declaration that splits it into
				per vertex and per instance segments

				\param[in] index An index of an element within the declaration from which a per instance
				segment begins

				\param[in] instancesPerData A number of instances that will be passed between per instance
				data will be changed

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE AddInstancingDivisor(U32 index, U32 instancesPerData) override;

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
				\brief The method returns a size of current declaration in bytes. If the parameter's value greater than zero
				and there're a few instancing divisions within the declaration the method returns a size of all elements
				that are attached to the division which index was specified

				\param[in] sourceIndex An index specifies group of elements within the declaration

				\return The method returns a size of current declaration in bytes
			*/

			TDE2_API U32 GetStrideSize(U32 sourceIndex = 0) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVertexDeclaration)
		protected:
			TAttributesArray     mElements;

			TInstancingInfoArray mInstancingInfo;
	};
}