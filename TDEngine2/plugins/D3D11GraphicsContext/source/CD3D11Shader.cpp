#include "../include/CD3D11Shader.h"
#include "../include/CD3D11GraphicsContext.h"
#include "../include/CD3D11Utils.h"
#include "../include/CD3D11ShaderCompiler.h"
#include "../include/CD3D11ConstantBuffer.h"
#include <graphics/CBaseShader.h>
#include <graphics/IShaderCompiler.h>
#include <editor/CPerfProfiler.h>
#include <cstring>
#include <array>


#if defined (TDE2_USE_WINPLATFORM)


namespace TDEngine2
{
	CD3D11Shader::CD3D11Shader() :
		CBaseShader(), mp3dDeviceContext(nullptr), mpVertexShader(nullptr), mpPixelShader(nullptr), mpGeometryShader(nullptr), mpComputeShader(nullptr)
	{
	}
	
	E_RESULT_CODE CD3D11Shader::Reset()
	{
		mIsInitialized = false;

		E_RESULT_CODE result = RC_OK;

		if ((result = SafeReleaseCOMPtr<ID3D11VertexShader>(&mpVertexShader)) != RC_OK		||
			(result = SafeReleaseCOMPtr<ID3D11PixelShader>(&mpPixelShader)) != RC_OK		||
			(result = SafeReleaseCOMPtr<ID3D11GeometryShader>(&mpGeometryShader)) != RC_OK        ||
			(result = SafeReleaseCOMPtr<ID3D11ComputeShader>(&mpComputeShader)) != RC_OK)
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
		CBaseShader::Bind();

		if (!mp3dDeviceContext || !mIsInitialized)
		{
			return;
		}

		mp3dDeviceContext->VSSetShader(mpVertexShader, nullptr, 0);
		mp3dDeviceContext->PSSetShader(mpPixelShader, nullptr, 0);
		mp3dDeviceContext->GSSetShader(mpGeometryShader, nullptr, 0);
		
		if (mpComputeShader)
		{
			mp3dDeviceContext->CSSetShader(mpComputeShader, nullptr, 0);
		}
	}

	void CD3D11Shader::Unbind()
	{
		/*mp3dDeviceContext->VSSetShader(nullptr, nullptr, 0);
		mp3dDeviceContext->PSSetShader(nullptr, nullptr, 0);
		mp3dDeviceContext->GSSetShader(nullptr, nullptr, 0);*/
	}

	E_RESULT_CODE CD3D11Shader::_createInternalHandlers(const TShaderCompilerOutput* pCompilerData)
	{
		const TD3D11ShaderCompilerOutput* pD3D11ShaderCompilerData = dynamic_cast<const TD3D11ShaderCompilerOutput*>(pCompilerData);

		if (!pD3D11ShaderCompilerData)
		{
			return RC_INVALID_ARGS;
		}
		
		USIZE bytecodeSize = pD3D11ShaderCompilerData->mVSByteCode.size();

		mVertexShaderBytecode.mpBytecode = new U8[bytecodeSize];		
		mVertexShaderBytecode.mLength = bytecodeSize;

		if (bytecodeSize > 0)
		{
			memcpy(mVertexShaderBytecode.mpBytecode, &pD3D11ShaderCompilerData->mVSByteCode[0], bytecodeSize * sizeof(U8));
		}
		
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

		/// create a compute shader
		bytecodeSize = pD3D11ShaderCompilerData->mCSByteCode.size();

		if (bytecodeSize > 0)
		{
			if (FAILED(p3dDevice->CreateComputeShader(&pD3D11ShaderCompilerData->mCSByteCode[0], bytecodeSize, nullptr, &mpComputeShader)))
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

		mUniformBuffers.resize(uniformBuffersInfo.size() - TotalNumberOfInternalConstantBuffers);

		IConstantBuffer* pConstantBuffer = nullptr;

		/// here only user uniforms buffers are created
		for (auto iter = uniformBuffersInfo.cbegin(); iter != uniformBuffersInfo.cend(); ++iter)
		{
			currDesc = (*iter).second;

			/// skip internal buffers, because they are created separately by IGlobalShaderProperties implementation
			if ((currDesc.mFlags & E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL) == E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL)
			{
				continue;
			}

			pConstantBuffer = CreateD3D11ConstantBuffer(mpGraphicsContext, BUT_DYNAMIC, currDesc.mSize, nullptr, result);

			const U32 index = static_cast<U32>(iter->second.mSlot - TotalNumberOfInternalConstantBuffers);
			TDE2_ASSERT(index >= 0 && index <= 1024);

			/// \note Ensure that we compute correct size of the constant buffer. We use ID3D11ShaderReflection to retrieve accurate information


			mUniformBuffers[index] = pConstantBuffer; // the offset is used because the shaders doesn't store internal buffer by themselves
		}

		return RC_OK;
	}

	TShaderCompilerOutput* CD3D11Shader::_createMetaDataFromShaderParams(IShaderCache* pShaderCache, const TShaderParameters* pShaderParams)
	{
		static const std::string ShaderLangId = "hlsl";

		TD3D11ShaderCompilerOutput* pResult = new TD3D11ShaderCompilerOutput();
		
		for (auto&& currShaderResourceInfo : pShaderParams->mShaderResourcesInfo)
		{
			pResult->mShaderResourcesInfo.emplace(currShaderResourceInfo.first, currShaderResourceInfo.second);
		}

		for (auto&& currUniformBufferInfo : pShaderParams->mUniformBuffersInfo)
		{
			pResult->mUniformBuffersInfo.emplace(currUniformBufferInfo.first, currUniformBufferInfo.second);
		}

		auto it = pShaderParams->mStages.find(E_SHADER_STAGE_TYPE::SST_VERTEX);
		if (it != pShaderParams->mStages.cend())
		{
			pResult->mVSByteCode = std::move(pShaderCache->GetBytecode(it->second.mBytecodeInfo.at(ShaderLangId)));
		}

		it = pShaderParams->mStages.find(E_SHADER_STAGE_TYPE::SST_PIXEL);
		if (it != pShaderParams->mStages.cend())
		{
			pResult->mPSByteCode = std::move(pShaderCache->GetBytecode(it->second.mBytecodeInfo.at(ShaderLangId)));
		}

		it = pShaderParams->mStages.find(E_SHADER_STAGE_TYPE::SST_GEOMETRY);
		if (it != pShaderParams->mStages.cend())
		{
			pResult->mGSByteCode = std::move(pShaderCache->GetBytecode(it->second.mBytecodeInfo.at(ShaderLangId)));
		}

		it = pShaderParams->mStages.find(E_SHADER_STAGE_TYPE::SST_COMPUTE);
		if (it != pShaderParams->mStages.cend())
		{
			pResult->mCSByteCode = std::move(pShaderCache->GetBytecode(it->second.mBytecodeInfo.at(ShaderLangId)));
		}

		pResult->mShaderLanguageId = ShaderLangId;

		return pResult;
	}
	
	const TShaderBytecodeDesc& CD3D11Shader::GetVertexShaderBytecode() const
	{
		return mVertexShaderBytecode;
	}

	void CD3D11Shader::_bindUniformBuffer(U32 slot, IConstantBuffer* pBuffer)
	{
		pBuffer->Bind(slot);
	}


	TDE2_API IShader* CreateD3D11Shader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IShader, CD3D11Shader, result, pResourceManager, pGraphicsContext, name);
	}


	CD3D11ShaderFactory::CD3D11ShaderFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D11ShaderFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsContext || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;
		
		return RC_OK;
	}

	IResource* CD3D11ShaderFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		return nullptr;
	}

	IResource* CD3D11ShaderFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateD3D11Shader(mpResourceManager, mpGraphicsContext, name, result));
	}

	TypeId CD3D11ShaderFactory::GetResourceTypeId() const
	{
		return IShader::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateD3D11ShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CD3D11ShaderFactory, result, pResourceManager, pGraphicsContext);
	}
}

#endif