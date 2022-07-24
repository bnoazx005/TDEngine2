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
		APPENDABLE,
		CONSUMED,
	};


	struct TStructuredBuffersInitParams
	{
		IGraphicsContext*        mpGraphicsContext;
		E_STRUCTURED_BUFFER_TYPE mBufferType;				///< A type of a structured buffer (refers to different operations set in a shader)
		E_BUFFER_USAGE_TYPE      mUsageType;
		U32                      mElementsCount;			///< A maximum capacity of the buffer in elements
		USIZE                    mElementStride;			///< A number of bytes that's occupied by a single element
		bool                     mIsWriteable;				///< A flag that determines whether the buffer can be used as an output in a shader
		void*                    mpInitialData = nullptr;	///< A pointer to data that will initialize a buffer
		USIZE                    mInitialDataSize = 0;		///< A size of pInitialData
	};


	class IStructuredBuffer : public IBuffer
	{
		public:
			/*!
				\brief The method initializes an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const TStructuredBuffersInitParams& params) = 0;

			/*!
				\brief The method binds current buffer to a pipeline

				\param[in] slot A binding slot's index
			*/

			TDE2_API virtual void Bind(U32 slot) = 0;

			TDE2_API virtual USIZE GetStrideSize() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IStructuredBuffer)
	};
}