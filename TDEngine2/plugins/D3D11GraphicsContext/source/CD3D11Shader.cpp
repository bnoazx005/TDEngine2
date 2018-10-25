#include "./../include/CD3D11Shader.h"
#include "./../include/CD3D11GraphicsContext.h"


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CD3D11Shader::CD3D11Shader() :
		CBaseShader()
	{
	}

	E_RESULT_CODE CD3D11Shader::_createInternalHandlers(const TShaderCompilerResult& shaderBytecode)
	{
		U32 bytecodeSize = shaderBytecode.mVSByteCode.size();

		TGraphicsCtxInternalData graphicsInternalData = mpGraphicsContext->GetInternalData();

		ID3D11Device* p3dDevice = nullptr;

#if _HAS_CXX17
		p3dDevice = std::get<TD3D11CtxInternalData>(graphicsInternalData).mp3dDevice;
#else
		p3dDevice = graphicsInternalData.mD3D11.mp3dDevice;
#endif

		if (bytecodeSize > 0) /// create a vertex shader
		{
			if (FAILED(p3dDevice->CreateVertexShader(&shaderBytecode.mVSByteCode[0], bytecodeSize, nullptr, &mpVertexShader)))
			{
				return RC_FAIL;
			}
		}

		/// create a pixel shader
		bytecodeSize = shaderBytecode.mPSByteCode.size();

		if (bytecodeSize > 0)
		{
			if (FAILED(p3dDevice->CreatePixelShader(&shaderBytecode.mPSByteCode[0], bytecodeSize, nullptr, &mpPixelShader)))
			{
				return RC_FAIL;
			}
		}

		/// create a geometry shader
		bytecodeSize = shaderBytecode.mGSByteCode.size();

		if (bytecodeSize > 0)
		{
			if (FAILED(p3dDevice->CreateGeometryShader(&shaderBytecode.mGSByteCode[0], bytecodeSize, nullptr, &mpGeometryShader)))
			{
				return RC_FAIL;
			}
		}

		return RC_OK;
	}


	CD3D11ShaderFactory::CD3D11ShaderFactory() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CD3D11ShaderFactory::Init(IGraphicsContext* pGraphicsContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;
		
		return RC_OK;
	}

	E_RESULT_CODE CD3D11ShaderFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IResource* CD3D11ShaderFactory::Create(const TBaseResourceParameters* pParams) const
	{
		return nullptr;
	}

	U32 CD3D11ShaderFactory::GetResourceTypeId() const
	{
		return CD3D11Shader::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateD3D11ShaderFactory(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		CD3D11ShaderFactory* pShaderFactoryInstance = new (std::nothrow) CD3D11ShaderFactory();

		if (!pShaderFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pShaderFactoryInstance->Init(pGraphicsContext);

		if (result != RC_OK)
		{
			delete pShaderFactoryInstance;

			pShaderFactoryInstance = nullptr;
		}

		return pShaderFactoryInstance;
	}
}

#endif