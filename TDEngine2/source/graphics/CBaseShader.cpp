#include "../../include/graphics/CBaseShader.h"
#include "../../include/graphics/IShaderCompiler.h"
#include "../../include/graphics/IConstantBuffer.h"
#include "../../include/graphics/ITexture.h"


namespace TDEngine2
{
	CBaseShader::CBaseShader():
		CBaseResource()
	{
	}

	E_RESULT_CODE CBaseShader::Unload()
	{
		if (mpShaderMeta)
		{
			delete mpShaderMeta;
		}

		return CBaseResource::Unload();
	}

	E_RESULT_CODE CBaseShader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name)
	{
		E_RESULT_CODE result = _init(pResourceManager, name);

		if (result != RC_OK)
		{
			return result;
		}

		if (!pGraphicsContext)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseShader::Compile(const IShaderCompiler* pShaderCompiler, const std::string& sourceCode)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pShaderCompiler)
		{
			return RC_INVALID_ARGS;
		}

		mSourceCode = sourceCode;

		TResult<TShaderCompilerOutput*> compilerOutput = pShaderCompiler->Compile(sourceCode);

		if (compilerOutput.HasError())
		{
			return compilerOutput.GetError();
		}

		mpShaderMeta = compilerOutput.Get();
		
		E_RESULT_CODE result = _createInternalHandlers(mpShaderMeta); /// reimplement this method in a derived class to do some extra work

		if (result != RC_OK)
		{
			return result;
		}

		if ((result = _createTexturesHashTable(mpShaderMeta)) != RC_OK)
		{
			return result;
		}

		if (result != RC_OK)
		{
			return result;
		}

		return RC_OK;
	}
	
	E_RESULT_CODE CBaseShader::SetUserUniformsBuffer(U8 slot, const U8* pData, USIZE dataSize)
	{
		if (slot >= MaxNumberOfUserConstantBuffers)
		{
			return RC_INVALID_ARGS;
		}

		/// \todo add checking for sizes of input data
		IConstantBuffer* pCurrUniformBuffer = mUniformBuffers[slot];

		if (!pCurrUniformBuffer)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = pCurrUniformBuffer->Map(BMT_WRITE_DISCARD);

		if (result != RC_OK)
		{
			return result;
		}

		if ((result = pCurrUniformBuffer->Write(pData, dataSize)) != RC_OK)
		{
			return result;
		}

		pCurrUniformBuffer->Unmap();

		/// \note add the offset because all user-defined buffers go after the internal ones
		_bindUniformBuffer(TotalNumberOfInternalConstantBuffers + slot, pCurrUniformBuffer);

		return RC_OK;
	}

	void CBaseShader::Bind()
	{
		ITexture* pCurrTexture = nullptr;

		for (U32 i = 0; i < mpTextures.size(); ++i)
		{
			pCurrTexture = mpTextures[i];

			if (!pCurrTexture)
			{
				continue;
			}

			pCurrTexture->Bind(i);
		}

		U8 currUserBufferId = 0;
		for (auto& pCurrUniformBuffer : mUniformBuffers)
		{
			_bindUniformBuffer(TotalNumberOfInternalConstantBuffers + currUserBufferId++, pCurrUniformBuffer);
		}
	}

	E_RESULT_CODE CBaseShader::SetTextureResource(const std::string& resourceName, ITexture* pTexture)
	{
		if (resourceName.empty() || !pTexture)
		{
			return RC_INVALID_ARGS;
		}

		auto hashIter = mTexturesHashTable.find(resourceName);

		if (hashIter == mTexturesHashTable.cend())
		{
			return RC_FAIL;
		}

		mpTextures[hashIter->second] = pTexture;

		return RC_OK;
	}

	const TShaderCompilerOutput* CBaseShader::GetShaderMetaData() const
	{
		return mpShaderMeta;
	}

	E_RESULT_CODE CBaseShader::_freeUniformBuffers()
	{
		E_RESULT_CODE result      = RC_OK;
		E_RESULT_CODE totalResult = RC_OK;

		IConstantBuffer* pCurrBuffer = nullptr;
		
		for (auto iter = mUniformBuffers.begin(); iter != mUniformBuffers.end(); ++iter)
		{
			pCurrBuffer = (*iter);

			if (!pCurrBuffer)
			{
				continue;
			}

			result = pCurrBuffer->Free();

			if (result != RC_OK)
			{
				totalResult = result;
			}
		}

		return result;
	}

	E_RESULT_CODE CBaseShader::_createTexturesHashTable(const TShaderCompilerOutput* pCompilerData)
	{
		auto shaderResourcesMap = pCompilerData->mShaderResourcesInfo;

		if (shaderResourcesMap.empty())
		{
			return RC_OK;
		}

		U8 currSlotIndex = 0;
		
		for (auto currShaderResourceInfo : shaderResourcesMap)
		{
			currSlotIndex = currShaderResourceInfo.second.mSlot;

			mTexturesHashTable[currShaderResourceInfo.first] = currSlotIndex;

			mpTextures.resize(currSlotIndex + 1);

			mpTextures[currSlotIndex] = nullptr;
		}

		return RC_OK;
	}

	const TPtr<IResourceLoader> CBaseShader::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IShader>();
	}
}