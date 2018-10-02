/*!
	\file CD3D11GraphicsContext.h
	\date 21.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include "./../../core/IGraphicsContext.h"


#if defined(TDE2_USE_WIN32PLATFORM) /// Used only on Windows platform

namespace TDEngine2
{
	class CD3D11GraphicsContext : public IGraphicsContext
	{
		public:
			friend TDE2_API IGraphicsContext* CreateD3D11GraphicsContext(const IWindowSystem* pWindowSystem, E_RESULT_CODE& result);
		public:
			TDE2_API virtual ~CD3D11GraphicsContext();

			/*!
				\brief The method initializes an initial state of the object

				\param[in] pWindowSystem A pointer to implementation of IWindowSystem interface

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(const IWindowSystem* pWindowSystem) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method clears up back buffer with specified color

				\param[in] color The new color of a back buffer
			*/

			TDE2_API void ClearBackBuffer(const TColor32F& color) override;

			/*!
				\brief The method swaps a current buffer with a back one
			*/

			TDE2_API void Present() override;

			/*!
				\brief The method sets up a viewport's parameters

				\param[in] x x position of left hand side of a viewport
				\param[in] y y position of a left top corner of a viewport
				\param[in] width width of a viewport
				\param[in] height height of a viewport
				\param[in] minDepth minimum depth of a viewport
				\param[in] maxDepth maximum depth of a viewport
			*/

			TDE2_API void SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth, F32 maxDepth) override;

			/*!
				\brief The method returns an object that contains internal handlers that are used by the system.

				The structure of the object and its members can vary on different platforms.

				return The method returns an object that contains internal handlers that are used by the system
			*/

			TDE2_API const TGraphicsCtxInternalData& GetInternalData() const override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;
		protected:
			TDE2_API CD3D11GraphicsContext();
			TDE2_API CD3D11GraphicsContext(const CD3D11GraphicsContext& graphicsCtx) = delete;
			TDE2_API virtual CD3D11GraphicsContext& operator= (CD3D11GraphicsContext& graphicsCtx) = delete;

			TDE2_API E_RESULT_CODE _createSwapChain(const IWindowSystem* pWindowSystem, ID3D11Device* p3dDevice);

			TDE2_API E_RESULT_CODE _createBackBuffer(IDXGISwapChain* pSwapChain, ID3D11Device* p3dDevice);

			TDE2_API E_RESULT_CODE _createDepthBuffer(U32 width, U32 height, IDXGISwapChain* pSwapChain, ID3D11Device* p3dDevice,
													  ID3D11DepthStencilView** ppDepthStencilView, ID3D11Texture2D** pDepthStencilBuffer);
		protected:
			ID3D11Device*            mp3dDevice;
			ID3D11DeviceContext*     mp3dDeviceContext;
			TGraphicsCtxInternalData mInternalDataObject;
			bool                     mIsInitialized;

			IDXGISwapChain*          mpSwapChain;

			ID3D11RenderTargetView*  mpBackBufferView;

			ID3D11DepthStencilView*  mpDefaultDepthStencilView;
			ID3D11Texture2D*         mpDefaultDepthStencilBuffer;

			D3D_FEATURE_LEVEL        mCurrFeatureLevel; 
			#if _DEBUG
			ID3D11Debug*             mpDebuggerInstance;
			#endif

			bool                     mIsVSyncEnabled;
	};


	/*!
		\brief A factory function for creation objects of CD3D11GraphicsContext's type

		\return A pointer to CD3D11GraphicsContext's implementation
	*/

	TDE2_API IGraphicsContext* CreateD3D11GraphicsContext(const IWindowSystem* pWindowSystem, E_RESULT_CODE& result);
}

#endif