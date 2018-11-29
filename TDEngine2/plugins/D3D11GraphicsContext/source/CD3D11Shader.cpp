#include "./../include/CD3D11Shader.h"
#include "./../include/CD3D11GraphicsContext.h"
#include "./../include/CD3D11Utils.h"
#include "./../include/CD3D11ShaderCompiler.h"
#include "./../include/CD3D11ConstantBuffer.h"
#include <graphics/CBaseShader.h>
#include <graphics/IShaderCompiler.h>
#include <cstring>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CD3D11Shader::CD3D11Shader() :
		CBaseShader(), mp3dDeviceContext(nullptr), mpVertexShader(nullptr), mpPixelShader(nullptr), mpGeometryShader(nullptr)
	{
	}

	E_RESULT_CODE CD3D11Shader::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		const IResourceLoader* pResourceLoader = mpResourceManager->GetResourceLoader<CBaseShader>();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		return pResourceLoader->LoadResource(this);
	}

	E_RESULT_CODE CD3D11Shader::Unload()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CD3D11Shader::Reset()
	{
		mIsInitialized = false;

		E_RESULT_CODE result = RC_OK;

		if ((result = SafeReleaseCOMPtr<ID3D11VertexShader>(&mpVertexShader)) != RC_OK		||
			(result = SafeReleaseCOMPtr<ID3D11PixelShader>(&mpPixelShader)) != RC_OK		||
			(result = SafeReleaseCOMPtr<ID3D11GeometryShader>(&mpGeometryShader)) != RC_OK)
		{
			return result;
		}
		
		if ((result = _freeUniformBuffers()) != RC_OK)
		{
			return result;
		}

		if (mVertexShaderBytecode.mpBytecode)
		{
			delete[] mVertexShaderBytecode.mpBytecode;

			mVertexShaderBytecode.mpBytecode = nullptr;
		}

		mp3dDeviceContext = nullptr;

		return RC_OK;
	}

	void CD3D11Shader::Bind()
	{
		if (!mp3dDeviceContext || !mIsInitialized)
		{
			return;
		}

		if (mpVertexShader)
		{
			mp3dDeviceContext->VSSetShader(mpVertexShader, nullptr, 0);
		}

		if (mpPixelShader)
		{
			mp3dDeviceContext->PSSetShader(mpPixelShader, nullptr, 0);
		}

		if (mpGeometryShader)
		{
			mp3dDeviceContext->GSSetShader(mpGeometryShader, nullptr, 0);
		}
	}

	void CD3D11Shader::Unbind()
	{
		mp3dDeviceContext->VSSetShader(nullptr, nullptr, 0);
		mp3dDeviceContext->PSSetShader(nullptr, nullptr, 0);
		mp3dDeviceContext->GSSetShader(nullptr, nullptr, 0);
	}

	E_RESULT_CODE CD3D11Shader::_createInternalHandlers(const TShaderCompilerOutput* pCompilerData)
	{
		const TD3D11ShaderCompilerOutput* pD3D11ShaderCompilerData = dynamic_cast<const TD3D11ShaderCompilerOutput*>(pCompilerData);

		if (!pD3D11ShaderCompilerData)
		{
			return RC_INVALID_ARGS;
		}
		
		U32 bytecodeSize = pD3D11ShaderCompilerData->mVSByteCode.size();

		mVertexShaderBytecode.mpBytecode = new U8[bytecodeSize];
		
		mVertexShaderBytecode.mLength = bytecodeSize;

		memcpy(mVertexShaderBytecode.mpBytecode, &pD3D11ShaderCompilerData->mVSByteCode[0], bytecodeSize * sizeof(U8));
		
		TGraphicsCtxInternalData graphicsInternalData = mpGraphicsContext->GetInternalData();

		ID3D11Device* p3dDevice = nullptr;

#if _HAS_CXX17
		p3dDevice = std::get<TD3D11CtxInternalData>(graphicsInternalData).mp3dDevice;

		mp3dDeviceContext = std::get<TD3D11CtxInternalData>(graphicsInternalData).mp3dDeviceContext;
#else
		p3dDevice = graphicsInternalData.mD3D11.mp3dDevice;

		mp3dDeviceContext = graphicsInternalData.mD3D11.mp3dDeviceContext;
#endif

		if (bytecodeSize > 0) /// create a vertex shader
		{
			if (FAILED(p3dDevice->CreateVertexShader(&pD3D11ShaderCompilerData->mVSByteCode[0], bytecodeSize, nullptr, &mpVertexShader)))
			{
				return RC_FAIL;
			}
		}

		/// create a pixel shader
		bytecodeSize = pD3D11ShaderCompilerData->mPSByteCode.size();

		if (bytecodeSize > 0)
		{
			if (FAILED(p3dDevice->CreatePixelShader(&pD3D11ShaderCompilerData->mPSByteCode[0], bytecodeSize, nullptr, &mpPixelShader)))
			{
				return RC_FAIL;
			}
		}

		/// create a geometry shader
		bytecodeSize = pD3D11ShaderCompilerData->mGSByteCode.size();

		if (bytecodeSize > 0)
		{
			if (FAILED(p3dDevice->CreateGeometryShader(&pD3D11ShaderCompilerData->mGSByteCode[0], bytecodeSize, nullptr, &mpGeometryShader)))
			{
				return RC_FAIL;
			}
		}

		return _createUniformBuffers(pCompilerData);
	}

	E_RESULT_CODE CD3D11Shader::_createUniformBuffers(const TShaderCompilerOutput* pCompilerData)
	{
		auto uniformBuffersInfo = pCompilerData->mUniformBuffersInfo;

		TUniformBufferDesc currDesc;

		E_RESULT_CODE result = RC_OK;

		mUniformBuffers.resize(uniformBuffersInfo.size());

		for (auto iter = uniformBuffersInfo.cbegin(); iter != uniformBuffersInfo.cend(); ++iter)
		{
			currDesc = (*iter).second;

			mUniformBuffers[currDesc.mSlot] = CreateD3D11ConstantBuffer(mpGraphicsContext, BUT_DYNAMIC, currDesc.mSize, nullptr, result);
		}

		return RC_OK;
	}
	
	const TShaderBytecodeDesc& CD3D11Shader::GetVertexShaderBytecode() const
	{
		return mVertexShaderBytecode;
	}


	TDE2_API IShader* CreateD3D11Shader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
										TResourceId id, E_RESULT_CODE& result)
	{
		CD3D11Shader* pShaderInstance = new (std::nothrow) CD3D11Shader();

		if (!pShaderInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pShaderInstance->Init(pResourceManager, pGraphicsContext, name, id);

		if (result != RC_OK)
		{
			delete pShaderInstance;

			pShaderInstance = nullptr;
		}

		return pShaderInstance;
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

		if (!pGraphicsContext)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = pGraphicsContext;

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

	IResource* CD3D11ShaderFactory::CreateDefault(const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateD3D11Shader(params.mpResourceManager, mpGraphicsContext, params.mName, params.mId, result));
	}

	U32 CD3D11ShaderFactory::GetResourceTypeId() const
	{
		return CBaseShader::GetTypeId();
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