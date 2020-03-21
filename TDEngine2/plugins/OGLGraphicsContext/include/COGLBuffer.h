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
			/*!
				\brief The enumeration contains all possible types, how COGLBuffer can be used
			*/

			enum E_BUFFER_TYPE
			{
				BT_VERTEX_BUFFER,			///< A buffer will be used as vertex buffer
				BT_INDEX_BUFFER,			///< A buffer will be used as index buffer
				BT_CONSTANT_BUFFER,			///< A buffer will be used as uniforms buffer object
			};
		public:
			friend TDE2_API IBuffer* CreateOGLBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, E_BUFFER_TYPE bufferType,
				U32 totalBufferSize, const void* pDataPtr, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an initial state of a buffer

				\param[in] pGraphicsContext A pointer to implementation of IGraphicsContext interface
				\param[in] usageType A usage type of a buffer
				\param[in] bufferType A type of a buffer
				\param[in] totalBufferSize Total size of a buffer
				\param[in] pDataPtr A pointer to data that will initialize a buffer

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, E_BUFFER_TYPE bufferType, U32 totalBufferSize,
										const void* pDataPtr);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

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

			TDE2_API E_RESULT_CODE Write(const void* pData, U32 size) override;

			/*!
				\brief The method returns a pointer to buffer's data

				\return The method returns a pointer to buffer's data
			*/

			TDE2_API void* Read() override;

			/*!
				\brief The method returns an internal data of a buffer, which
				contains low-level platform specific buffer's handlers

				\return The method returns an internal data of a buffer, which
				contains low-level platform specific buffer's handlers
			*/

			TDE2_API const TBufferInternalData& GetInternalData() const override;

			/*!
				\brief The method returns buffer's size in bytes

				\return The method returns buffer's size in bytes
			*/

			TDE2_API U32 GetSize() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLBuffer)

			TDE2_API GLenum _getBufferType(E_BUFFER_TYPE type) const;
		protected:
			GLuint                   mBufferHandler;

			U32                      mBufferSize;

			E_BUFFER_USAGE_TYPE      mBufferUsageType;

			E_BUFFER_TYPE            mBufferType;

			void*                    mpMappedBufferData;

			TBufferInternalData      mBufferInternalData;

#if TDE2_DEBUG_MODE
			U8                       mLockChecker = 0;
#endif
	};


	/*!
		\brief A factory function for creation objects of COGLGraphicsContext's type

		\return A pointer to COGLGraphicsContext's implementation
	*/

	TDE2_API IBuffer* CreateOGLBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, COGLBuffer::E_BUFFER_TYPE bufferType,
									  U32 totalBufferSize, const void* pDataPtr, E_RESULT_CODE& result);
}