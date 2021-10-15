/*!
	\file CD3D11CubemapTexture.h
	\date 04.10.2019
	\authors Kasimov Ildar
*/

#pragma once


#include <core/CBaseObject.h>
#include <graphics/CBaseCubemapTexture.h>
#include <utils/Utils.h>


#if defined (TDE2_USE_WINPLATFORM)

#include <d3d11.h>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CD3D11CubemapTexture's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11CubemapTexture's implementation
	*/

	TDE2_API ICubemapTexture* CreateD3D11CubemapTexture(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);


	/*!
		\brief A factory function for creation objects of CD3D11CubemapTexture's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[in] params Additional parameters of a texture

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11Texture2D's implementation
	*/

	TDE2_API ICubemapTexture* CreateD3D11CubemapTexture(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
														const TTexture2DParameters& params, E_RESULT_CODE& result);

	/*!
		class CD3D11CubemapTexture

		\brief The class represents an implementation of a 2d texture
		for D3D11 GAPI
	*/

	class CD3D11CubemapTexture : public CBaseCubemapTexture
	{
		public:
			friend TDE2_API ICubemapTexture* CreateD3D11CubemapTexture(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);
			friend TDE2_API ICubemapTexture* CreateD3D11CubemapTexture(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
																	   const TTexture2DParameters& params, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CD3D11CubemapTexture)

			/*!
				\brief The method binds a texture object to a given slot

				\param[in] slot Slot's index
			*/

			TDE2_API void Bind(U32 slot) override;

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;

			/*!
				\brief The method writes data into a specified texture's region

				\param[in] face An index of cubemap's face
				\param[in] regionRect A region, which will be overwritten
				\param[in] pData Data that will be written into a given region

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE WriteData(E_CUBEMAP_FACE face, const TRectI32& regionRect, const U8* pData) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11CubemapTexture)

			TDE2_API E_RESULT_CODE _createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
															     U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality) override;

			TDE2_API E_RESULT_CODE _createShaderTextureView(ID3D11Device* p3dDevice, E_FORMAT_TYPE format, U32 mipLevelsCount);

			TDE2_API TResult<ID3D11Texture2D*> _createD3D11TextureResource(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
																		   U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality,
																		   U32 accessType = 0x0);
		protected:
			ID3D11Device*             mp3dDevice;

			ID3D11DeviceContext*      mp3dDeviceContext;
								       
			ID3D11Texture2D*          mpTexture;

			ID3D11ShaderResourceView* mpShaderTextureView;
	};


	/*!
		\brief A factory function for creation objects of CD3D11CubemapTextureFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11CubemapTextureFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateD3D11CubemapTextureFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CD3D11CubemapTextureFactory

		\brief The class is an abstract factory of CD3D11CubemapTexture objects that
		is used by a resource manager
	*/

	class CD3D11CubemapTextureFactory :public CBaseObject, public ICubemapTextureFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateD3D11CubemapTextureFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11CubemapTextureFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}

#endif