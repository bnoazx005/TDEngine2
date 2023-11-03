/*!
	\file CVulkanShader.h
	\date 03.11.2023
	\authors Kasimov Ildar
*/

#pragma once


#include <core/CBaseObject.h>
#include <graphics/CBaseShader.h>
#include <string>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;


	/*!
		\brief A factory function for creation objects of CVulkanShader's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CVulkanShader's implementation
	*/

	TDE2_API IShader* CreateVulkanShader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);

	/*!
		class CVulkanShader

		\brief The class is a common implementation for all platforms
	*/

	class CVulkanShader : public CBaseShader
	{
		public:
			friend TDE2_API IShader* CreateVulkanShader(IResourceManager*, IGraphicsContext*, const std::string&, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CVulkanShader)

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;

			/*!
				\brief The method binds a shader to a rendering pipeline
			*/

			TDE2_API void Bind() override;

			/*!
				\brief The method rejects a shader from a rendering pipeline
			*/

			TDE2_API void Unbind() override;

			TDE2_API VkPipelineShaderStageCreateInfo GetPipelineShaderStage(E_SHADER_STAGE_TYPE stageType) const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanShader)

			TDE2_API E_RESULT_CODE _createInternalHandlers(const TShaderCompilerOutput* pCompilerData) override;

			TDE2_API E_RESULT_CODE _createUniformBuffers(const TShaderCompilerOutput* pCompilerData);

			TDE2_API void _bindUniformBuffer(U32 slot, IConstantBuffer* pBuffer) override;
		protected:
			std::array<VkShaderModule, SST_NONE>                  mShaderStageModules;
			std::array<VkPipelineShaderStageCreateInfo, SST_NONE> mPipelineShaderStagesInfo;
			VkDevice                                              mDevice;
	};


	/*!
		\brief A factory function for creation objects of CVulkanShaderFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CVulkanShaderFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateVulkanShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CVulkanShaderFactory

		\brief The class is an abstract factory of CVulkanShader objects that
		is used by a resource manager
	*/

	class CVulkanShaderFactory : public CBaseObject, public IShaderFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateVulkanShaderFactory(IResourceManager*, IGraphicsContext*, E_RESULT_CODE&);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanShaderFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}