#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/IBuffer.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IPipeline.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IResource.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/IShader.h"
#include "../../include/graphics/IGlobalShaderProperties.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CStatsCounters.h"
#include <algorithm>
#include <stringUtils.hpp>


namespace TDEngine2
{
	static const std::string InvalidMaterialMessage = "{0} Invalid material was passed into the render command";


	constexpr U32 DEFAULT_PVP_INDEX_BUFFER_SLOT     = 30;
	constexpr U32 DEFAULT_PVP_INSTANCE_BUFFER_SLOT  = 31;
	constexpr U32 DEFAULT_PVP_VERTEX_BUFFER_SLOT    = 32;


	E_RESULT_CODE TUpdateBufferCommand::Submit(const TRenderCommandSubmitParams& params)
	{
		IGraphicsObjectManager* pGraphicsObjectManager = params.mpGraphicsContext->GetGraphicsObjectManager();
		
		TPtr<IBuffer> pBuffer = pGraphicsObjectManager->GetBufferPtr(mBufferHandle);
		if (!pBuffer)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = pBuffer->Map(mMapType, mMapOffset);
		if (RC_OK != result)
		{
			return result;
		}

		result = pBuffer->Write(mpData, mDataSize);
		if (RC_OK != result)
		{
			return result;
		}

		pBuffer->Unmap();

		return result;
	}


	static inline bool TryToBindMaterialProxy(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IMaterialProxyProvider* pMaterialsProxyProvider,
										TMaterialProxyId materialProxyId, const TRectU32 scissorRect)
	{
		const TMaterialRenderProxy* pMaterialRenderProxy = pMaterialsProxyProvider->GetProxy(materialProxyId);
		if (!pMaterialRenderProxy)
		{
			return false;
		}

		auto pShaderInstance = pResourceManager->GetResource<IShader>(pMaterialRenderProxy->mShaderHandle);
		TDE2_ASSERT(pShaderInstance);

		U8 userUniformBufferId = 0;

		for (const auto& currUserDataBuffer : pMaterialRenderProxy->mUserUniformBuffers)
		{
			if (!currUserDataBuffer.size())
			{
				continue;
			}

			PANIC_ON_FAILURE(pShaderInstance->SetUserUniformsBuffer(userUniformBufferId++, &currUserDataBuffer.front(), currUserDataBuffer.size()));
		}

		for (const auto& [resourceName, pTextureInstance] : pMaterialRenderProxy->mTextures)
		{
			pShaderInstance->SetTextureResource(resourceName, pTextureInstance);
		}

		if (auto pGraphicsPipeline = pGraphicsContext->GetGraphicsObjectManager()->GetGraphicsPipeline(pMaterialRenderProxy->mPipelineHandle))
		{
			pGraphicsPipeline->Bind();
		}

		if (pMaterialRenderProxy->mIsScissorTestEnabled)
		{
			pGraphicsContext->SetScissorRect(scissorRect);
		}

		return true;
	}


	static inline bool TryToBindMaterial(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, TResourceId materialHandle, 
		TMaterialInstanceId materialInstanceId, const TRectU32 scissorRect)
	{
		TPtr<IMaterial> pMaterial = pResourceManager->GetResource<IMaterial>(materialHandle);
		if (!pMaterial)
		{
			return false;
		}
		
		pMaterial->Bind(materialInstanceId);

		if (pMaterial->IsScissorTestEnabled())
		{
			pGraphicsContext->SetScissorRect(scissorRect);
		}

		return true;
	}


	E_RESULT_CODE TDrawCommand::Submit(const TRenderCommandSubmitParams& params)
	{
		if (TResourceId::Invalid == mMaterialHandle)
		{
			LOG_ERROR(Wrench::StringUtils::Format(InvalidMaterialMessage, "[TDrawCommand]"));
			return RC_INVALID_ARGS;
		}

		TDE2_STATS_COUNTER_INCREMENT(mDrawCallsCount);

		IGraphicsContext* pGraphicsContext = params.mpGraphicsContext;
		IResourceManager* pResourceManager = params.mpResourceManager;
		IGlobalShaderProperties* pGlobalShaderProperties = params.mpGlobalShaderProperties;
		IMaterialProxyProvider* pMaterialsProxyProvider = params.mpMaterialProxiesProvider;

		TDE2_ASSERT(pMaterialsProxyProvider);

		if (!TryToBindMaterialProxy(pResourceManager, pGraphicsContext, pMaterialsProxyProvider, mMaterialProxyHandle, mScissorRect)) // preferred way of working with materials
		{
			TryToBindMaterial(pResourceManager, pGraphicsContext, mMaterialHandle, mMaterialInstanceId, mScissorRect); /// \note Now is used for backward compatibility prefer to use material proxies
		}

		pGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_PER_OBJECT, reinterpret_cast<const U8*>(&mObjectData), sizeof(mObjectData));

		pGraphicsContext->SetStructuredBuffer(DEFAULT_PVP_VERTEX_BUFFER_SLOT, mVertexBufferHandle);

		for (U32 i = 0; i < ADDITIONAL_VERTEX_BUFFERS_MAX_COUNT; ++i)
		{
			pGraphicsContext->SetStructuredBuffer(DEFAULT_PVP_VERTEX_BUFFER_SLOT + i + 1, mAdditionalVertexBuffers[i]);
		}

		pGraphicsContext->Draw(mPrimitiveType, 0, mNumOfVertices);

		return RC_OK;
	}


	E_RESULT_CODE TDrawIndexedCommand::Submit(const TRenderCommandSubmitParams& params)
	{
		if (TResourceId::Invalid == mMaterialHandle)
		{
			LOG_ERROR(Wrench::StringUtils::Format(InvalidMaterialMessage, "[TDrawIndexedCommand]"));
			return RC_INVALID_ARGS;
		}

		IGraphicsContext* pGraphicsContext = params.mpGraphicsContext;
		IResourceManager* pResourceManager = params.mpResourceManager;
		IGlobalShaderProperties* pGlobalShaderProperties = params.mpGlobalShaderProperties;
		IMaterialProxyProvider* pMaterialsProxyProvider = params.mpMaterialProxiesProvider;

		TDE2_ASSERT(pMaterialsProxyProvider);

		if (!TryToBindMaterialProxy(pResourceManager, pGraphicsContext, pMaterialsProxyProvider, mMaterialProxyHandle, mScissorRect)) // preferred way of working with materials
		{
			TryToBindMaterial(pResourceManager, pGraphicsContext, mMaterialHandle, mMaterialInstanceId, mScissorRect); /// \note Now is used for backward compatibility prefer to use material proxies
		}

		TDE2_STATS_COUNTER_INCREMENT(mDrawCallsCount);

		pGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_PER_OBJECT, reinterpret_cast<const U8*>(&mObjectData), sizeof(mObjectData));
		
		pGraphicsContext->SetStructuredBuffer(DEFAULT_PVP_VERTEX_BUFFER_SLOT, mVertexBufferHandle);
		pGraphicsContext->SetStructuredBuffer(DEFAULT_PVP_INDEX_BUFFER_SLOT, mIndexBufferHandle);

		for (U32 i = 0; i < ADDITIONAL_VERTEX_BUFFERS_MAX_COUNT; ++i)
		{
			pGraphicsContext->SetStructuredBuffer(DEFAULT_PVP_VERTEX_BUFFER_SLOT + i + 1, mAdditionalVertexBuffers[i]);
		}

		pGraphicsContext->Draw(mPrimitiveType, 0, mNumOfIndices);

		return RC_OK;
	}

	E_RESULT_CODE TDrawInstancedCommand::Submit(const TRenderCommandSubmitParams& params)
	{
		TDE2_STATS_COUNTER_INCREMENT(mDrawCallsCount);
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE TDrawIndexedInstancedCommand::Submit(const TRenderCommandSubmitParams& params)
	{
		if (TResourceId::Invalid == mMaterialHandle)
		{
			LOG_ERROR(Wrench::StringUtils::Format(InvalidMaterialMessage, "[TDrawIndexedInstancedCommand]"));
			return RC_INVALID_ARGS;
		}

		TDE2_STATS_COUNTER_INCREMENT(mDrawCallsCount);

		IGraphicsContext* pGraphicsContext = params.mpGraphicsContext;
		IResourceManager* pResourceManager = params.mpResourceManager;
		IGlobalShaderProperties* pGlobalShaderProperties = params.mpGlobalShaderProperties;
		IMaterialProxyProvider* pMaterialsProxyProvider = params.mpMaterialProxiesProvider;

		TDE2_ASSERT(pMaterialsProxyProvider);

		if (!TryToBindMaterialProxy(pResourceManager, pGraphicsContext, pMaterialsProxyProvider, mMaterialProxyHandle, mScissorRect)) // preferred way of working with materials
		{
			TryToBindMaterial(pResourceManager, pGraphicsContext, mMaterialHandle, mMaterialInstanceId, mScissorRect); /// \note Now is used for backward compatibility prefer to use material proxies
		}

		pGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_PER_OBJECT, reinterpret_cast<const U8*>(&mObjectData), sizeof(mObjectData));

		pGraphicsContext->SetStructuredBuffer(DEFAULT_PVP_VERTEX_BUFFER_SLOT, mVertexBufferHandle);
		pGraphicsContext->SetStructuredBuffer(DEFAULT_PVP_INDEX_BUFFER_SLOT, mIndexBufferHandle);
		pGraphicsContext->SetStructuredBuffer(DEFAULT_PVP_INSTANCE_BUFFER_SLOT, mInstancingBufferHandle);

		for (U32 i = 0; i < ADDITIONAL_VERTEX_BUFFERS_MAX_COUNT; ++i)
		{
			pGraphicsContext->SetStructuredBuffer(DEFAULT_PVP_VERTEX_BUFFER_SLOT + i + 1, mAdditionalVertexBuffers[i]);
		}

		pGraphicsContext->DrawInstanced(mPrimitiveType, 0, mIndicesPerInstance, 0, mNumOfInstances);

		return RC_OK;
	}

	E_RESULT_CODE TDrawIndirectInstancedCommand::Submit(const TRenderCommandSubmitParams& params)
	{
		if (TResourceId::Invalid == mMaterialHandle)
		{
			LOG_ERROR(Wrench::StringUtils::Format(InvalidMaterialMessage, "[TDrawIndirectInstancedCommand]"));
			return RC_INVALID_ARGS;
		}

		TDE2_STATS_COUNTER_INCREMENT(mDrawCallsCount);

		IGraphicsContext* pGraphicsContext = params.mpGraphicsContext;
		IResourceManager* pResourceManager = params.mpResourceManager;
		IGlobalShaderProperties* pGlobalShaderProperties = params.mpGlobalShaderProperties;
		IMaterialProxyProvider* pMaterialsProxyProvider = params.mpMaterialProxiesProvider;

		TDE2_ASSERT(pMaterialsProxyProvider);

		if (!TryToBindMaterialProxy(pResourceManager, pGraphicsContext, pMaterialsProxyProvider, mMaterialProxyHandle, mScissorRect)) // preferred way of working with materials
		{
			TryToBindMaterial(pResourceManager, pGraphicsContext, mMaterialHandle, mMaterialInstanceId, mScissorRect); /// \note Now is used for backward compatibility prefer to use material proxies
		}

		pGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_PER_OBJECT, reinterpret_cast<const U8*>(&mObjectData), sizeof(mObjectData));

		pGraphicsContext->SetStructuredBuffer(DEFAULT_PVP_VERTEX_BUFFER_SLOT, mVertexBufferHandle);
		pGraphicsContext->SetStructuredBuffer(DEFAULT_PVP_INDEX_BUFFER_SLOT, mIndexBufferHandle);

		for (U32 i = 0; i < ADDITIONAL_VERTEX_BUFFERS_MAX_COUNT; ++i)
		{
			pGraphicsContext->SetStructuredBuffer(DEFAULT_PVP_VERTEX_BUFFER_SLOT + i + 1, mAdditionalVertexBuffers[i]);
		}

		pGraphicsContext->DrawIndirectInstanced(mPrimitiveType, mArgsBufferHandle, mAlignedOffset);

		return RC_OK;
	}


	CRenderQueue::CRenderQueueIterator::CRenderQueueIterator(TCommandsArray& commandsBuffer, U32 initialIndex) :
		mpTargetCollection(&commandsBuffer), mCurrCommandIndex(initialIndex)
	{
	}

	CRenderQueue::CRenderQueueIterator::CRenderQueueIterator(const CRenderQueueIterator& iter) :
		mpTargetCollection(iter.mpTargetCollection), mCurrCommandIndex(iter.mCurrCommandIndex)
	{
	}

	CRenderQueue::CRenderQueueIterator::CRenderQueueIterator(CRenderQueueIterator&& iter):
		mpTargetCollection(iter.mpTargetCollection), mCurrCommandIndex(iter.mCurrCommandIndex)
	{
		iter.mpTargetCollection = nullptr;
		iter.mCurrCommandIndex  = 0;
	}

	TRenderCommand* CRenderQueue::CRenderQueueIterator::GetNext()
	{
		return std::get<TRenderCommand*>((*mpTargetCollection)[++mCurrCommandIndex]);
	}

	bool CRenderQueue::CRenderQueueIterator::HasNext() const
	{
		return (mCurrCommandIndex + 1) <= mpTargetCollection->size();
	}

	void CRenderQueue::CRenderQueueIterator::Reset()
	{
		mCurrCommandIndex = 0;
	}

	TRenderCommand* CRenderQueue::CRenderQueueIterator::Get() const
	{
		return std::get<TRenderCommand*>((*mpTargetCollection)[mCurrCommandIndex]);
	}

	U32 CRenderQueue::CRenderQueueIterator::GetIndex() const
	{
		return mCurrCommandIndex;
	}
	
	CRenderQueue::CRenderQueueIterator& CRenderQueue::CRenderQueueIterator::operator++()
	{
		++mCurrCommandIndex;

		return *this;
	}

	CRenderQueue::CRenderQueueIterator CRenderQueue::CRenderQueueIterator::operator++(int)
	{
		CRenderQueueIterator oldIter(*this);

		++mCurrCommandIndex;

		return oldIter;
	}

	TRenderCommand* CRenderQueue::CRenderQueueIterator::operator*() const
	{
		return std::get<TRenderCommand*>((*mpTargetCollection)[mCurrCommandIndex]);
	}


	CRenderQueue::CRenderQueue():
		CBaseObject()
	{
	}

	E_RESULT_CODE CRenderQueue::Init(IAllocator* pTempAllocator)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pTempAllocator)
		{
			return RC_INVALID_ARGS;
		}

		mpTempAllocator = pTempAllocator;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CRenderQueue::Clear()
	{
		mCommandsBuffer.clear();

		return mpTempAllocator->Clear();
	}

	void CRenderQueue::Sort()
	{
		std::sort(mCommandsBuffer.begin(), mCommandsBuffer.end(), [](const std::tuple<U32, TRenderCommand*>& left, const std::tuple<U32, TRenderCommand*>& right)
		{
			return std::get<U32>(left) > std::get<U32>(right);
		});
	}

	bool CRenderQueue::IsEmpty() const
	{
		return mCommandsBuffer.empty();
	}

	E_RESULT_CODE CRenderQueue::_onFreeInternal()
	{
		return mpTempAllocator->Free();
	}

	CRenderQueue::CRenderQueueIterator CRenderQueue::GetIterator()
	{
		return CRenderQueueIterator(mCommandsBuffer);
	}


	TDE2_API CRenderQueue* CreateRenderQueue(IAllocator* pTempAllocator, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CRenderQueue, CRenderQueue, result, pTempAllocator);
	}
}