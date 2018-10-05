/*!
	\file IVertexBuffer.h
	\date 03.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IBuffer.h"


namespace TDEngine2
{
	class IGraphicsContext;

	/*!
		interface IVertexBuffer

		\brief The interface describes a functionality of a hardware vertex buffer
	*/


	class IVertexBuffer : public IBuffer
	{
		public:
			/*!
				\brief The method initializes an initial state of a buffer

				\param[in] pGraphicsContext A pointer to implementation of IGraphicsContext interface
				\param[in] usageType A usage type of a buffer
				\param[in] totalBufferSize Total size of a buffer
				\param[in] pDataPtr A pointer to data that will initialize a buffer

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize, 
												void* pDataPtr) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IVertexBuffer)
	};
}