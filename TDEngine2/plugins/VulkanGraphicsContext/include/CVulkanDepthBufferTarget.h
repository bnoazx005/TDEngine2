/*!
	\file CVulkanDepthBufferTarget.h
	\date 10.11.2023
	\authors Kasimov Ildar
*/

#pragma once


#include <core/CBaseObject.h>
#include <graphics/CBaseRenderTarget.h>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CVulkanDepthBufferTarget's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
		\param[in] name A resource's name
		\param[in] params Additional parameters of a texture
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CVulkanDepthBufferTarget's implementation
	*/

	TDE2_API IDepthBufferTarget* CreateVulkanDepthBufferTarget(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
		const TRenderTargetParameters& params, E_RESULT_CODE& result);


	/*!
		class CVulkanDepthBufferTarget

		\brief The class represents an implementation of a render target object for Vulkan GAPI
	*/

	class CVulkanDepthBufferTarget : public CBaseDepthBufferTarget
	{
		public:
			friend TDE2_API IDepthBufferTarget* CreateVulkanDepthBufferTarget(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
				const TRenderTargetParameters& params, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CVulkanDepthBufferTarget)

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

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanDepthBufferTarget)

			TDE2_API E_RESULT_CODE _createInternalTextureHandler(IGraphicsContext* pGraphicsContext, const TRenderTargetParameters& params) override;

		protected:
	};


	/*!
		\brief A factory function for creation objects of CVulkanDepthBufferTargetFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CVulkanDepthBufferTargetFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateVulkanDepthBufferTargetFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CVulkanDepthBufferTargetFactory

		\brief The class is an abstract factory of CVulkanDepthBufferTarget objects that is used by a resource manager
	*/

	class CVulkanDepthBufferTargetFactory : public CBaseObject, public IDepthBufferTargetFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateVulkanDepthBufferTargetFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanDepthBufferTargetFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}