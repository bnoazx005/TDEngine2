#include "./../../include/graphics/CBaseGraphicsObjectManager.h"
#include "./../../include/graphics/IVertexDeclaration.h"
#include "./../../include/core/IGraphicsContext.h"
#include "./../../include/graphics/CDebugUtility.h"
#include "./../../include/graphics/IRenderer.h"


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

	E_RESULT_CODE CBaseGraphicsObjectManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		if (((result = _freeBuffers()) != RC_OK)            ||
			((result = _freeVertexDeclarations()) != RC_OK) ||
			((result = _freeTextureSamplers()) != RC_OK)    ||
			((result = mpDebugUtility->Free()) != RC_OK))
		{
			return result;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	TResult<IDebugUtility*> CBaseGraphicsObjectManager::CreateDebugUtility(IResourceManager* pResourceManager, IRenderer* pRenderer)
	{
		if (mpDebugUtility)
		{
			return TOkValue<IDebugUtility*>(mpDebugUtility);
		}

		E_RESULT_CODE result = RC_OK;

		mpDebugUtility = TDEngine2::CreateDebugUtility(pResourceManager, pRenderer, this, result);

		if (result != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		return TOkValue<IDebugUtility*>(mpDebugUtility);
	}

	IGraphicsContext* CBaseGraphicsObjectManager::GetGraphicsContext() const
	{
		return mpGraphicsContext;
	}

	void CBaseGraphicsObjectManager::_insertBuffer(IBuffer* pBuffer)
	{
		U32 index = 0;

		if (mFreeBuffersSlots.empty())
		{
			index = mBuffersArray.size();

			mBuffersArray.push_back(pBuffer);

			return;
		}

		index = mFreeBuffersSlots.front();

		mFreeBuffersSlots.pop_front();

		mBuffersArray[index] = pBuffer;

		return;
	}

	void CBaseGraphicsObjectManager::_insertVertexDeclaration(IVertexDeclaration* pVertDecl)
	{
		U32 index = 0;

		if (mFreeVertDeclsSlots.empty())
		{
			index = mVertexDeclarationsArray.size();

			mVertexDeclarationsArray.push_back(pVertDecl);

			return;
		}

		index = mFreeVertDeclsSlots.front();

		mFreeVertDeclsSlots.pop_front();

		mVertexDeclarationsArray[index] = pVertDecl;

		return;
	}

	E_RESULT_CODE CBaseGraphicsObjectManager::_freeBuffers()
	{
		IBuffer* pCurrBuffer = nullptr;

		E_RESULT_CODE result      = RC_OK;
		E_RESULT_CODE totalResult = RC_OK;

		for (TBuffersArray::iterator iter = mBuffersArray.begin(); iter != mBuffersArray.end(); ++iter)
		{
			pCurrBuffer = (*iter);

			if (!pCurrBuffer)
			{
				continue;
			}

			if ((result = pCurrBuffer->Free()) != RC_OK)
			{
				totalResult = result;
			}
		}

		mBuffersArray.clear();
		mFreeBuffersSlots.clear();

		return totalResult;
	}

	E_RESULT_CODE CBaseGraphicsObjectManager::_freeVertexDeclarations()
	{
		IVertexDeclaration* pCurrVertDecl = nullptr;

		E_RESULT_CODE result      = RC_OK;
		E_RESULT_CODE totalResult = RC_OK;

		for (TVertexDeclarationsArray::iterator iter = mVertexDeclarationsArray.begin(); iter != mVertexDeclarationsArray.end(); ++iter)
		{
			pCurrVertDecl = (*iter);

			if (!pCurrVertDecl)
			{
				continue;
			}

			if ((result = pCurrVertDecl->Free()) != RC_OK)
			{
				totalResult = result;
			}
		}

		mVertexDeclarationsArray.clear();
		mFreeVertDeclsSlots.clear();

		return totalResult;
	}
}