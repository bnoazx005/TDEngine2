/*!
	\file COGLBuffer.h
	\date 06.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/IBuffer.h>
#include <core/CBaseObject.h>


namespace TDEngine2
{
	


	/*!
		class COGLBuffer

		\brief The class is an implementation of a common OGL buffer, which
		can be used in different
	*/

	class COGLBuffer : public CBaseObject, public IBuffer
	{			
		public:
			friend TDE2_API IBuffer* CreateOGLBuffer(IGraphicsContext*, const TInitBufferParams&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an initial state of a buffer

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params) override;

			/*!
				\brief The method locks a buffer to provide safe data reading/writing

				\param[in] mapType A way the data should be processed during a lock

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Map(E_BUFFER_MAP_TYPE mapType) override;

			/*!
				\brief The method unlocks a buffer, so GPU can access to it after that operation
			*/

			TDE2_API void Unmap() override;

			/*!
				\brief The method writes data into a buffer

				\param[in] pData A pointer to data, which should be written
				\param[in] count A size of data in bytes

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Write(const void* pData, USIZE size) override;

			/*!
				\brief The method returns a pointer to buffer's data

				\return The method returns a pointer to buffer's data
			*/

			TDE2_API void* Read() override;

			/*!
				\brief The method recreates a buffer with a new specified size all previous data will be discarded
			*/

			TDE2_API E_RESULT_CODE Resize(USIZE newSize) override;

			/*!
				\brief The method returns an internal data of a buffer, which
				contains low-level platform specific buffer's handlers

				\return The method returns an internal data of a buffer, which
				contains low-level platform specific buffer's handlers
			*/

			TDE2_API void* GetInternalData() override;

			/*!
				\brief The method returns buffer's size in bytes

				\return The method returns buffer's size in bytes
			*/

			TDE2_API USIZE GetSize() const override;

			/*!
				\return The method returns an amount of occupied buffer's bytes
			*/

			TDE2_API USIZE GetUsedSize() const override;

			TDE2_API const TInitBufferParams& GetParams() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLBuffer)

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			GLuint                   mBufferHandler;

			USIZE                    mBufferSize;
			USIZE                    mUsedBytesSize;

			E_BUFFER_USAGE_TYPE      mBufferUsageType;

			E_BUFFER_TYPE            mBufferType;

			void*                    mpMappedBufferData;

			TInitBufferParams        mInitParams;

#if TDE2_DEBUG_MODE
			U8                       mLockChecker = 0;
#endif
	};


	/*!
		\brief A factory function for creation objects of COGLGraphicsContext's type

		\return A pointer to COGLGraphicsContext's implementation
	*/

	TDE2_API IBuffer* CreateOGLBuffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params, E_RESULT_CODE& result);
}