#include "../include/CD3D11Shader.h"
#include "../include/CD3D11GraphicsContext.h"
#include "../include/CD3D11Utils.h"
#include "../include/CD3D11ShaderCompiler.h"
#include <graphics/CBaseShader.h>
#include <graphics/IShaderCompiler.h>
#include <graphics/IGraphicsObjectManager.h>
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
			(result = SafeReleaseCOMPtr<ID3D11GeometryShader>(&mpGeometryShader)) != RC_OK  ||
			(result = SafeReleaseCOMPtr<ID3D11ComputeShader>(&mpComputeShader)) != RC_OK)
		{
			return result;
		}
		
		mVertexShaderBytecode.clear();

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
		if (!pCompilerData)
		{
			return RC_INVALID_ARGS;
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

		USIZE bytecodeSize = 0;

		auto it = pCompilerData->mStagesInfo.find(SST_VERTEX);
		if (it != pCompilerData->mStagesInfo.end())
		{
			bytecodeSize = it->second.mBytecode.size();

			mVertexShaderBytecode.resize(bytecodeSize);

			if (bytecodeSize > 0)
			{
				memcpy(mVertexShaderBytecode.data(), &it->second.mBytecode[0], bytecodeSize * sizeof(U8));
			}

			if (bytecodeSize > 0) /// create a vertex shader
			{
				if (FAILED(p3dDevice->CreateVertexShader(&it->second.mBytecode[0], bytecodeSize, nullptr, &mpVertexShader)))
				{
					return RC_FAIL;
				}
			}
		}

		/// create a pixel shader
		it = pCompilerData->mStagesInfo.find(SST_PIXEL);
		if (it != pCompilerData->mStagesInfo.end())
		{
			bytecodeSize = it->second.mBytecode.size();

			if (bytecodeSize > 0)
			{
				if (FAILED(p3dDevice->CreatePixelShader(&it->second.mBytecode[0], bytecodeSize, nullptr, &mpPixelShader)))
				{
					return RC_FAIL;
				}
			}
		}

		/// create a geometry shader
		it = pCompilerData->mStagesInfo.find(SST_GEOMETRY);
		if (it != pCompilerData->mStagesInfo.end())
		{
			bytecodeSize = it->second.mBytecode.size();

			if (bytecodeSize > 0)
			{
				if (FAILED(p3dDevice->CreateGeometryShader(&it->second.mBytecode[0], bytecodeSize, nullptr, &mpGeometryShader)))
				{
					return RC_FAIL;
				}
			}
		}

		/// create a compute shader
		it = pCompilerData->mStagesInfo.find(SST_COMPUTE);
		if (it != pCompilerData->mStagesInfo.end())
		{
			bytecodeSize = it->second.mBytecode.size();

			if (bytecodeSize > 0)
			{
				if (FAILED(p3dDevice->CreateComputeShader(&it->second.mBytecode[0], bytecodeSize, nullptr, &mpComputeShader)))
				{
					return RC_FAIL;
				}
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

		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();
		
		/// here only user uniforms buffers are created
		for (auto iter = uniformBuffersInfo.cbegin(); iter != uniformBuffersInfo.cend(); ++iter)
		{
			currDesc = (*iter).second;

			/// skip internal buffers, because they are created separately by IGlobalShaderProperties implementation
			if ((currDesc.mFlags & E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL) == E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL)
			{
				continue;
			}

			auto createBufferResult = pGraphicsObjectManager->CreateBuffer({ E_BUFFER_USAGE_TYPE::DYNAMIC, E_BUFFER_TYPE::CONSTANT, currDesc.mSize, nullptr });
			if (createBufferResult.HasError())
			{
				return createBufferResult.GetError();
			}

			const U32 index = static_cast<U32>(iter->second.mSlot - TotalNumberOfInternalConstantBuffers);
			TDE2_ASSERT(index >= 0 && index <= 1024);

			/// \note Ensure that we compute correct size of the constant buffer. We use ID3D11ShaderReflection to retrieve accurate information

			mUniformBuffers[index] = createBufferResult.Get(); // the offset is used because the shaders doesn't store internal buffer by themselves
		}

		return RC_OK;
	}

	const std::vector<U8>& CD3D11Shader::GetVertexShaderBytecode() const
	{
		return mVertexShaderBytecode;
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