#include "./../../include/graphics/CBaseShader.h"
#include "./../../include/graphics/IShaderCompiler.h"


namespace TDEngine2
{
	CBaseShader::CBaseShader():
		CBaseResource()
	{
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

		TShaderCompilerResult compilerOutput = pShaderCompiler->Compile(sourceCode);

		if (compilerOutput.mResultCode != RC_OK)
		{
			return compilerOutput.mResultCode;
		}

		E_RESULT_CODE result = _createInternalHandlers(compilerOutput); /// reimplement this method in a derived class to do some extra work

		if (result != RC_OK)
		{
			return result;
		}

		return RC_OK;
	}
}