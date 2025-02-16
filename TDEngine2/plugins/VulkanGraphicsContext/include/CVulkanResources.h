/*!
	\file CVulkanResources.h
	\date 28.10.2024
	\authors Kasimov Ildar
*/

#pragma once


#include <core/CBaseObject.h>
#include <graphics/IBuffer.h>
#include <graphics/CBaseShader.h>
#include <graphics/CBaseTexture2D.h>
#include <graphics/CVertexDeclaration.h>
#include <graphics/BasePipelines.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>
#include "vk_mem_alloc.h"


namespace TDEngine2
{
	class CVulkanGraphicsContext;
	class IGraphicsPipeline;
	class IComputePipeline;
	class CVulkanGraphicsObjectManager;
	struct TRenderPassInfo;


	/*!
		class CVulkanBuffer

		\brief The class is an implementation of a common Vulkan buffer, which can be used in different situations like
		vertex / index / uniform / storage / etc
	*/

	class CVulkanBuffer : public CBaseObject, public IBuffer
	{			
		public:
			friend IBuffer* CreateVulkanBuffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an initial state of a buffer

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params);

			/*!
				\brief The method locks a buffer to provide safe data reading/writing

				\param[in] mapType A way the data should be processed during a lock

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Map(E_BUFFER_MAP_TYPE mapType, USIZE offset = 0) override;

			/*!
				\brief The method unlocks a buffer, so GPU can access to it after that operation
			*/

			void Unmap() override;

			/*!
				\brief The method writes data into a buffer

				\param[in] pData A pointer to data, which should be written
				\param[in] count A size of data in bytes

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Write(const void* pData, USIZE size) override;

			/*!
				\brief The method returns a pointer to buffer's data

				\return The method returns a pointer to buffer's data
			*/

			void* Read() override;

			/*!
				\brief The method recreates a buffer with a new specified size all previous data will be discarded
			*/

			E_RESULT_CODE Resize(USIZE newSize) override;

			/*!
				\brief The method returns an internal data of a buffer, which
				contains low-level platform specific buffer's handlers

				\return The method returns an internal data of a buffer, which
				contains low-level platform specific buffer's handlers
			*/

			void* GetInternalData() override;

			/*!
				\brief The method returns buffer's size in bytes

				\return The method returns buffer's size in bytes
			*/

			USIZE GetSize() const override;

			VkBuffer GetVulkanHandle();

			const TInitBufferParams& GetParams() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanBuffer)

			E_RESULT_CODE _discardCurrentBuffer(USIZE newSize, E_STRUCTURED_BUFFER_TYPE structuredBufferType);
			E_RESULT_CODE _onFreeInternal() override;			
		protected:
			USIZE                    mBufferSize = 0;

			E_BUFFER_USAGE_TYPE      mBufferUsageType = E_BUFFER_USAGE_TYPE::DEFAULT;

			E_BUFFER_TYPE            mBufferType = E_BUFFER_TYPE::GENERIC;

			void*                    mpMappedBufferData = nullptr;

			TBufferInternalData      mBufferInternalData;

#if TDE2_DEBUG_MODE
			U8                       mLockChecker = 0;
#endif

			CVulkanGraphicsContext*  mpGraphicsContextImpl = nullptr;

			VkBuffer                 mInternalBufferHandle = VK_NULL_HANDLE;
			VmaAllocator             mAllocator = VK_NULL_HANDLE;
			VmaAllocation            mAllocation = VK_NULL_HANDLE;
			VkDevice                 mDevice = VK_NULL_HANDLE;

			bool                     mIsUnorderedAccessResource = false;

			TInitBufferParams        mInitParams;
	};


	/*!
		\brief A factory function for creation objects of CVulkanGraphicsContext's type

		\return A pointer to CVulkanGraphicsContext's implementation
	*/

	IBuffer* CreateVulkanBuffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params, E_RESULT_CODE& result);


	/*!
		\brief A factory function for creation objects of CVulkanShader's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CVulkanShader's implementation
	*/

	IShader* CreateVulkanShader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);


	struct TVulkanPipelineLayoutInfo
	{
		struct TBindingInfo
		{
			enum class E_TYPE { BUFFER, TEXTURE };

			U32    mSlot = 0;
			E_TYPE mType = E_TYPE::TEXTURE;
		};

		std::vector<U32>          mCBVActiveSlots {};
		std::vector<TBindingInfo> mSRVActiveSlots {};
		std::vector<TBindingInfo> mUAVActiveSlots {};
		std::vector<U32>          mSamplersActiveSlots {};
	};


	/*!
		class CVulkanShader

		\brief The class is a common implementation for all platforms
	*/

	class CVulkanShader : public CBaseShader
	{
		public:
			friend IShader* CreateVulkanShader(IResourceManager*, IGraphicsContext*, const std::string&, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CVulkanShader)

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Reset() override;

			/*!
				\brief The method binds a shader to a rendering pipeline
			*/

			void Bind() override;

			/*!
				\brief The method rejects a shader from a rendering pipeline
			*/

			void Unbind() override;

			VkPipelineShaderStageCreateInfo GetPipelineShaderStage(E_SHADER_STAGE_TYPE stageType) const;
			VkPipelineShaderStageCreateInfo* GetStages();
			U32 GetStagesCount() const;

			const VkPipelineLayout GetPipelineLayout() const;
			const VkDescriptorSetLayout GetDescriptorSetLayout() const;

			const TVulkanPipelineLayoutInfo& GetLayoutInfo() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanShader)

			E_RESULT_CODE _createInternalHandlers(const TShaderCompilerOutput* pCompilerData) override;

			E_RESULT_CODE _createUniformBuffers(const TShaderCompilerOutput* pCompilerData);

		protected:
			std::array<VkShaderModule, SST_NONE>                  mShaderStageModules;
			std::array<VkPipelineShaderStageCreateInfo, SST_NONE> mPipelineShaderStagesInfo;
			VkDevice                                              mDevice = VK_NULL_HANDLE;
			VkPipelineLayout                                      mPipelineLayout = VK_NULL_HANDLE;
			VkDescriptorSetLayout                                 mDescriptorsSetLayout = VK_NULL_HANDLE;

			TVulkanPipelineLayoutInfo                             mLayoutInfo{};
	};


	/*!
		\brief A factory function for creation objects of CVulkanShaderFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CVulkanShaderFactory's implementation
	*/

	IResourceFactory* CreateVulkanShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	ITextureImpl* CreateVulkanTextureImpl(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params, E_RESULT_CODE& result);


	/*!
		class CVulkanTextureImpl
	*/

	class CVulkanTextureImpl : public virtual ITextureImpl, public CBaseObject
	{
		public:
			friend ITextureImpl* CreateVulkanTextureImpl(IGraphicsContext*, const TInitTextureImplParams&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an initial state of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params) override;

			E_RESULT_CODE Resize(U32 width, U32 height, U32 depth = 1) override;
			E_RESULT_CODE Transition(E_RESOURCE_LAYOUT newLayout) override;

			E_RESULT_CODE SetHandle(TTextureHandleId handle, const CPassKey<CBaseGraphicsObjectManager>& passkey) override;

			VkImage GetTextureHandle();
			VkImageView GetTextureViewHandle();

			std::vector<U8> ReadBytes(U32 index) override;

			E_RESOURCE_LAYOUT GetLayout() const override;

			const TInitTextureParams& GetParams() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanTextureImpl)

			E_RESULT_CODE _onInitInternal();
			E_RESULT_CODE _onFreeInternal() override;
		protected:
			CVulkanGraphicsContext* mpGraphicsContextImpl = nullptr;

			TInitTextureImplParams  mInitParams;
			TTextureHandleId        mHandle;

			VkImage                 mInternalImageHandle = VK_NULL_HANDLE;
			VkImageView             mInternalImageViewHandle = VK_NULL_HANDLE;

			VmaAllocator            mAllocator = VK_NULL_HANDLE;
			VmaAllocation           mAllocation = VK_NULL_HANDLE;

			VkDevice                mDevice = VK_NULL_HANDLE;

			E_RESOURCE_LAYOUT       mCurrLayout;

			TPtr<CVulkanBuffer>     mpReadbackBuffer = nullptr;
	};


	/*!
		\brief A factory function for creation objects of CVulkanVertexDeclaration's type

		\return A pointer to CVulkanVertexDeclaration's implementation
	*/

	IVertexDeclaration* CreateVulkanVertexDeclaration(E_RESULT_CODE& result);


	class CVulkanBasePipeline : public virtual IPipeline
	{
		public:
			virtual E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext);

			virtual VkPipelineLayout GetPipelineLayout() const;
			virtual const TVulkanPipelineLayoutInfo& GetLayoutInfo() const;

			virtual VkPipeline GetBasePipelineHandle() const;
			virtual U32 GetHash() const = 0;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanBasePipeline)
		protected:
			CVulkanGraphicsObjectManager* mpVulkanGraphicsObjectManagerImpl = nullptr;
			CVulkanGraphicsContext*       mpVulkanGraphicsContext = nullptr;

			VkPipeline                    mBasePipelineHandle = VK_NULL_HANDLE;
			VkPipelineLayout              mCachedPipelineLayoutHandle = VK_NULL_HANDLE;

			TVulkanPipelineLayoutInfo     mLayoutInfo{};
	};


	TDE2_API IGraphicsPipeline* CreateVulkanGraphicsPipeline(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, const TGraphicsPipelineConfigDesc& pipelineConfig, E_RESULT_CODE& result);
	TDE2_API IComputePipeline* CreateVulkanComputePipeline(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, const std::string& shaderId, E_RESULT_CODE& result);


	/*!
		\brief CVulkanGraphicsPipeline's definition
	*/

	class CVulkanGraphicsPipeline : public CBaseGraphicsPipeline, public CVulkanBasePipeline
	{
		public:
			TDE2_API friend IGraphicsPipeline* CreateVulkanGraphicsPipeline(IGraphicsContext*, IResourceManager*, const TGraphicsPipelineConfigDesc&, E_RESULT_CODE&);
		public:
			E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, const TGraphicsPipelineConfigDesc& pipelineConfig) override;
			E_RESULT_CODE Bind() override;

			VkPipeline GetPipelineForRenderPass(const TRenderPassInfo& renderPassInfo);

			U32 GetHash() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanGraphicsPipeline)
		private:
			VkGraphicsPipelineCreateInfo  mBasePipelineConfig{};
			U32                           mConfigHash = 0;
	};


	/*!
		\brief CVulkanComputePipeline's definition
	*/

	class CVulkanComputePipeline : public CBaseComputePipeline, public CVulkanBasePipeline
	{
		public:
			TDE2_API friend IComputePipeline* CreateVulkanComputePipeline(IGraphicsContext*, IResourceManager*, const std::string&, E_RESULT_CODE&);
		public:
			E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, const std::string& shaderId) override;
			E_RESULT_CODE Bind() override;

			U32 GetHash() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanComputePipeline)
		private:
			VkComputePipelineCreateInfo  mBasePipelineConfig{};
			U32                           mConfigHash = 0;
	};
}