/*!
	\file CD3D11IndexBuffer.h
	\date 03.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/IIndexBuffer.h>
#include <core/IBaseObject.h>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	class IBuffer;
	class IGraphicsContext;


	/*!
		class CD3D11IndexBuffer

		\brief The class implements a functionality of a hardware index buffer for D3D11 GAPI
	*/


	class CD3D11IndexBuffer : public IIndexBuffer, public virtual IBaseObject
	{
		public:
			friend TDE2_API IIndexBuffer* CreateD3D11IndexBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, E_INDEX_FORMAT_TYPE indexFormatType,
																 U32 totalBufferSize, void* pDataPtr, E_RESULT_CODE& result);
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

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize,
										E_INDEX_FORMAT_TYPE indexFormatType, void* pDataPtr) override;

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
				\brief The method binds current index buffer to a rendering pipeline

				\param[in] offset An offset in bytes from the first index in a buffer
			*/

			TDE2_API void Bind(U32 offset) override;

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11IndexBuffer)
		protected:
			ID3D11DeviceContext * mp3dDeviceContext;

			ID3D11Buffer*         mpInternalIndexBuffer;

			bool                  mIsInitialized;

			IBuffer*              mpBufferImpl;
			
			E_INDEX_FORMAT_TYPE   mIndexFormatType;
	};


	/*!
		\brief A factory function for creation objects of CD3D11IndexBuffer's type

		\return A pointer to CD3D11IndexBuffer's implementation
	*/

	TDE2_API IIndexBuffer* CreateD3D11IndexBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, E_INDEX_FORMAT_TYPE indexFormatType,
												  U32 totalBufferSize, void* pDataPtr, E_RESULT_CODE& result);
}

#endif