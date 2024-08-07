#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/IBuffer.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
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

		auto pGraphicsObjectManager = pGraphicsContext->GetGraphicsObjectManager();

		auto pMaterial = pResourceManager->GetResource<IMaterial>(mMaterialHandle);

		auto pAttachedShader = pResourceManager->GetResource<IShader>(pMaterial->GetShaderHandle());

		mpVertexDeclaration->Bind(pGraphicsContext, { mVertexBufferHandle }, pAttachedShader.Get());

		pMaterial->Bind(mMaterialInstanceId);

		if (pMaterial->IsScissorTestEnabled())
		{
			pGraphicsContext->SetScissorRect(mScissorRect);
		}

		pGraphicsContext->SetVertexBuffer(0, mVertexBufferHandle, 0, mpVertexDeclaration->GetStrideSize(0)); /// \todo replace magic constants

		pGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_PER_OBJECT, reinterpret_cast<const U8*>(&mObjectData), sizeof(mObjectData));

		pGraphicsContext->Draw(mPrimitiveType, mStartVertex, mNumOfVertices);

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

		auto pMaterial = pResourceManager->GetResource<IMaterial>(mMaterialHandle);
		if (!pMaterial)
		{
			TDE2_ASSERT(false);
			return RC_FAIL;
		}

		auto pAttachedShader = pResourceManager->GetResource<IShader>(pMaterial->GetShaderHandle());
		if (!pAttachedShader)
		{
			TDE2_ASSERT(false);
			return RC_FAIL;
		}

		TDE2_STATS_COUNTER_INCREMENT(mDrawCallsCount);

		mpVertexDeclaration->Bind(pGraphicsContext, { mVertexBufferHandle }, pAttachedShader.Get());

		pMaterial->Bind(mMaterialInstanceId);

		if (pMaterial->IsScissorTestEnabled())
		{
			pGraphicsContext->SetScissorRect(mScissorRect);
		}

		pGraphicsContext->SetVertexBuffer(0, mVertexBufferHandle, 0, mpVertexDeclaration->GetStrideSize(0)); /// \todo replace magic constants
		pGraphicsContext->SetIndexBuffer(mIndexBufferHandle, 0);

		pGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_PER_OBJECT, reinterpret_cast<const U8*>(&mObjectData), sizeof(mObjectData));
		
		auto pIndexBuffer = pGraphicsContext->GetGraphicsObjectManager()->GetBufferPtr(mIndexBufferHandle);

		pGraphicsContext->DrawIndexed(mPrimitiveType, pIndexBuffer->GetParams().mIndexFormat, mStartVertex, mStartIndex, mNumOfIndices);

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

		auto pMaterial = pResourceManager->GetResource<IMaterial>(mMaterialHandle);

		auto pAttachedShader = pResourceManager->GetResource<IShader>(pMaterial->GetShaderHandle());

		mpVertexDeclaration->Bind(pGraphicsContext, { mVertexBufferHandle, mInstancingBufferHandle }, pAttachedShader.Get());

		pMaterial->Bind(mMaterialInstanceId);

		if (pMaterial->IsScissorTestEnabled())
		{
			pGraphicsContext->SetScissorRect(mScissorRect);
		}

		pGraphicsContext->SetVertexBuffer(0, mVertexBufferHandle, 0, mpVertexDeclaration->GetStrideSize(0)); /// \todo replace magic constants
		pGraphicsContext->SetVertexBuffer(1, mInstancingBufferHandle, 0, mpVertexDeclaration->GetStrideSize(1)); /// \todo replace magic constants
		pGraphicsContext->SetIndexBuffer(mIndexBufferHandle, 0);

		pGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_PER_OBJECT, reinterpret_cast<const U8*>(&mObjectData), sizeof(mObjectData));

		auto pIndexBuffer = pGraphicsContext->GetGraphicsObjectManager()->GetBufferPtr(mIndexBufferHandle);

		pGraphicsContext->DrawIndexedInstanced(mPrimitiveType, pIndexBuffer->GetParams().mIndexFormat, mBaseVertexIndex, mStartIndex,
											   mStartInstance, mIndicesPerInstance, mNumOfInstances);

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

		auto pMaterial = pResourceManager->GetResource<IMaterial>(mMaterialHandle);

		auto pAttachedShader = pResourceManager->GetResource<IShader>(pMaterial->GetShaderHandle());

		mpVertexDeclaration->Bind(pGraphicsContext, { mVertexBufferHandle }, pAttachedShader.Get());

		pMaterial->Bind(mMaterialInstanceId);

		if (pMaterial->IsScissorTestEnabled())
		{
			pGraphicsContext->SetScissorRect(mScissorRect);
		}

		pGraphicsContext->SetVertexBuffer(0, mVertexBufferHandle, 0, mpVertexDeclaration->GetStrideSize(0)); /// \todo replace magic constants
		pGraphicsContext->SetIndexBuffer(mIndexBufferHandle, 0);

		pGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_PER_OBJECT, reinterpret_cast<const U8*>(&mObjectData), sizeof(mObjectData));

		if (mUseIndexedCommand)
		{
			pGraphicsContext->DrawIndirectIndexedInstanced(mPrimitiveType, E_INDEX_FORMAT_TYPE::INDEX16, mArgsBufferHandle, mAlignedOffset);
		}
		else
		{
			pGraphicsContext->DrawIndirectInstanced(mPrimitiveType, mArgsBufferHandle, mAlignedOffset);
		}

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