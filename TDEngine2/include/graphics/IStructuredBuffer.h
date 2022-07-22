/*!
	\file IStructuredBuffer.h
	\date 22.07.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/CBaseObject.h"
#include "IBuffer.h"


namespace TDEngine2
{
	class IGraphicsContext;


	enum class E_STRUCTURED_BUFFER_TYPE : U32
	{
		DEFAULT,
		APPEND,
		CONSUMED,
	};


	class IStructuredBuffer : public IBuffer
	{
		public:
			/*!
				\brief The method initializes an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, E_STRUCTURED_BUFFER_TYPE bufferType, E_BUFFER_USAGE_TYPE usageType,
												U32 elementsCount, U32 elementStride, bool isWriteable) = 0;

			/*!
				\brief The method binds current buffer to a pipeline

				\param[in] slot A binding slot's index
			*/

			TDE2_API virtual void Bind(U32 slot) = 0;

			TDE2_API virtual U32 GetStrideSize() const = 0;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(IStructuredBuffer)
	};
}