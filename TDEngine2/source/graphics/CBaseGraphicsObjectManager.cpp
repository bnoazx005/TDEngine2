#include "../../include/graphics/CBaseGraphicsObjectManager.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IFile.h"
#include "../../include/graphics/CDebugUtility.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/CBaseShader.h"
#include <unordered_map>
#include <algorithm>


namespace TDEngine2
{
	CBaseGraphicsObjectManager::CBaseGraphicsObjectManager() :
		CBaseObject(), mpDebugUtility(nullptr)
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

	E_RESULT_CODE CBaseGraphicsObjectManager::DestroyBuffer(TBufferHandleId bufferHandle)
	{
		if (TBufferHandleId::Invalid == bufferHandle)
		{
			return RC_INVALID_ARGS;
		}

		const USIZE bufferPlacementIndex = static_cast<USIZE>(bufferHandle);
		if (bufferPlacementIndex >= mBuffersArray.size())
		{
			return RC_FAIL;
		}

		mBuffersArray[bufferPlacementIndex] = nullptr;

		return RC_OK;
	}

	IGraphicsContext* CBaseGraphicsObjectManager::GetGraphicsContext() const
	{
		return mpGraphicsContext;
	}

	E_DEFAULT_SHADER_TYPE CBaseGraphicsObjectManager::GetDefaultShaderTypeByName(const std::string& name)
	{
		return E_DEFAULT_SHADER_TYPE::DST_BASIC;
	}

	TPtr<IBuffer> CBaseGraphicsObjectManager::GetBufferPtr(TBufferHandleId handle)
	{
		if (TBufferHandleId::Invalid == handle)
		{
			return nullptr;
		}

		const USIZE bufferPlacementIndex = static_cast<USIZE>(handle);
		if (bufferPlacementIndex >= mBuffersArray.size())
		{
			return nullptr;
		}

		return mBuffersArray[bufferPlacementIndex];
	}

	TBufferHandleId CBaseGraphicsObjectManager::_insertBuffer(TPtr<IBuffer> pBuffer)
	{
		auto it = std::find(mBuffersArray.begin(), mBuffersArray.end(), nullptr);
		const USIZE placementIndex = static_cast<USIZE>(std::distance(mBuffersArray.begin(), it));

		if (placementIndex >= mBuffersArray.size())
		{
			mBuffersArray.emplace_back(pBuffer);
		}
		else
		{
			mBuffersArray[placementIndex] = pBuffer;
		}

		return static_cast<TBufferHandleId>(placementIndex);
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