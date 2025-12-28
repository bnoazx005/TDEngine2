#include "../../include/graphics/CBaseGraphicsObjectManager.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IFile.h"
#include "../../include/core/CProjectSettings.h"
#include "../../include/graphics/CDebugUtility.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/ITexture.h"
#include "../../include/graphics/CBaseShader.h"
#include "../../include/graphics/BasePipelines.h"
#include "../../include/graphics/IShaderCompiler.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/utils/CFileLogger.h"
#include <unordered_map>
#include <algorithm>


namespace TDEngine2
{
	void TDescriptorsBindingsTable::Reset()
	{
		std::fill(mConstantBuffers.begin(), mConstantBuffers.end(), TBufferHandleId::Invalid);
		std::fill(mSRVBuffers.begin(), mSRVBuffers.end(), TDescriptorHandle{});
		std::fill(mUAVBuffers.begin(), mUAVBuffers.end(), TDescriptorHandle{});
		std::fill(mSamplers.begin(), mSamplers.end(), TTextureSamplerId::Invalid);
	}


	TRenderPassInfo::TRenderPassInfo(IGraphicsObjectManager* pGraphicsObjectManager, const TFramebufferInfo& framebufferInfo)
	{
		TDE2_ASSERT(pGraphicsObjectManager);
		TDE2_ASSERT(framebufferInfo.mAttachments.size() < static_cast<USIZE>(RENDER_TARGETS_MAX_COUNT));

		std::fill(mRenderTargetFormats.begin(), mRenderTargetFormats.end(), E_FORMAT_TYPE::FT_UNKNOWN);

		for (USIZE i = 0; i < framebufferInfo.mAttachments.size(); ++i)
		{
			const TFramebufferInfo::TAttachment& currAttachment = framebufferInfo.mAttachments[i];

			TPtr<ITextureImpl> pRenderTargetTexture = pGraphicsObjectManager->GetTexturePtr(currAttachment.mTargetHandle);
			if (!pRenderTargetTexture)
			{
				continue;
			}

			mRenderTargetFormats[i] = pRenderTargetTexture->GetParams().mFormat;
		}

		if (framebufferInfo.mDepthStencilAttachment)
		{
			TPtr<ITextureImpl> pDepthBufferTexture = pGraphicsObjectManager->GetTexturePtr(framebufferInfo.mDepthStencilAttachment.value().mTargetHandle);
			if (pDepthBufferTexture)
			{
				mDepthStencilFormat = pDepthBufferTexture->GetParams().mFormat;
			}
		}
	}


	TFramebufferInfo::TDepthStencilAttachment::TDepthStencilAttachment(TTextureHandleId id):
		mTargetHandle(id)
	{
	}

	TFramebufferInfo::TDepthStencilAttachment::TDepthStencilAttachment(TTextureHandleId id, F32 depthValue):
		mTargetHandle(id), mDepthClearValue(depthValue)
	{
	}

	TFramebufferInfo::TDepthStencilAttachment::TDepthStencilAttachment(TTextureHandleId id, U8 stencilValue):
		mTargetHandle(id), mStencilClearValue(stencilValue)
	{
	}


	CBaseGraphicsObjectManager::CBaseGraphicsObjectManager() :
		CBaseObject(), mpDebugUtility(nullptr)
	{
	}

	E_RESULT_CODE CBaseGraphicsObjectManager::Init(IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsContext || !pFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = pGraphicsContext;
		mpFileSystem = pFileSystem;

		auto createShaderCacheResult = CreateShaderCache(mpFileSystem);
		if (createShaderCacheResult.HasError())
		{
			return createShaderCacheResult.GetError();
		}

		mpShaderCache = createShaderCacheResult.Get();


		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseGraphicsObjectManager::_onFreeInternal()
	{
		E_RESULT_CODE result = RC_OK;

		mpGraphicsPipelines.clear();
		mpComputePipelines.clear();

		result = result | _freeVertexDeclarations();
		result = result | _freeTextureSamplers();
		result = result | _freeBlendStates();
		result = result | _freeDepthStencilStates();
		result = result | _freeRasterizerStates();
		result = result | mpDebugUtility->Free();

		return result;
	}

	CPassKey<CBaseGraphicsObjectManager> CBaseGraphicsObjectManager::_getPassKey() const
	{
		return {};
	}

	TResult<IDebugUtility*> CBaseGraphicsObjectManager::CreateDebugUtility(IResourceManager* pResourceManager, IRenderer* pRenderer)
	{
		if (mpDebugUtility)
		{
			return Wrench::TOkValue<IDebugUtility*>(mpDebugUtility);
		}

		E_RESULT_CODE result = RC_OK;

		mpDebugUtility = TDEngine2::CreateDebugUtility(pResourceManager, pRenderer, this, result);

		if (result != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<IDebugUtility*>(mpDebugUtility);
	}

	TResult<TBufferHandleId> CBaseGraphicsObjectManager::CreateBuffer(const TInitBufferParams& params)
	{
		if (E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT == (params.mFlags & E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT))
		{			
			if (auto&& it = mTransientBuffersPool.find(ComputeStateDescHash(params)); it != mTransientBuffersPool.cend())
			{				
				if (auto& availableTransientBuffers = it->second; !availableTransientBuffers.empty())
				{
					const TBufferHandleId resourceId = availableTransientBuffers.back();
					availableTransientBuffers.pop_back();

					return TResult<TBufferHandleId>(resourceId);
				}
			}
		}

		TPtr<IBuffer> pBuffer = _createBufferInternal(params);
		if (!pBuffer)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		const USIZE placementIndex = _insertBuffer(pBuffer);

		pBuffer->SetHandle(static_cast<TBufferHandleId>(placementIndex), _getPassKey());

		return Wrench::TOkValue<TBufferHandleId>(static_cast<TBufferHandleId>(placementIndex));
	}

	TResult<TTextureHandleId> CBaseGraphicsObjectManager::CreateTexture(const TInitTextureImplParams& params)
	{
		if (E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT == (params.mFlags & E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT))
		{
			if (auto&& it = mTransientTexturesPool.find(ComputeStateDescHash(params)); it != mTransientTexturesPool.cend())
			{				
				if (auto& availableTransientTextures = it->second; !availableTransientTextures.empty())
				{
					const TTextureHandleId resourceId = availableTransientTextures.back();
					availableTransientTextures.pop_back();

					return TResult<TTextureHandleId>(resourceId);
				}
			}
		}

		TPtr<ITextureImpl> pTexture = _createTextureInternal(params);
		if (!pTexture)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		const USIZE placementIndex = _insertTexture(pTexture);
		pTexture->SetHandle(static_cast<TTextureHandleId>(placementIndex), _getPassKey());

		return Wrench::TOkValue<TTextureHandleId>(static_cast<TTextureHandleId>(placementIndex));
	}

	TResult<TGraphicsPipelineStateId> CBaseGraphicsObjectManager::CreateGraphicsPipelineState(const TGraphicsPipelineConfigDesc& pipelineConfigDesc)
	{
		const U32 hash = ComputeStateDescHash(pipelineConfigDesc);

		auto existingItemIt = mGraphicsPipelinesHashTable.find(hash);
		if (existingItemIt != mGraphicsPipelinesHashTable.cend())
		{
			return Wrench::TOkValue<TGraphicsPipelineStateId>(existingItemIt->second);
		}

		TPtr<IGraphicsPipeline> pGraphicsPipeline = _createGraphicsPipelineInternal(pipelineConfigDesc);
		if (!pGraphicsPipeline)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		auto it = std::find(mpGraphicsPipelines.begin(), mpGraphicsPipelines.end(), nullptr);
		const USIZE placementIndex = static_cast<USIZE>(std::distance(mpGraphicsPipelines.begin(), it));

		if (placementIndex >= mpGraphicsPipelines.size())
		{
			mpGraphicsPipelines.emplace_back(pGraphicsPipeline);
		}
		else
		{
			mpGraphicsPipelines[placementIndex] = pGraphicsPipeline;
		}

		mGraphicsPipelinesHashTable.emplace(hash, static_cast<TGraphicsPipelineStateId>(placementIndex));

		return Wrench::TOkValue<TGraphicsPipelineStateId>(static_cast<TGraphicsPipelineStateId>(placementIndex));
	}

	TResult<TComputePipelineStateId> CBaseGraphicsObjectManager::CreateComputePipelineState(const std::string& shaderId)
	{
		const U32 hash = TDE2_STRING_ID(shaderId.c_str());

		auto existingItemIt = mComputePipelinesHashTable.find(hash);
		if (existingItemIt != mComputePipelinesHashTable.cend())
		{
			return Wrench::TOkValue<TComputePipelineStateId>(existingItemIt->second);
		}

		TPtr<IComputePipeline> pComputePipeline = _createComputePipelineInternal(shaderId);
		if (!pComputePipeline)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		auto it = std::find(mpComputePipelines.begin(), mpComputePipelines.end(), nullptr);
		const USIZE placementIndex = static_cast<USIZE>(std::distance(mpComputePipelines.begin(), it));

		if (placementIndex >= mpComputePipelines.size())
		{
			mpComputePipelines.emplace_back(pComputePipeline);
		}
		else
		{
			mpComputePipelines[placementIndex] = pComputePipeline;
		}

		mComputePipelinesHashTable.emplace(hash, static_cast<TComputePipelineStateId>(placementIndex));

		return Wrench::TOkValue<TComputePipelineStateId>(static_cast<TComputePipelineStateId>(placementIndex));
	}


	static E_RESULT_CODE CompileShader(const IShaderCompiler* pShaderCompiler, const std::string& resourceName,
		IShaderImpl* pShader, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem)
	{
		TDE2_PROFILER_SCOPE("CBaseGraphicsObjectManager::CompileShader");

		/// load source code
		TResult<TFileEntryId> shaderFileId = pFileSystem->Open<ITextFileReader>(resourceName);

		auto loadDefaultShaderRoutine = [&resourceName, pGraphicsContext, pShaderCompiler, pShader]
			{
				LOG_WARNING(std::string("[Shader Loader] Could not load the specified shader (").append(resourceName).append("), load default one instead..."));

				/// \note can't load file with the shader, so load default one
				return pShader->Compile(pShaderCompiler, pGraphicsContext->GetGraphicsObjectManager()->GetDefaultShaderCode(CBaseGraphicsObjectManager::GetDefaultShaderTypeByName(resourceName)));
			};

		if (shaderFileId.HasError())
		{
			return loadDefaultShaderRoutine();
		}

		ITextFileReader* pShaderFileReader = pFileSystem->Get<ITextFileReader>(shaderFileId.Get());
		if (!pShaderFileReader)
		{
			return RC_FILE_NOT_FOUND;
		}

		const std::string& shaderSourceCode = pShaderFileReader->ReadToEnd();

		E_RESULT_CODE result = RC_OK;

		if ((result = pShaderFileReader->Close()) != RC_OK)
		{
			return result;
		}

		/// parse it and compile needed variant
		if ((result = pShader->Compile(pShaderCompiler, shaderSourceCode)) != RC_OK)
		{
			return loadDefaultShaderRoutine();
		}

		return result;
	}


	static E_RESULT_CODE TryToCompileShader(IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, const IShaderCompiler* pShaderCompiler, IShaderCache* pShaderCache,
		IShaderImpl* pShader, const std::string& shaderId)
	{
		E_RESULT_CODE result = RC_OK;

		/// \note If there is meta information within the manifest try to read precompiled shader first		
		if (!pShaderCache->HasShaderMetaData(shaderId) || !CProjectSettings::Get()->mGraphicsSettings.mIsShaderCacheEnabled)
		{
			return CompileShader(pShaderCompiler, shaderId, pShader, pGraphicsContext, pFileSystem);
		}

		if (RC_OK != (result = pShader->LoadFromShaderCache(pShaderCache)))
		{
			// if we failed in loading of precompiled shader try to compile it in runtime
			return CompileShader(pShaderCompiler, shaderId, pShader, pGraphicsContext, pFileSystem);
		}

		return RC_OK;
	}


	TResult<TShaderHandleId> CBaseGraphicsObjectManager::LoadShader(const std::string& shaderId)
	{
		TDE2_PROFILER_SCOPE("CBaseGraphicsObjectManager::LoadShader");

		const U32 hash = TDE2_STRING_ID(shaderId.c_str());

		auto existingItemIt = mLoadedShadersTable.find(hash);
		if (existingItemIt != mLoadedShadersTable.cend())
		{
			return Wrench::TOkValue<TShaderHandleId>(existingItemIt->second);
		}

		TPtr<IShaderImpl> pShader = _createShaderImplInternal(shaderId);
		if (!pShader)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (!mpShaderCompiler)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (E_RESULT_CODE result = TryToCompileShader(mpGraphicsContext, mpFileSystem, mpShaderCompiler.Get(), mpShaderCache.Get(), pShader.Get(), shaderId); RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		const USIZE placementIndex = _insertShaderImpl(pShader);

		mLoadedShadersTable.emplace(hash, static_cast<TShaderHandleId>(placementIndex));

		return Wrench::TOkValue<TShaderHandleId>(static_cast<TShaderHandleId>(placementIndex));
	}

	TResult<TPtr<IShaderCache>> CBaseGraphicsObjectManager::CreateShaderCache(bool isReadOnly)
	{
		E_RESULT_CODE result = RC_OK;

		TPtr<IShaderCache> pShaderCache = TPtr<IShaderCache>(
			::TDEngine2::CreateShaderCache(
				mpFileSystem->Get<IBinaryFileReader>(mpFileSystem->Open<IBinaryFileReader>(_getShaderCacheFilePath(), true).Get()),
				isReadOnly ? nullptr : mpFileSystem->Get<IBinaryFileWriter>(mpFileSystem->Open<IBinaryFileWriter>(_getShaderCacheFilePath(), true).Get()), result));

		if (!pShaderCache || RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<TPtr<IShaderCache>>(pShaderCache);
	}

	E_RESULT_CODE CBaseGraphicsObjectManager::SetShaderCompiler(TPtr<IShaderCompiler> pShaderCompiler)
	{
		if (!pShaderCompiler)
		{
			return RC_INVALID_ARGS;
		}

		mpShaderCompiler = pShaderCompiler;

		return RC_OK;
	}

	IGraphicsContext* CBaseGraphicsObjectManager::GetGraphicsContext() const
	{
		return mpGraphicsContext;
	}

	TPtr<IGraphicsPipeline> CBaseGraphicsObjectManager::GetGraphicsPipeline(TGraphicsPipelineStateId handle)
	{
		if (TGraphicsPipelineStateId::Invalid == handle)
		{
			return nullptr;
		}

		const USIZE placementIndex = static_cast<USIZE>(handle);
		if (placementIndex >= mpGraphicsPipelines.size())
		{
			return nullptr;
		}

		return mpGraphicsPipelines[placementIndex];
	}

	TPtr<IComputePipeline> CBaseGraphicsObjectManager::GetComputePipeline(TComputePipelineStateId handle)
	{
		if (TComputePipelineStateId::Invalid == handle)
		{
			return nullptr;
		}

		const USIZE placementIndex = static_cast<USIZE>(handle);
		if (placementIndex >= mpComputePipelines.size())
		{
			return nullptr;
		}

		return mpComputePipelines[placementIndex];
	}

	E_DEFAULT_SHADER_TYPE CBaseGraphicsObjectManager::GetDefaultShaderTypeByName(const std::string& name)
	{
		return E_DEFAULT_SHADER_TYPE::DST_BASIC;
	}

	TTextureSamplerId CBaseGraphicsObjectManager::GetDefaultTextureSampler(E_TEXTURE_FILTER_TYPE filterType)
	{
		auto textureSamplerResult = CreateTextureSampler({ filterType, E_ADDRESS_MODE_TYPE::AMT_CLAMP, E_ADDRESS_MODE_TYPE::AMT_CLAMP, E_ADDRESS_MODE_TYPE::AMT_CLAMP, false });
		if (textureSamplerResult.HasError())
		{
			return TTextureSamplerId::Invalid;
		}

		return textureSamplerResult.Get();
	}

	IVertexDeclaration* CBaseGraphicsObjectManager::GetDefaultPositionOnlyVertexDeclaration()
	{
		if (!mpDefaultPositionOnlyVertDeclaration)
		{
			mpDefaultPositionOnlyVertDeclaration = CreateVertexDeclaration().Get();
			mpDefaultPositionOnlyVertDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });
		}

		return mpDefaultPositionOnlyVertDeclaration;
	}

	TPtr<IGraphicsPipeline> CBaseGraphicsObjectManager::_createGraphicsPipelineInternal(const TGraphicsPipelineConfigDesc& pipelineConfigDesc)
	{
		E_RESULT_CODE result = RC_OK;
		return TPtr<IGraphicsPipeline>(CreateBaseGraphicsPipeline(mpGraphicsContext, pipelineConfigDesc, result));
	}

	TPtr<IComputePipeline> CBaseGraphicsObjectManager::_createComputePipelineInternal(const std::string& shaderId)
	{
		E_RESULT_CODE result = RC_OK;
		return TPtr<IComputePipeline>(CreateBaseComputePipeline(mpGraphicsContext, shaderId, result));
	}

	void CBaseGraphicsObjectManager::_insertVertexDeclaration(IVertexDeclaration* pVertDecl)
	{
		U32 index = 0;

		if (mFreeVertDeclsSlots.empty())
		{
			index = static_cast<U32>(mVertexDeclarationsArray.size());

			mVertexDeclarationsArray.push_back(pVertDecl);

			return;
		}

		index = mFreeVertDeclsSlots.front();

		mFreeVertDeclsSlots.pop_front();

		mVertexDeclarationsArray[index] = pVertDecl;

		return;
	}

	E_RESULT_CODE CBaseGraphicsObjectManager::_freeVertexDeclarations()
	{
		IVertexDeclaration* pCurrVertDecl = nullptr;

		E_RESULT_CODE result = RC_OK;

		for (auto iter = mVertexDeclarationsArray.begin(); iter != mVertexDeclarationsArray.end(); ++iter)
		{
			pCurrVertDecl = (*iter);

			if (!pCurrVertDecl)
			{
				continue;
			}

			result = result | pCurrVertDecl->Free();
		}

		mVertexDeclarationsArray.clear();
		mFreeVertDeclsSlots.clear();

		return result;
	}
}