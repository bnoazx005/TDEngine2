/*!
	\file COGLRenderTarget.h
	\date 09.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CBaseRenderTarget.h>
#include <GL/glew.h>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of COGLRenderTarget's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[in] params Additional parameters of a texture

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to COGLRenderTarget's implementation
	*/

	TDE2_API IRenderTarget* CreateOGLRenderTarget(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												  const TTexture2DParameters& params, E_RESULT_CODE& result);


	/*!
		class COGLRenderTarget

		\brief The class represents an implementation of a render target for OGL 3.x GAPI
	*/

	class COGLRenderTarget : public CBaseRenderTarget
	{
		public:
			friend TDE2_API IRenderTarget* CreateOGLRenderTarget(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
																 const TTexture2DParameters& params, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(COGLRenderTarget)

			/*!
				\brief The method binds a texture object to a given slot

				\param[in] slot Slot's index
			*/

			TDE2_API void Bind(U32 slot) override;

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

			/*!
				\brief The method returns an internal handler of the render target
				\return The method returns an internal handler of the render target
			*/

			TDE2_API GLuint GetInternalHandler() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLRenderTarget)

			TDE2_API E_RESULT_CODE _createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
																 U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality) override;

			TDE2_API E_RESULT_CODE _createFrameBufferHandler();
		protected:
			GLuint mTextureHandler;
			GLuint mFrameBufferHandler;
	};


	/*!
		\brief A factory function for creation objects of COGLRenderTargetFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to COGLRenderTargetFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateOGLRenderTargetFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class COGLRenderTargetFactory

		\brief The class is an abstract factory of COGLRenderTarget objects that
		is used by a resource manager
	*/

	class COGLRenderTargetFactory : public IRenderTargetFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateOGLRenderTargetFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

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

			TDE2_API U32 GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLRenderTargetFactory)
		protected:
			bool              mIsInitialized;

			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}