/*!
	\file IConstantBuffer.h
	\date 07.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IBuffer.h"


namespace TDEngine2
{
	class IGraphicsContext;

	/*!
		interface IConstantBuffer

		\brief The interface describes a functionality of a hardware constant buffer,
		which stores shader uniforms as data blocks within GPU's memory
	*/


	class IConstantBuffer : public IBuffer
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

			TDE2_API virtual E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, USIZE totalBufferSize, const void* pDataPtr) = 0;

			/*!
				\brief The method binds a constant buffer to a given slot

				\param[in] slot An index of a slot, in which the constant buffer will be binded to
			*/

			TDE2_API virtual void Bind(U32 slot) = 0;

			/*!
				\brief The method unbinds a constant buffer from rendering pipeline
			*/

			TDE2_API virtual void Unbind() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IConstantBuffer)
	};
}