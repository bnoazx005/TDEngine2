/*!
	\file IVertexDeclaration.h
	\date 11.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../core/IBaseObject.h"


namespace TDEngine2
{
	class IGraphicsContext;
	class IShader;
	class IVertexBuffer;


	/*!
		struct TVertDeclElementDesc

		\brief The union contains fields that are needed to 
		create a new element withit an object of IVertexDeclaration type
	*/

	typedef struct TVertDeclElementDesc
	{
		E_FORMAT_TYPE                  mFormatType;

		U32                            mSource;		// for D3D this value means an input slot's index, an implementation for OGL ignores this attribute

		E_VERTEX_ELEMENT_SEMANTIC_TYPE mSemanticType;

		bool                           mIsPerInstanceData;
	} TVertDeclElementDesc, *TVertDeclElementDescPtr;
	

	/*!
		interface IVertexDeclaration

		\brief The interface describes a vertex declaration, which
		stores an information about a single vertex's structure
	*/

	class IVertexDeclaration: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes a vertex declaration's object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method adds a new element within existing declaration

				\param[in] elementDesc An object that contains element's parameters

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE AddElement(const TVertDeclElementDesc& elementDesc) = 0;

			/*!
				\brief The method adds a divisor into a vertex declaration that splits it into
				per vertex and per instance segments

				\param[in] index An index of an element within the declaration from which a per instance
				segment begins

				\param[in] instancesPerData A number of instances that will be passed between per instance
				data will be changed

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE AddInstancingDivisor(U32 index, U32 instancesPerData) = 0;

			/*!
				\brief The method removes specified entity, which placed at the index-th slot

				\param[in] index A slot's index

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE RemoveElement(U32 index) = 0;

			/*!
				\brief The method creates an internal handlers for a vertex declaration and binds it
				to a rendering pipeline

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

				\param[in, out] pVertexBuffer A pointer to IVertexBuffer implementation

				\param[in, out] pShader A pointer to IShader implementation
			*/

			TDE2_API virtual void Bind(IGraphicsContext* pGraphicsContext, IVertexBuffer* pVertexBuffer, IShader* pShader) = 0;

			/*!
				\brief The method returns current number of elements within the declaration

				\return  The method returns current number of elements within the declaration
			*/

			TDE2_API virtual U32 GetElementsCount() const = 0;

			/*!
				\brief The method returns a size of current declaration in bytes. If the parameter's value greater than zero 
				and there're a few instancing divisions within the declaration the method returns a size of all elements 
				that are attached to the division which index was specified

				\param[in] sourceIndex An index specifies group of elements within the declaration

				\return The method returns a size of current declaration in bytes
			*/

			TDE2_API virtual U32 GetStrideSize(U32 sourceIndex = 0) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IVertexDeclaration)
	};
}
