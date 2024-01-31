/*!
	\file CD3D11DepthBufferTarget.h
	\date 03.04.2020
	\authors Kasimov Ildar
*/

#pragma once


#include <core/CBaseObject.h>
#include <graphics/CBaseRenderTarget.h>


#if defined (TDE2_USE_WINPLATFORM)

#include <d3d11.h>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CD3D11DepthBufferTarget's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[in] params Additional parameters of a texture

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11DepthBufferTarget's implementation
	*/

	TDE2_API IDepthBufferTarget* CreateD3D11DepthBufferTarget(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
															  const TRenderTargetParameters& params, E_RESULT_CODE& result);


	/*!
		class CD3D11DepthBufferTarget

		\brief The class represents an implementation of a render target object for D3D11 GAPI
	*/

	class CD3D11DepthBufferTarget : public CBaseDepthBufferTarget
	{
		public:
			friend TDE2_API IDepthBufferTarget* CreateD3D11DepthBufferTarget(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
																			 const TRenderTargetParameters& params, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CD3D11DepthBufferTarget)

			/*!
				\brief The method binds a texture object to a given slot

				\param[in] slot Slot's index
			*/

			TDE2_API void Bind(U32 slot) override;

			TDE2_API void UnbindFromShader() override;

			/*!
				\brief The method loads resource data into memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load() override;

			/*!
				\brief The method unloads resource data from memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Unload() override;

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;

			/*!
				\brief The method copies existing data of the render target into given texture object

				\param[in] pDestTexture A pointer to ITexture2D implementation, the object should be already initialized

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Blit(ITexture2D*& pDestTexture) override;

			TDE2_API E_RESULT_CODE GenerateMipMaps() override;

			/*!
				\brief The method returns an internal view of the render target
				\return The method returns an internal view of the render target
			*/

			TDE2_API ID3D11DepthStencilView* GetDepthBufferTargetView() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11DepthBufferTarget)

			TDE2_API E_RESULT_CODE _createInternalTextureHandler(IGraphicsContext* pGraphicsContext, const TRenderTargetParameters& params) override;

			TDE2_API E_RESULT_CODE _createShaderTextureView(ID3D11Device* p3dDevice, E_FORMAT_TYPE format, U32 mipLevelsCount);

			TDE2_API E_RESULT_CODE _createDepthBufferTargetView(ID3D11Device* p3dDevice, E_FORMAT_TYPE format);
		protected:
			ID3D11Device*              mp3dDevice;

			ID3D11DeviceContext*       mp3dDeviceContext;

			ID3D11Texture2D*           mpDepthBufferTexture;

			ID3D11ShaderResourceView*  mpShaderTextureView;

			ID3D11DepthStencilView*    mpDepthBufferTargetView;

			ID3D11UnorderedAccessView* mpUavDepthBufferView;
	};


	/*!
		\brief A factory function for creation objects of CD3D11DepthBufferTargetFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11DepthBufferTargetFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateD3D11DepthBufferTargetFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CD3D11DepthBufferTargetFactory

		\brief The class is an abstract factory of CD3D11DepthBufferTarget objects that
		is used by a resource manager
	*/

	class CD3D11DepthBufferTargetFactory : public CBaseObject, public IDepthBufferTargetFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateD3D11DepthBufferTargetFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* Create(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* CreateDefault(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11DepthBufferTargetFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}

#endif