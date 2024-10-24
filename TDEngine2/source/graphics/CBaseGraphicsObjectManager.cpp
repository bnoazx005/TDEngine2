#include "../../include/graphics/CBaseGraphicsObjectManager.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/graphics/CDebugUtility.h"
#include "../../include/graphics/IRenderer.h"
#include <unordered_map>


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
		E_RESULT_CODE result = _freeBuffers();

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

	IGraphicsContext* CBaseGraphicsObjectManager::GetGraphicsContext() const
	{
		return mpGraphicsContext;
	}

	E_DEFAULT_SHADER_TYPE CBaseGraphicsObjectManager::GetDefaultShaderTypeByName(const std::string& name)
	{
		static const std::unordered_map<std::string, E_DEFAULT_SHADER_TYPE> shaderNameToTypeMapping
		{
			{ "DefaultEditorUI", E_DEFAULT_SHADER_TYPE::DST_EDITOR_UI },
			{ "DefaultScreenSpaceEffect", E_DEFAULT_SHADER_TYPE::DST_SCREEN_SPACE },
			{ "ToneMappingPass", E_DEFAULT_SHADER_TYPE::DST_TONE_MAPPING_PASS },
			{ "Selection", E_DEFAULT_SHADER_TYPE::DST_SELECTION },
			{ "SelectionSkinned", E_DEFAULT_SHADER_TYPE::DST_SELECTION_SKINNED },
			{ "SelectionUI", E_DEFAULT_SHADER_TYPE::DST_SELECTION_UI },
			{ "SelectionOutline", E_DEFAULT_SHADER_TYPE::DST_SELECTION_OUTLINE },
			{ "SelectionSkinnedOutline", E_DEFAULT_SHADER_TYPE::DST_SELECTION_SKINNED_OUTLINE },
			{ "ShadowPass", E_DEFAULT_SHADER_TYPE::DST_SHADOW_PASS },
			{ "SkinnedShadowPass", E_DEFAULT_SHADER_TYPE::DST_SHADOW_SKINNED_PASS },
		};

		if (shaderNameToTypeMapping.find(name) == shaderNameToTypeMapping.cend())
		{
			return E_DEFAULT_SHADER_TYPE::DST_BASIC;
		}

		return shaderNameToTypeMapping.at(name);
	}

	void CBaseGraphicsObjectManager::_insertBuffer(IBuffer* pBuffer)
	{
		U32 index = 0;

		if (mFreeBuffersSlots.empty())
		{
			index = static_cast<U32>(mBuffersArray.size());

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
			index = static_cast<U32>(mVertexDeclarationsArray.size());

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

		E_RESULT_CODE result = RC_OK;

		for (auto iter = mBuffersArray.begin(); iter != mBuffersArray.end(); ++iter)
		{
			pCurrBuffer = (*iter);

			if (!pCurrBuffer)
			{
				continue;
			}

			result = result | pCurrBuffer->Free();
		}

		mBuffersArray.clear();
		mFreeBuffersSlots.clear();

		return result;
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