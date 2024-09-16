/*!
	\file CD3D11Texture2D.h
	\date 20.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <core/CBaseObject.h>
#include <graphics/CBaseTexture2D.h>
#include <utils/Utils.h>


#if defined (TDE2_USE_WINPLATFORM)

#include <d3d11.h>


namespace TDEngine2
{
	enum E_D3D11_TEXTURE_ACCESS_TYPE: U32
	{
		DTAT_CPU_READ  = 0x1,
		DTAT_CPU_WRITE = 0x2,
		DTAT_GPU_READ  = 0x4,
		DTAT_GPU_WRITE = 0x8,
	};


	ITextureImpl* CreateD3D11TextureImpl(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params, E_RESULT_CODE& result);


	/*!
		class CD3D11TextureImpl
	*/
	
	class CD3D11TextureImpl : public virtual ITextureImpl, public CBaseObject
	{
		public:
			friend ITextureImpl* CreateD3D11TextureImpl(IGraphicsContext*, const TInitTextureImplParams&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an initial state of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params) override;

			E_RESULT_CODE Resize(U32 width, U32 height, U32 depth = 1) override;

			ID3D11Resource* GetTextureResource();
			ID3D11ShaderResourceView* GetShaderResourceView();

			ID3D11RenderTargetView* GetRenderTargetView();
			ID3D11DepthStencilView* GetDepthStencilView();
			ID3D11UnorderedAccessView* GetUnorderedAccessView();

			std::vector<U8> ReadBytes(U32 index) override;

			const TInitTextureParams& GetParams() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11TextureImpl)

			E_RESULT_CODE _onInitInternal();
			E_RESULT_CODE _onFreeInternal() override;
		protected:
			TInitTextureImplParams     mInitParams;

			ID3D11Device*              mp3dDevice = nullptr;
			ID3D11DeviceContext*       mp3dDeviceContext = nullptr;

			ID3D11Resource*            mpTextureResource = nullptr;

			ID3D11ShaderResourceView*  mpShaderTextureView = nullptr;

			// optional views
			ID3D11UnorderedAccessView* mpUavTextureView = nullptr;
			ID3D11RenderTargetView*    mpRenderTargetView = nullptr;
			ID3D11DepthStencilView*    mpDepthStencilView = nullptr;
	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(CD3D11TextureImpl)
}

#endif