#include "../../include/graphics/CBaseGraphicsObjectManager.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IFile.h"
#include "../../include/graphics/CDebugUtility.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/CBaseShader.h"
#include "../../include/graphics/CBaseGraphicsPipeline.h"
#include <unordered_map>
#include <algorithm>


namespace TDEngine2
{
	CBaseGraphicsObjectManager::CBaseGraphicsObjectManager() :
		CBaseObject(), mpDebugUtility(nullptr), mpGraphicsContext(nullptr)
	{
	}

	E_RESULT_CODE CBaseGraphicsObjectManager::Init(IGraphicsContext* pGraphicsContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsContext)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseGraphicsObjectManager::_onFreeInternal()
	{
		E_RESULT_CODE result = RC_OK;

		mpGraphicsPipelines.clear();

		result = result | _freeVertexDeclarations();
		result = result | _freeTextureSamplers();
		result = result | _freeBlendStates();
		result = result | _freeDepthStencilStates();
		result = result | _freeRasterizerStates();
		result = result | mpDebugUtility->Free();

		return result;
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

	TResult<TGraphicsPipelineStateId> CBaseGraphicsObjectManager::CreateGraphicsPipelineState(const TGraphicsPipelineConfigDesc& pipelineConfigDesc)
	{
		E_RESULT_CODE result = RC_OK;

		const U32 hash = ComputeStateDescHash(pipelineConfigDesc);

		auto existingItemIt = mGraphicsPipelinesHashTable.find(hash);
		if (existingItemIt != mGraphicsPipelinesHashTable.cend())
		{
			return Wrench::TOkValue<TGraphicsPipelineStateId>(existingItemIt->second);
		}

		TPtr<IGraphicsPipeline> pGraphicsPipeline = TPtr<IGraphicsPipeline>(CreateBaseGraphicsPipeline(mpGraphicsContext, pipelineConfigDesc, result));
		if (!pGraphicsPipeline || RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
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

	TResult<TPtr<IShaderCache>> CBaseGraphicsObjectManager::CreateShaderCache(IFileSystem* pFileSystem, bool isReadOnly)
	{
		E_RESULT_CODE result = RC_OK;

		TPtr<IShaderCache> pShaderCache = TPtr<IShaderCache>(
			::TDEngine2::CreateShaderCache(
				pFileSystem->Get<IBinaryFileReader>(pFileSystem->Open<IBinaryFileReader>(_getShaderCacheFilePath(), true).Get()),
				isReadOnly ? nullptr : pFileSystem->Get<IBinaryFileWriter>(pFileSystem->Open<IBinaryFileWriter>(_getShaderCacheFilePath(), true).Get()), result));

		if (!pShaderCache || RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<TPtr<IShaderCache>>(pShaderCache);
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

	E_DEFAULT_SHADER_TYPE CBaseGraphicsObjectManager::GetDefaultShaderTypeByName(const std::string& name)
	{
		return E_DEFAULT_SHADER_TYPE::DST_BASIC;
	}

	TTextureSamplerId CBaseGraphicsObjectManager::GetDefaultTextureSampler(E_TEXTURE_FILTER_TYPE filterType)
	{
		auto textureSamplerResult = CreateTextureSampler({ filterType, E_ADDRESS_MODE_TYPE::AMT_CLAMP, E_ADDRESS_MODE_TYPE::AMT_CLAMP, E_ADDRESS_MODE_TYPE::AMT_CLAMP, true });
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