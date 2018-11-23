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
}