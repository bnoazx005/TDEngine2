#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/IVertexBuffer.h"
#include "../../include/graphics/IIndexBuffer.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IResource.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/IShader.h"
#include "../../include/graphics/IGlobalShaderProperties.h"
#include "../../include/utils/CFileLogger.h"
#include <algorithm>
#include <stringUtils.hpp>


namespace TDEngine2
{
	static const std::string InvalidMaterialMessage = "{0} Invalid material was passed into the render command";


	E_RESULT_CODE TDrawCommand::Submit(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IGlobalShaderProperties* pGlobalShaderProperties)
	{
		if (TResourceId::Invalid == mMaterialHandle)
		{
			LOG_ERROR(Wrench::StringUtils::Format(InvalidMaterialMessage, "[TDrawCommand]"));
			return RC_INVALID_ARGS;
		}

		IMaterial* pMaterial = pResourceManager->GetResource<IMaterial>(mMaterialHandle);

		IShader* pAttachedShader = pResourceManager->GetResource<IShader>(pMaterial->GetShaderHandle());

		mpVertexDeclaration->Bind(pGraphicsContext, { mpVertexBuffer }, pAttachedShader);

		pMaterial->Bind(mMaterialInstanceId);

		if (mpVertexBuffer)
		{
			mpVertexBuffer->Bind(0, 0, mpVertexDeclaration->GetStrideSize(0)); /// \todo replace magic constants
		}

		pGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_PER_OBJECT, reinterpret_cast<const U8*>(&mObjectData), sizeof(mObjectData));

		pGraphicsContext->Draw(mPrimitiveType, mStartVertex, mNumOfVertices);

		return RC_OK;
	}


	E_RESULT_CODE TDrawIndexedCommand::Submit(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IGlobalShaderProperties* pGlobalShaderProperties)
	{
		if (TResourceId::Invalid == mMaterialHandle)
		{
			LOG_ERROR(Wrench::StringUtils::Format(InvalidMaterialMessage, "[TDrawIndexedCommand]"));
			return RC_INVALID_ARGS;
		}

		IMaterial* pMaterial = pResourceManager->GetResource<IMaterial>(mMaterialHandle);

		IShader* pAttachedShader = pResourceManager->GetResource<IShader>(pMaterial->GetShaderHandle());

		mpVertexDeclaration->Bind(pGraphicsContext, { mpVertexBuffer }, pAttachedShader);

		pMaterial->Bind(mMaterialInstanceId);

		mpVertexBuffer->Bind(0, 0, mpVertexDeclaration->GetStrideSize(0)); /// \todo replace magic constants

		mpIndexBuffer->Bind(0);

		pGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_PER_OBJECT, reinterpret_cast<const U8*>(&mObjectData), sizeof(mObjectData));
		
		pGraphicsContext->DrawIndexed(mPrimitiveType, mpIndexBuffer->GetIndexFormat(), mStartVertex, mStartIndex, mNumOfIndices);

		return RC_OK;
	}

	E_RESULT_CODE TDrawInstancedCommand::Submit(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IGlobalShaderProperties* pGlobalShaderProperties)
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE TDrawIndexedInstancedCommand::Submit(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IGlobalShaderProperties* pGlobalShaderProperties)
	{
		if (TResourceId::Invalid == mMaterialHandle)
		{
			LOG_ERROR(Wrench::StringUtils::Format(InvalidMaterialMessage, "[TDrawIndexedInstancedCommand]"));
			return RC_INVALID_ARGS;
		}

		IMaterial* pMaterial = pResourceManager->GetResource<IMaterial>(mMaterialHandle);

		IShader* pAttachedShader = pResourceManager->GetResource<IShader>(pMaterial->GetShaderHandle());

		mpVertexDeclaration->Bind(pGraphicsContext, { mpVertexBuffer, mpInstancingBuffer }, pAttachedShader);

		pMaterial->Bind(mMaterialInstanceId);

		mpVertexBuffer->Bind(0, 0, mpVertexDeclaration->GetStrideSize(0)); /// \todo replace magic constants
		mpInstancingBuffer->Bind(1, 0, mpVertexDeclaration->GetStrideSize(1));

		mpIndexBuffer->Bind(0);

		pGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_PER_OBJECT, reinterpret_cast<const U8*>(&mObjectData), sizeof(mObjectData));

		pGraphicsContext->DrawIndexedInstanced(mPrimitiveType, mpIndexBuffer->GetIndexFormat(), mBaseVertexIndex, mStartIndex, 
											   mStartInstance, mIndicesPerInstance, mNumOfInstances);

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

	E_RESULT_CODE CRenderQueue::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

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

	CRenderQueue::CRenderQueueIterator CRenderQueue::GetIterator()
	{
		return CRenderQueueIterator(mCommandsBuffer);
	}


	TDE2_API CRenderQueue* CreateRenderQueue(IAllocator* pTempAllocator, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CRenderQueue, CRenderQueue, result, pTempAllocator);
	}
}