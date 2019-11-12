/*!
	\file IIndexBuffer.h
	\date 03.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IBuffer.h"


namespace TDEngine2
{
	class IGraphicsContext;


	/*!
		enumeration E_INDEX_FORMAT_TYPE

		\brief The enumeration contains available formats for a single index, which is stored
		within an index buffer
	*/

	enum E_INDEX_FORMAT_TYPE : U8
	{
		IFT_INDEX16 = sizeof(U16),		///< 16 bits for a single index
		IFT_INDEX32 = sizeof(U32)		///< 32 bits for a single index
	};


	/*!
		interface IIndexBuffer

		\brief The interface describes a functionality of a hardware index buffer
	*/


	class IIndexBuffer : public IBuffer
	{
		public:
			/*!
				\brief The method initializes an initial state of a buffer

				\param[in] pGraphicsContext A pointer to implementation of IGraphicsContext interface
				\param[in] usageType A usage type of a buffer
				\param[in] totalBufferSize Total size of a buffer
				\param[in] indexFormatType A value, which defines single index's stride size
				\param[in] pDataPtr A pointer to data that will initialize a buffer

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize,
												E_INDEX_FORMAT_TYPE indexFormatType, const void* pDataPtr) = 0;

			/*!
				\brief The method binds current index buffer to a rendering pipeline

				\param[in] offset An offset in bytes from the first index in a buffer
			*/

			TDE2_API virtual void Bind(U32 offset) = 0;

			/*!
				\brief The method returns a format of a single stored index

				\return The method returns a format of a single stored index
			*/

			TDE2_API virtual E_INDEX_FORMAT_TYPE GetIndexFormat() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IIndexBuffer)
	};
}