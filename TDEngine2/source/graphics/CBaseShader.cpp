#include "./../../include/graphics/CBaseShader.h"
#include "./../../include/graphics/IShaderCompiler.h"
#include "./../../include/graphics/IConstantBuffer.h"


namespace TDEngine2
{
	CBaseShader::CBaseShader():
		CBaseResource()
	{
	}

	E_RESULT_CODE CBaseShader::Unload()
	{
		return Reset();
	}

	E_RESULT_CODE CBaseShader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, TResourceId id)
	{
		E_RESULT_CODE result = _init(pResourceManager, name, id);

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

		TShaderCompilerOutput* pCompilerData = compilerOutput.Get();

		E_RESULT_CODE result = _createInternalHandlers(pCompilerData); /// reimplement this method in a derived class to do some extra work

		delete pCompilerData;

		if (result != RC_OK)
		{
			return result;
		}

		return RC_OK;
	}

	E_RESULT_CODE CBaseShader::SetInternalUniformsBuffer(E_INTERNAL_UNIFORM_BUFFER_REGISTERS slot, const U8* pData, U32 dataSize)
	{
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

		if ((result = pCurrUniformBuffer->Write(pData, sizeof(dataSize))) != RC_OK)
		{
			return result;
		}

		pCurrUniformBuffer->Unmap();

		_bindUniformBuffer(slot, pCurrUniformBuffer);

		return RC_OK;
	}

	E_RESULT_CODE CBaseShader::SetUserUniformsBuffer(U8 slot, const U8* pData, U32 dataSize)
	{
		if (slot >= MaxNumberOfUserConstantBuffers)
		{
			return RC_INVALID_ARGS;
		}

		U32 goalSlot = IUBR_LAST_USED_SLOT + 1 + slot;

		/// \todo add checking for sizes of input data
		IConstantBuffer* pCurrUniformBuffer = mUniformBuffers[goalSlot];

		if (!pCurrUniformBuffer)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = pCurrUniformBuffer->Map(BMT_WRITE_DISCARD);

		if (result != RC_OK)
		{
			return result;
		}

		if ((result = pCurrUniformBuffer->Write(pData, sizeof(dataSize))) != RC_OK)
		{
			return result;
		}

		pCurrUniformBuffer->Unmap();

		_bindUniformBuffer(slot, pCurrUniformBuffer);

		return RC_OK;
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
}