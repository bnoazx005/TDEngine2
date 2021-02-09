/*!
	\file COGLDepthBufferTarget.h
	\date 07.02.2021
	\authors Kasimov Ildar
*/

#pragma once


#include <core/CBaseObject.h>
#include <graphics/CBaseRenderTarget.h>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of COGLDepthBufferTarget's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
		\param[in] name A resource's name
		\param[in] params Additional parameters of a texture
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to COGLDepthBufferTarget's implementation
	*/

	TDE2_API IDepthBufferTarget* CreateOGLDepthBufferTarget(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
		const TTexture2DParameters& params, E_RESULT_CODE& result);


	/*!
		class COGLDepthBufferTarget

		\brief The class represents an implementation of a render target object for GL GAPI
	*/

	class COGLDepthBufferTarget : public CBaseDepthBufferTarget
	{
		public:
			friend TDE2_API IDepthBufferTarget* CreateOGLDepthBufferTarget(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
				const TTexture2DParameters& params, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(COGLDepthBufferTarget)

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
				\brief The method returns an internal handler of the depth buffer target
				\return The method returns an internal handler of the depth buffer target
			*/

			TDE2_API GLuint GetInternalHandler() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLDepthBufferTarget)

			TDE2_API E_RESULT_CODE _createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
				U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality) override;

		protected:
			GLuint mDepthBufferHandle;
	};


	/*!
		\brief A factory function for creation objects of COGLDepthBufferTargetFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to COGLDepthBufferTargetFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateOGLDepthBufferTargetFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class COGLDepthBufferTargetFactory

		\brief The class is an abstract factory of COGLDepthBufferTarget objects that is used by a resource manager
	*/

	class COGLDepthBufferTargetFactory : public CBaseObject, public IDepthBufferTargetFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateOGLDepthBufferTargetFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
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

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLDepthBufferTargetFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}