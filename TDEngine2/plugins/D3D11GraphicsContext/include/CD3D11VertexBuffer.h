/*!
	\file CD3D11VertexBuffer.h
	\date 05.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/IVertexBuffer.h>
#include <core/CBaseObject.h>


#if defined (TDE2_USE_WINPLATFORM)

namespace TDEngine2
{
	class IBuffer;
	class IGraphicsContext;


	/*!
		class CD3D11VertexBuffer

		\brief The class implements a functionality of a hardware vertex buffer for D3D11 GAPI
	*/


	class CD3D11VertexBuffer : public IVertexBuffer, public CBaseObject
	{
		public:
			friend TDE2_API IVertexBuffer* CreateD3D11VertexBuffer(IGraphicsContext*, E_BUFFER_USAGE_TYPE, USIZE, const void*, E_RESULT_CODE&);
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

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, USIZE totalBufferSize, const void* pDataPtr) override;

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
				\brief The method binds current vertex buffer to a rendering pipeline

				\param[in] slot A binding slot's index

				\param[in] offset An offset in bytes from a beginning of a vertex buffer, from
				which the data should be read

				\param[in] stride A size of a single vertex entry
			*/

			TDE2_API void Bind(U32 slot, U32 offset, U32 stride) override;

			/*!
				\brief The method links current vertex buffer with a given input layout

				\param[in] pInputLayout A pointer to ID3D11InputLayout object

				\param[in] stride A size of a single vertex element in bytes
			*/

			TDE2_API void SetInputLayout(ID3D11InputLayout* pInputLayout, USIZE stride);

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

			TDE2_API USIZE GetSize() const override;

			/*!
				\return The method returns an amount of occupied buffer's bytes
			*/

			TDE2_API USIZE GetUsedSize() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11VertexBuffer)

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			ID3D11DeviceContext* mp3dDeviceContext;

			IBuffer*             mpBufferImpl;

			ID3D11InputLayout*   mpInputLayout;
			
			ID3D11Buffer*        mpInternalVertexBuffer;

			USIZE                mVertexStride;
	};


	/*!
	\brief A factory function for creation objects of CD3D11VertexBuffer's type

	\return A pointer to CD3D11VertexBuffer's implementation
	*/

	TDE2_API IVertexBuffer* CreateD3D11VertexBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType,
												    USIZE totalBufferSize, const void* pDataPtr, E_RESULT_CODE& result);
}

#endif