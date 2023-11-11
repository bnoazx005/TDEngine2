/*!
	\file CD3D11Buffer.h
	\date 03.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/IBuffer.h>
#include <core/CBaseObject.h>


#if defined (TDE2_USE_WINPLATFORM)


namespace TDEngine2
{
	/*!
		class CD3D11Buffer

		\brief The class is an implementation of a common D3D11 buffer, which 
		can be used in different 
	*/

	class CD3D11Buffer : public CBaseObject, public IBuffer
	{
		public:
			friend TDE2_API IBuffer* CreateD3D11Buffer(IGraphicsContext*, const TInitBufferParams&, E_RESULT_CODE&);
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

			/*!
				\brief The method returns a pointer to ID3D11DeviceContext implementation

				\return The method returns a pointer to ID3D11DeviceContext implementation
			*/

			TDE2_API ID3D11DeviceContext* GetDeviceContext() const;

			TDE2_API const TInitBufferParams& GetParams() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11Buffer)
			
			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			ID3D11DeviceContext*     mp3dDeviceContext;

			ID3D11Buffer*            mpBufferInstance;
			
			USIZE                    mBufferSize;
			USIZE                    mUsedBytesSize;
			USIZE                    mElementStrideSize;

			E_BUFFER_USAGE_TYPE      mBufferUsageType;

			E_BUFFER_TYPE            mBufferType;

			U32                      mAccessFlags;

			D3D11_MAPPED_SUBRESOURCE mMappedBufferData;

			TInitBufferParams        mInitParams;
	};


	/*!
		\brief A factory function for creation objects of CD3D11GraphicsContext's type

		\return A pointer to CD3D11GraphicsContext's implementation
	*/

	TDE2_API IBuffer* CreateD3D11Buffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params, E_RESULT_CODE& result);
}

#endif