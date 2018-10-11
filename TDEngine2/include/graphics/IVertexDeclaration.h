/*!
	\file IVertexDeclaration.h
	\date 11.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"


namespace TDEngine2
{
	/*!
		union TVertDeclElementDesc

		\brief The union contains fields that are needed to 
		create a new element withit an object of IVertexDeclaration type
	*/

	typedef union TVertDeclElementDesc
	{
#if defined (TDE2_USE_WIN32PLATFORM)
		typedef struct TD3D11VertDeclElementDesc
		{
			U32                     mInputSlot;
			E_VERTEX_ELEMENT_SEMANTIC_TYPE mSemanticType;
			E_FORMAT_TYPE           mFormatType;
		} TD3D11VertDeclElementDesc;

		TD3D11VertDeclElementDesc mD3D11;
#endif

		typedef struct TOGLVertDeclElementDesc
		{
			E_FORMAT_TYPE mFormatType;
		} TOGLVertDeclElementDesc;

		TOGLVertDeclElementDesc mOGL;
	} TElementDesc, *TElementDescPtr;


	/*!
		interface IVertexDeclaration

		\brief The interface describes a vertex declaration, which
		stores an information about a single vertex's structure
	*/

	class IVertexDeclaration
	{
		public:
			/*!
				\brief The method initializes a vertex declaration's object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method adds a new element within existing declaration

				\param[in] A union object that contains element's parameters

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE AddElement(const TVertDeclElementDesc& elementDesc) = 0;

			/*!
				\brief The method removes specified entity, which placed at the index-th slot

				\param[in] index A slot's index

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE RemoveElement(U32 index) = 0;

			/*!
				\brief The method returns current number of elements within the declaration

				\return  The method returns current number of elements within the declaration
			*/

			TDE2_API virtual U32 GetElementsCount() const = 0;

			/*!
				\brief The method returns a size of current declaration in bytes

				\return The method returns a size of current declaration in bytes
			*/

			TDE2_API virtual U32 GetStrideSize() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IVertexDeclaration)
	};
}
