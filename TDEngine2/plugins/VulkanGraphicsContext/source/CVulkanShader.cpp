#include "../include/CVulkanShader.h"
#include "../include/CVulkanGraphicsContext.h"
#include "../include/CVulkanMappings.h"


namespace TDEngine2
{
	CVulkanShader::CVulkanShader() :
		CBaseShader()
	{
	}
	
	E_RESULT_CODE CVulkanShader::Reset()
	{
		mIsInitialized = false;

		E_RESULT_CODE result = RC_OK;

		for (auto& currShaderModule : mShaderStageModules)
		{
			vkDestroyShaderModule(mDevice, currShaderModule, nullptr);
		}

		return RC_OK;
	}

	void CVulkanShader::Bind()
	{
		CBaseShader::Bind();

	}

	void CVulkanShader::Unbind()
	{
	}

	VkPipelineShaderStageCreateInfo CVulkanShader::GetPipelineShaderStage(E_SHADER_STAGE_TYPE stageType) const
	{
		return mPipelineShaderStagesInfo[stageType];
	}


	static TResult<VkShaderModule> CreateShaderModule(VkDevice device, E_SHADER_STAGE_TYPE stageType, const TShaderCompilerOutput* pCompilerData)
	{
		VkShaderModule shaderModule = VK_NULL_HANDLE;

		auto it = pCompilerData->mStagesInfo.find(stageType);
		if (it == pCompilerData->mStagesInfo.end())
		{
			return Wrench::TOkValue<VkShaderModule>(shaderModule);
		}

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = it->second.mBytecode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(it->second.mBytecode.data());

		VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
		if (VK_SUCCESS != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(CVulkanMappings::GetErrorCode(result));
		}

		return Wrench::TOkValue<VkShaderModule>(shaderModule);
	}


	static VkPipelineShaderStageCreateInfo CreatePipelineShaderStageInfo(VkDevice device, E_SHADER_STAGE_TYPE stageType, VkShaderModule shaderModule, const TShaderCompilerOutput* pCompilerData)
	{
		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = CVulkanMappings::GetShaderStageType(stageType);
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = pCompilerData->mStagesInfo.at(stageType).mEntrypointName.c_str();

		return shaderStageInfo;
	}


	E_RESULT_CODE CVulkanShader::_createInternalHandlers(const TShaderCompilerOutput* pCompilerData)
	{
		if (!pCompilerData)
		{
			return RC_INVALID_ARGS;
		}

		CVulkanGraphicsContext* pVulkanImplContext = dynamic_cast<CVulkanGraphicsContext*>(mpGraphicsContext);
		TDE2_ASSERT(pVulkanImplContext);

		mDevice = pVulkanImplContext->GetDevice();

		for (U32 stageIndex = SST_VERTEX; stageIndex < SST_NONE; stageIndex++)
		{
			auto creationResult = CreateShaderModule(mDevice, static_cast<E_SHADER_STAGE_TYPE>(stageIndex), pCompilerData);
			if (creationResult.HasError())
			{
				return creationResult.GetError();
			}

			mShaderStageModules[stageIndex] = creationResult.Get();

			if (VK_NULL_HANDLE == mShaderStageModules[stageIndex])
			{
				continue;
			}

			mPipelineShaderStagesInfo[stageIndex] = CreatePipelineShaderStageInfo(mDevice, static_cast<E_SHADER_STAGE_TYPE>(stageIndex), mShaderStageModules[stageIndex], pCompilerData);
		}

		return _createUniformBuffers(pCompilerData);
	}

	E_RESULT_CODE CVulkanShader::_createUniformBuffers(const TShaderCompilerOutput* pCompilerData)
	{
		//auto uniformBuffersInfo = pCompilerData->mUniformBuffersInfo;

		//TUniformBufferDesc currDesc;

		//E_RESULT_CODE result = RC_OK;

		//mUniformBuffers.resize(uniformBuffersInfo.size() - TotalNumberOfInternalConstantBuffers);

		//IConstantBuffer* pConstantBuffer = nullptr;

		///// here only user uniforms buffers are created
		//for (auto iter = uniformBuffersInfo.cbegin(); iter != uniformBuffersInfo.cend(); ++iter)
		//{
		//	currDesc = (*iter).second;

		//	/// skip internal buffers, because they are created separately by IGlobalShaderProperties implementation
		//	if ((currDesc.mFlags & E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL) == E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL)
		//	{
		//		continue;
		//	}

		//	pConstantBuffer = CreateVulkanConstantBuffer(mpGraphicsContext, BUT_DYNAMIC, currDesc.mSize, nullptr, result);

		//	const U32 index = static_cast<U32>(iter->second.mSlot - TotalNumberOfInternalConstantBuffers);
		//	TDE2_ASSERT(index >= 0 && index <= 1024);

		//	/// \note Ensure that we compute correct size of the constant buffer. We use IVulkanShaderReflection to retrieve accurate information


		//	mUniformBuffers[index] = pConstantBuffer; // the offset is used because the shaders doesn't store internal buffer by themselves
		//}

		return RC_OK;
	}


	TDE2_API IShader* CreateVulkanShader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IShader, CVulkanShader, result, pResourceManager, pGraphicsContext, name);
	}


	CVulkanShaderFactory::CVulkanShaderFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVulkanShaderFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsContext || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;
		
		return RC_OK;
	}

	IResource* CVulkanShaderFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		return nullptr;
	}

	IResource* CVulkanShaderFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateVulkanShader(mpResourceManager, mpGraphicsContext, name, result));
	}

	TypeId CVulkanShaderFactory::GetResourceTypeId() const
	{
		return IShader::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateVulkanShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CVulkanShaderFactory, result, pResourceManager, pGraphicsContext);
	}
}