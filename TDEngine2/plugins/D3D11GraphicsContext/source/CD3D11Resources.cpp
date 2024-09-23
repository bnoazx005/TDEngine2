#include "../include/CD3D11Resources.h"
#include "../include/CD3D11Mappings.h"
#include "../include/CD3D11Utils.h"
#include "../include/CD3D11GraphicsObjectManager.h"
#include <graphics/CBaseGraphicsPipeline.h>
#include <core/IGraphicsContext.h>
#include <memory>
#include "deferOperation.hpp"


#if defined (TDE2_USE_WINPLATFORM)

namespace TDEngine2
{
	static TResult<ID3D11Buffer*> CreateBufferInternal(ID3D11Device* p3dDevice, const TInitBufferParams& params, bool createEmpty = false)
	{
		D3D11_BUFFER_DESC bufferDesc {};

		U32 bufferCreationFlags = 0x0;

		switch (params.mBufferType)
		{
			case E_BUFFER_TYPE::VERTEX:
				bufferCreationFlags = D3D11_BIND_VERTEX_BUFFER;
				break;
			case E_BUFFER_TYPE::INDEX:
				bufferCreationFlags = D3D11_BIND_INDEX_BUFFER;
				break;
			case E_BUFFER_TYPE::CONSTANT:
				bufferCreationFlags = D3D11_BIND_CONSTANT_BUFFER;
				break;
			case E_BUFFER_TYPE::STRUCTURED:
				bufferCreationFlags = D3D11_BIND_SHADER_RESOURCE | (params.mIsUnorderedAccessResource ? D3D11_BIND_UNORDERED_ACCESS : 0x0);
				break;
		}

		bufferDesc.BindFlags = bufferCreationFlags;
		bufferDesc.ByteWidth = static_cast<U32>(params.mTotalBufferSize);
		bufferDesc.CPUAccessFlags = CD3D11Mappings::GetAccessFlags(params.mUsageType);
		bufferDesc.Usage = CD3D11Mappings::GetUsageType(params.mUsageType);

		if (E_BUFFER_TYPE::STRUCTURED == params.mBufferType)
		{
			bufferDesc.StructureByteStride = static_cast<UINT>(params.mElementStrideSize);

			switch (params.mStructuredBufferType)
			{
				case E_STRUCTURED_BUFFER_TYPE::DEFAULT:
				case E_STRUCTURED_BUFFER_TYPE::APPENDABLE:
					bufferDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
					break;
				case E_STRUCTURED_BUFFER_TYPE::RAW:
					bufferDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
					break;
				case E_STRUCTURED_BUFFER_TYPE::INDIRECT_DRAW_BUFFER:
					bufferDesc.MiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
					break;
				default:
					TDE2_UNIMPLEMENTED();
					break;
			}
		}

		D3D11_SUBRESOURCE_DATA bufferData;

		memset(&bufferData, 0, sizeof(bufferData));

		bufferData.pSysMem = createEmpty ? nullptr : params.mpDataPtr;

		ID3D11Buffer* pBuffer = nullptr;

		if (FAILED(p3dDevice->CreateBuffer(&bufferDesc, createEmpty ? nullptr : (params.mpDataPtr ? &bufferData : nullptr), &pBuffer)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

#if TDE2_DEBUG_MODE
		if (params.mName)
		{
			pBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<U32>(strlen(params.mName)), params.mName);
		}
#endif

		return Wrench::TOkValue<ID3D11Buffer*>(pBuffer);
	}


	static TResult<ID3D11ShaderResourceView*> CreateTypedBufferViewInternal(ID3D11Device* p3dDevice, ID3D11Resource* pBuffer, const TInitBufferParams& params)
	{
		if (!params.mElementStrideSize)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		ID3D11ShaderResourceView* pView = nullptr;

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
		viewDesc.Format = DXGI_FORMAT_UNKNOWN;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		viewDesc.Buffer.FirstElement = 0;
		viewDesc.Buffer.NumElements = static_cast<U32>(params.mTotalBufferSize / params.mElementStrideSize);

		if (FAILED(p3dDevice->CreateShaderResourceView(pBuffer, &viewDesc, &pView)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11ShaderResourceView*>(pView);
	}


	static TResult<ID3D11UnorderedAccessView*> CreateWriteableTypedBufferViewInternal(ID3D11Device* p3dDevice, ID3D11Resource* pBuffer, const TInitBufferParams& params)
	{
		if (!params.mElementStrideSize)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		ID3D11UnorderedAccessView* pView = nullptr;

		D3D11_UNORDERED_ACCESS_VIEW_DESC viewDesc;
		viewDesc.Format = E_STRUCTURED_BUFFER_TYPE::INDIRECT_DRAW_BUFFER != params.mStructuredBufferType ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_R32_UINT;
		viewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		viewDesc.Buffer.FirstElement = 0;
		viewDesc.Buffer.Flags = E_STRUCTURED_BUFFER_TYPE::APPENDABLE == params.mStructuredBufferType ? D3D11_BUFFER_UAV_FLAG_APPEND : 0x0;
		viewDesc.Buffer.NumElements = static_cast<U32>(params.mTotalBufferSize / params.mElementStrideSize);

		if (FAILED(p3dDevice->CreateUnorderedAccessView(pBuffer, &viewDesc, &pView)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11UnorderedAccessView*>(pView);
	}


	CD3D11Buffer::CD3D11Buffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D11Buffer::Init(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		TD3D11CtxInternalData internalD3D11Data;

#if _HAS_CXX17
		internalD3D11Data = std::get<TD3D11CtxInternalData>(pGraphicsContext->GetInternalData());
#else
		internalD3D11Data = pGraphicsContext->GetInternalData().mD3D11;
#endif

		mp3dDeviceContext = internalD3D11Data.mp3dDeviceContext;
		mp3dDevice = internalD3D11Data.mp3dDevice;

		if (!mp3dDeviceContext)
		{
			return RC_INVALID_ARGS;
		}

		mBufferSize = params.mTotalBufferSize;
		mBufferUsageType = params.mUsageType;
		mBufferType = params.mBufferType;

		E_RESULT_CODE result = _onInitInternal(params);
		if (RC_OK != result)
		{
			return result;
		}

		mInitParams = params;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D11Buffer::_onInitInternal(const TInitBufferParams& params)
	{
		auto createBufferResourceResult = CreateBufferInternal(mp3dDevice, params, false);
		if (createBufferResourceResult.HasError())
		{
			return createBufferResourceResult.GetError();
		}

		mpBufferInstance = createBufferResourceResult.Get();

		if (E_BUFFER_TYPE::STRUCTURED == params.mBufferType && E_STRUCTURED_BUFFER_TYPE::INDIRECT_DRAW_BUFFER != params.mStructuredBufferType)
		{
			auto createViewResourceResult = CreateTypedBufferViewInternal(mp3dDevice, mpBufferInstance, params);
			if (createBufferResourceResult.HasError())
			{
				return createBufferResourceResult.GetError();
			}

			mpShaderView = createViewResourceResult.Get();
		}

		if (params.mIsUnorderedAccessResource)
		{
			auto createWritableViewResourceResult = CreateWriteableTypedBufferViewInternal(mp3dDevice, mpBufferInstance, params);
			if (createWritableViewResourceResult.HasError())
			{
				return createWritableViewResourceResult.GetError();
			}

			mpWritableShaderView = createWritableViewResourceResult.Get();
		}

		return RC_OK;
	}

	E_RESULT_CODE CD3D11Buffer::_onFreeInternal()
	{
		E_RESULT_CODE result = RC_OK;

		result = SafeReleaseCOMPtr<ID3D11Buffer>(&mpBufferInstance);

		if (mpShaderView)
		{
			result = result | SafeReleaseCOMPtr<ID3D11ShaderResourceView>(&mpShaderView);
		}

		if (mpWritableShaderView)
		{
			result = result | SafeReleaseCOMPtr<ID3D11UnorderedAccessView>(&mpWritableShaderView);
		}

		return RC_OK;
	}

	E_RESULT_CODE CD3D11Buffer::Map(E_BUFFER_MAP_TYPE mapType, USIZE offset)
	{
		D3D11_MAP innerMapType = D3D11_MAP_WRITE_DISCARD;

		switch (mapType)
		{
			case BMT_NOOVERWRITE:
				innerMapType = D3D11_MAP_WRITE_NO_OVERWRITE;
				break;
			case BMT_WRITE:
				innerMapType = D3D11_MAP_WRITE;
				break;
			case BMT_READ_WRITE:
				innerMapType = D3D11_MAP_READ_WRITE;
				break;
			case BMT_READ:
				innerMapType = D3D11_MAP_READ;
				break;
			default:
				break;
		}

		if (FAILED(mp3dDeviceContext->Map(mpBufferInstance, 0, innerMapType, 0, &mMappedBufferData)))
		{
			return RC_FAIL;
		}

		mpLockDataPtr = reinterpret_cast<void*>(reinterpret_cast<U8*>(mMappedBufferData.pData) + offset);

		return RC_OK;
	}

	void CD3D11Buffer::Unmap()
	{
		mp3dDeviceContext->Unmap(mpBufferInstance, 0);
	}

	E_RESULT_CODE CD3D11Buffer::Write(const void* pData, USIZE size)
	{
		if (size > mBufferSize)
		{
			return RC_INVALID_ARGS;
		}

		if (!mpLockDataPtr)
		{
			return RC_FAIL;
		}

		memcpy(mpLockDataPtr, pData, size);

		return RC_OK;
	}

	void* CD3D11Buffer::Read()
	{
		return mpLockDataPtr;
	}

	E_RESULT_CODE CD3D11Buffer::Resize(USIZE newSize)
	{
		auto paramsCopy = mInitParams;
		paramsCopy.mTotalBufferSize = newSize;

		if (mpLockDataPtr)
		{
			Unmap();
		}

		E_RESULT_CODE result = _onFreeInternal();
		if (RC_OK != result)
		{
			return result;
		}

		result = _onInitInternal(paramsCopy);
		if (RC_OK != result)
		{
			return result;
		}

		mBufferSize = newSize;
		mInitParams.mTotalBufferSize = mBufferSize;

		return RC_OK;
	}

	void* CD3D11Buffer::GetInternalData()
	{
		return reinterpret_cast<void*>(mpBufferInstance);
	}

	USIZE CD3D11Buffer::GetSize() const
	{
		return mBufferSize;
	}

	ID3D11DeviceContext* CD3D11Buffer::GetDeviceContext() const
	{
		return mp3dDeviceContext;
	}

	const TInitBufferParams& CD3D11Buffer::GetParams() const
	{
		return mInitParams;
	}

	ID3D11Buffer* CD3D11Buffer::GetD3D11Buffer()
	{
		return mpBufferInstance;
	}

	ID3D11ShaderResourceView* CD3D11Buffer::GetShaderView()
	{
		return mpShaderView;
	}

	ID3D11UnorderedAccessView* CD3D11Buffer::GetWriteableShaderView()
	{
		return mpWritableShaderView;
	}


	IBuffer* CreateD3D11Buffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IBuffer, CD3D11Buffer, result, pGraphicsContext, params);
	}


	/*!
		\brief CD3D11Shader's definition
	*/

	CD3D11Shader::CD3D11Shader() :
		CBaseShader(), mp3dDeviceContext(nullptr), mpVertexShader(nullptr), mpPixelShader(nullptr), mpGeometryShader(nullptr), mpComputeShader(nullptr)
	{
	}

	E_RESULT_CODE CD3D11Shader::Reset()
	{
		mIsInitialized = false;

		E_RESULT_CODE result = RC_OK;

		if ((result = SafeReleaseCOMPtr<ID3D11VertexShader>(&mpVertexShader)) != RC_OK ||
			(result = SafeReleaseCOMPtr<ID3D11PixelShader>(&mpPixelShader)) != RC_OK ||
			(result = SafeReleaseCOMPtr<ID3D11GeometryShader>(&mpGeometryShader)) != RC_OK ||
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
		CBaseShader::Unbind();

		/// \fixme Hack to unbind all UAVs from the pipeline when shader is used with Dispatch
		std::array<ID3D11UnorderedAccessView*, 8> pNullUAVs { nullptr };
		mp3dDeviceContext->CSSetUnorderedAccessViews(0, static_cast<U32>(pNullUAVs.size()), pNullUAVs.data(), 0);

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


	IShader* CreateD3D11Shader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IShader, CD3D11Shader, result, pResourceManager, pGraphicsContext, name);
	}


	/*!
		class CD3D11ShaderFactory

		\brief The class is an abstract factory of CD3D11Shader objects that is used by a resource manager
	*/

	class CD3D11ShaderFactory : public CBaseObject, public IShaderFactory
	{
		public:
			friend IResourceFactory* CreateD3D11ShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			IResource* Create(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			IResource* CreateDefault(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11ShaderFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};


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


	IResourceFactory* CreateD3D11ShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CD3D11ShaderFactory, result, pResourceManager, pGraphicsContext);
	}


	/*!
		\brief CD3D11TextureImpl's definition
	*/

	template <typename T>
	void FillCommonTextureDesc(T& textureDesc, const TInitTextureImplParams& params)
	{
		const bool isCPUAccessible = params.mUsageType != E_TEXTURE_IMPL_USAGE_TYPE::STATIC;
		const bool isDepthBufferResource = E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER == (params.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER);

		textureDesc.Width = params.mWidth;
		textureDesc.Height = params.mHeight;
		textureDesc.Format = isDepthBufferResource ? CD3D11Mappings::GetTypelessVersionOfFormat(params.mFormat) : CD3D11Mappings::GetDXGIFormat(params.mFormat);
		textureDesc.MipLevels = params.mNumOfMipLevels;

		textureDesc.BindFlags = 0x0;

		if (E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE == (params.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE))
		{
			textureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		}

		if (isDepthBufferResource)
		{
			textureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
		}
		else if (E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET == (params.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET))
		{
			textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		}

		if (E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS == (params.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS))
		{
			textureDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}

		textureDesc.CPUAccessFlags = isDepthBufferResource ? 0 : D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		textureDesc.Usage = isCPUAccessible ? D3D11_USAGE_STAGING : D3D11_USAGE_DEFAULT;
		textureDesc.MiscFlags =
			((params.mType == E_TEXTURE_IMPL_TYPE::CUBEMAP) ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0x0) |
			(params.mNumOfMipLevels > 1 ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0x0);

	}


	static TResult<ID3D11Texture2D*> CreateTexture2DResourceInternal(ID3D11Device* p3dDevice, const TInitTextureImplParams& params)
	{
		D3D11_TEXTURE2D_DESC textureDesc;

		memset(&textureDesc, 0, sizeof(textureDesc));
		FillCommonTextureDesc<D3D11_TEXTURE2D_DESC>(textureDesc, params);

		textureDesc.SampleDesc.Count = params.mNumOfSamples;
		textureDesc.SampleDesc.Quality = params.mSamplingQuality;
		textureDesc.ArraySize = (params.mType == E_TEXTURE_IMPL_TYPE::CUBEMAP) ? 6 : params.mArraySize;

		ID3D11Texture2D* pTexture = nullptr;

		/// create blank texture with specified parameters
		if (FAILED(p3dDevice->CreateTexture2D(&textureDesc, nullptr, &pTexture)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

#if TDE2_DEBUG_MODE
		if (params.mName)
		{
			pTexture->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<U32>(strlen(params.mName)), params.mName);
		}
#endif

		return Wrench::TOkValue<ID3D11Texture2D*>(pTexture);
	}


	static TResult<ID3D11Texture3D*> CreateTexture3DResourceInternal(ID3D11Device* p3dDevice, const TInitTextureImplParams& params)
	{
		D3D11_TEXTURE3D_DESC textureDesc;

		memset(&textureDesc, 0, sizeof(textureDesc));
		FillCommonTextureDesc<D3D11_TEXTURE3D_DESC>(textureDesc, params);
		textureDesc.Depth = params.mDepth;

		ID3D11Texture3D* pTexture = nullptr;

		/// create blank texture with specified parameters
		if (FAILED(p3dDevice->CreateTexture3D(&textureDesc, nullptr, &pTexture)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

#if TDE2_DEBUG_MODE
		pTexture->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<U32>(strlen(params.mName)), params.mName);
#endif

		return Wrench::TOkValue<ID3D11Texture3D*>(pTexture);
	}


	static TResult<ID3D11ShaderResourceView*> CreateShaderResourceViewInternal(ID3D11Device* p3dDevice, ID3D11Resource* pTextureResource, const TInitTextureImplParams& params)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;

		memset(&viewDesc, 0, sizeof(viewDesc));

		const bool isCubemap = (params.mType == E_TEXTURE_IMPL_TYPE::CUBEMAP);
		const bool isDepthBufferResource = E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER == (params.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER);

		viewDesc.Format = CD3D11Mappings::GetDXGIFormat(isDepthBufferResource ? CD3D11Mappings::GetBestFitStrongTypeFormat(params.mFormat) : params.mFormat);
		viewDesc.ViewDimension = isCubemap ? D3D11_SRV_DIMENSION_TEXTURECUBE : (params.mArraySize > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DARRAY : D3D11_SRV_DIMENSION_TEXTURE2D);

		if (E_TEXTURE_IMPL_TYPE::TEXTURE_3D == params.mType)
		{
			viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		}

		switch (params.mType)
		{
		case E_TEXTURE_IMPL_TYPE::CUBEMAP:
			viewDesc.TextureCube.MipLevels = params.mNumOfMipLevels;
			viewDesc.TextureCube.MostDetailedMip = 0;
			break;
		case E_TEXTURE_IMPL_TYPE::TEXTURE_2D:
			viewDesc.Texture2D.MipLevels = params.mNumOfMipLevels;
			viewDesc.Texture2D.MostDetailedMip = 0;
			break;
		case E_TEXTURE_IMPL_TYPE::TEXTURE_2D_ARRAY:
			viewDesc.Texture2DArray.MipLevels = params.mNumOfMipLevels;
			viewDesc.Texture2DArray.ArraySize = params.mArraySize;
			viewDesc.Texture2DArray.FirstArraySlice = 0;
			viewDesc.Texture2DArray.MostDetailedMip = 0;
			break;
		case E_TEXTURE_IMPL_TYPE::TEXTURE_3D:
			viewDesc.Texture3D.MipLevels = params.mNumOfMipLevels;
			viewDesc.Texture3D.MostDetailedMip = 0;
			break;
		}

		ID3D11ShaderResourceView* pShaderResourceView = nullptr;

		if (FAILED(p3dDevice->CreateShaderResourceView(pTextureResource, &viewDesc, &pShaderResourceView)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11ShaderResourceView*>(pShaderResourceView);
	}


	static TResult<ID3D11RenderTargetView*> CreateRenderTargetViewInternal(ID3D11Device* p3dDevice, ID3D11Resource* pTextureResource, const TInitTextureImplParams& params)
	{
		D3D11_RENDER_TARGET_VIEW_DESC viewDesc;

		memset(&viewDesc, 0, sizeof(viewDesc));

		const bool isCubemap = (params.mType == E_TEXTURE_IMPL_TYPE::CUBEMAP);

		viewDesc.Format = CD3D11Mappings::GetDXGIFormat(params.mFormat);
		viewDesc.ViewDimension = (params.mArraySize > 1 || isCubemap) ? D3D11_RTV_DIMENSION_TEXTURE2DARRAY : D3D11_RTV_DIMENSION_TEXTURE2D;

		switch (params.mType)
		{
		case E_TEXTURE_IMPL_TYPE::TEXTURE_2D:
			viewDesc.Texture2D.MipSlice = 0;
			break;
		case E_TEXTURE_IMPL_TYPE::TEXTURE_2D_ARRAY:
		case E_TEXTURE_IMPL_TYPE::CUBEMAP:
			viewDesc.Texture2DArray.ArraySize = isCubemap ? 6 : params.mArraySize;
			viewDesc.Texture2DArray.MipSlice = 0;
			viewDesc.Texture2DArray.FirstArraySlice = 0;
			break;
		case E_TEXTURE_IMPL_TYPE::TEXTURE_3D:
			viewDesc.Texture3D.FirstWSlice = 0;
			viewDesc.Texture3D.MipSlice = 0;
			viewDesc.Texture3D.WSize = -1;
			break;
		}

		ID3D11RenderTargetView* pRenderTargetView = nullptr;

		if (FAILED(p3dDevice->CreateRenderTargetView(pTextureResource, &viewDesc, &pRenderTargetView)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11RenderTargetView*>(pRenderTargetView);
	}


	static TResult<ID3D11DepthStencilView*> CreateDepthStencilViewInternal(ID3D11Device* p3dDevice, ID3D11Resource* pTextureResource, const TInitTextureImplParams& params)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;

		memset(&viewDesc, 0, sizeof(viewDesc));

		const bool isCubemap = (params.mType == E_TEXTURE_IMPL_TYPE::CUBEMAP);

		viewDesc.Format = CD3D11Mappings::GetDXGIFormat(params.mFormat);
		viewDesc.ViewDimension = (params.mArraySize > 1 || isCubemap) ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2D;

		switch (params.mType)
		{
		case E_TEXTURE_IMPL_TYPE::TEXTURE_2D:
			viewDesc.Texture2D.MipSlice = 0;
			break;
		case E_TEXTURE_IMPL_TYPE::TEXTURE_2D_ARRAY:
		case E_TEXTURE_IMPL_TYPE::CUBEMAP:
		case E_TEXTURE_IMPL_TYPE::TEXTURE_3D:
			viewDesc.Texture2DArray.ArraySize = isCubemap ? 6 : params.mArraySize;
			viewDesc.Texture2DArray.MipSlice = 0;
			viewDesc.Texture2DArray.FirstArraySlice = 0;
			break;
		}

		ID3D11DepthStencilView* pDepthStencilView = nullptr;

		if (FAILED(p3dDevice->CreateDepthStencilView(pTextureResource, &viewDesc, &pDepthStencilView)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11DepthStencilView*>(pDepthStencilView);
	}


	static TResult<ID3D11UnorderedAccessView*> CreateUnorderedAccessViewInternal(ID3D11Device* p3dDevice, ID3D11Resource* pTextureResource, const TInitTextureImplParams& params)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC viewDesc;

		memset(&viewDesc, 0, sizeof(viewDesc));

		const bool isCubemap = (params.mType == E_TEXTURE_IMPL_TYPE::CUBEMAP);

		viewDesc.Format = CD3D11Mappings::GetDXGIFormat(params.mFormat);
		viewDesc.ViewDimension = (params.mArraySize > 1 || isCubemap) ? D3D11_UAV_DIMENSION_TEXTURE2DARRAY : D3D11_UAV_DIMENSION_TEXTURE2D;

		if (E_TEXTURE_IMPL_TYPE::TEXTURE_3D == params.mType)
		{
			viewDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
		}

		switch (params.mType)
		{
		case E_TEXTURE_IMPL_TYPE::TEXTURE_2D:
			viewDesc.Texture2D.MipSlice = 0;
			break;
		case E_TEXTURE_IMPL_TYPE::TEXTURE_2D_ARRAY:
		case E_TEXTURE_IMPL_TYPE::CUBEMAP:
			viewDesc.Texture2DArray.ArraySize = isCubemap ? 6 : params.mArraySize;
			viewDesc.Texture2DArray.MipSlice = 0;
			viewDesc.Texture2DArray.FirstArraySlice = 0;
			break;
		case E_TEXTURE_IMPL_TYPE::TEXTURE_3D:
			viewDesc.Texture3D.WSize = -1;
			viewDesc.Texture3D.MipSlice = 0;
			viewDesc.Texture3D.FirstWSlice = 0;
			break;
		}

		ID3D11UnorderedAccessView* pUnorderedAccessView = nullptr;

		if (FAILED(p3dDevice->CreateUnorderedAccessView(pTextureResource, &viewDesc, &pUnorderedAccessView)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11UnorderedAccessView*>(pUnorderedAccessView);
	}


	CD3D11TextureImpl::CD3D11TextureImpl() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D11TextureImpl::Init(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params)
	{
		if (!pGraphicsContext ||
			!params.mWidth ||
			!params.mHeight ||
			params.mDepth < 1 ||
			params.mNumOfMipLevels < 1 ||
			!params.mArraySize)
		{
			return RC_INVALID_ARGS;
		}

		mInitParams = params;

		auto&& internalData = pGraphicsContext->GetInternalData();

		mp3dDeviceContext = internalData.mD3D11.mp3dDeviceContext;
		mp3dDevice = internalData.mD3D11.mp3dDevice;

		E_RESULT_CODE result = _onInitInternal();
		if (RC_OK != result)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D11TextureImpl::Resize(U32 width, U32 height, U32 depth)
	{
		E_RESULT_CODE result = _onFreeInternal();

		mInitParams.mWidth = width;
		mInitParams.mHeight = height;
		mInitParams.mDepth = depth;

		return _onInitInternal();
	}

	ID3D11Resource* CD3D11TextureImpl::GetTextureResource()
	{
		return mpTextureResource;
	}

	ID3D11ShaderResourceView* CD3D11TextureImpl::GetShaderResourceView()
	{
		return mpShaderTextureView;
	}

	ID3D11RenderTargetView* CD3D11TextureImpl::GetRenderTargetView()
	{
		return mpRenderTargetView;
	}

	ID3D11DepthStencilView* CD3D11TextureImpl::GetDepthStencilView()
	{
		return mpDepthStencilView;
	}

	ID3D11UnorderedAccessView* CD3D11TextureImpl::GetUnorderedAccessView()
	{
		return mpUavTextureView;
	}

	std::vector<U8> CD3D11TextureImpl::ReadBytes(U32 index)
	{
		TInitTextureImplParams createTextureParams{};
		createTextureParams.mWidth = mInitParams.mWidth;
		createTextureParams.mHeight = mInitParams.mHeight;
		createTextureParams.mFormat = mInitParams.mFormat;
		createTextureParams.mNumOfMipLevels = mInitParams.mNumOfMipLevels;
		createTextureParams.mNumOfSamples = mInitParams.mNumOfSamples;
		createTextureParams.mSamplingQuality = mInitParams.mSamplingQuality;
		createTextureParams.mType = mInitParams.mType;
		createTextureParams.mUsageType = E_TEXTURE_IMPL_USAGE_TYPE::STAGING;

		/// \note create temporary texture with D3D11_USAGE_STAGING flag
		auto createTempTextureResult = CreateTexture2DResourceInternal(mp3dDevice, createTextureParams);
		if (createTempTextureResult.HasError())
		{
			TDE2_ASSERT(false);
			return {};
		}

		ID3D11Resource* pTempTexture = createTempTextureResult.Get();
		defer([=]
			{
				pTempTexture->Release();
			});

		/// \note copy data from actual texture into helper one
		mp3dDeviceContext->CopyResource(pTempTexture, mpTextureResource);

		/// \note retrieve pointer to memory from temp texture
		D3D11_MAPPED_SUBRESOURCE mappedData;

		if (FAILED(mp3dDeviceContext->Map(pTempTexture, index, D3D11_MAP_READ, 0x0, &mappedData)))
		{
			TDE2_ASSERT(false);
			return {};
		}

		const size_t size = static_cast<size_t>(createTextureParams.mWidth * createTextureParams.mHeight * CD3D11Mappings::GetFormatSize(createTextureParams.mFormat));
		std::vector<U8> pixelsData(size);

		memcpy(&pixelsData[0], mappedData.pData, size);

		mp3dDeviceContext->Unmap(pTempTexture, 0);

		return pixelsData;
	}

	const TInitTextureParams& CD3D11TextureImpl::GetParams() const
	{
		return mInitParams;
	}

	E_RESULT_CODE CD3D11TextureImpl::_onInitInternal()
	{
		if (E_TEXTURE_IMPL_TYPE::TEXTURE_3D != mInitParams.mType)
		{
			auto createResourceResult = CreateTexture2DResourceInternal(mp3dDevice, mInitParams);
			if (createResourceResult.HasError())
			{
				return createResourceResult.GetError();
			}

			mpTextureResource = createResourceResult.Get();
		}
		else
		{
			auto createResourceResult = CreateTexture3DResourceInternal(mp3dDevice, mInitParams);
			if (createResourceResult.HasError())
			{
				return createResourceResult.GetError();
			}

			mpTextureResource = createResourceResult.Get();
		}

		auto createDefaultSrvResult = CreateShaderResourceViewInternal(mp3dDevice, mpTextureResource, mInitParams);
		if (createDefaultSrvResult.HasError())
		{
			return createDefaultSrvResult.GetError();
		}

		mpShaderTextureView = createDefaultSrvResult.Get();

		if (E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER == (mInitParams.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER))
		{
			auto createDsvResult = CreateDepthStencilViewInternal(mp3dDevice, mpTextureResource, mInitParams);
			if (createDsvResult.HasError())
			{
				return createDsvResult.GetError();
			}

			mpDepthStencilView = createDsvResult.Get();
		}
		else if (E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET == (mInitParams.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET))
		{
			auto createRtvResult = CreateRenderTargetViewInternal(mp3dDevice, mpTextureResource, mInitParams);
			if (createRtvResult.HasError())
			{
				return createRtvResult.GetError();
			}

			mpRenderTargetView = createRtvResult.Get();
		}

		if (E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS == (mInitParams.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS))
		{
			auto createUavResult = CreateUnorderedAccessViewInternal(mp3dDevice, mpTextureResource, mInitParams);
			if (createUavResult.HasError())
			{
				return createUavResult.GetError();
			}

			mpUavTextureView = createUavResult.Get();
		}

		return RC_OK;
	}

	E_RESULT_CODE CD3D11TextureImpl::_onFreeInternal()
	{
		E_RESULT_CODE result = RC_OK;

		result = result | SafeReleaseCOMPtr<ID3D11Resource>(&mpTextureResource);
		result = result | SafeReleaseCOMPtr<ID3D11ShaderResourceView>(&mpShaderTextureView);

		if (mpUavTextureView)
		{
			result = result | SafeReleaseCOMPtr<ID3D11UnorderedAccessView>(&mpUavTextureView);
		}

		if (mpRenderTargetView)
		{
			result = result | SafeReleaseCOMPtr<ID3D11RenderTargetView>(&mpRenderTargetView);
		}

		if (mpDepthStencilView)
		{
			result = result | SafeReleaseCOMPtr<ID3D11DepthStencilView>(&mpDepthStencilView);
		}

		return result;
	}


	ITextureImpl* CreateD3D11TextureImpl(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITextureImpl, CD3D11TextureImpl, result, pGraphicsContext, params);
	}


	/*!
		\brief CD3D11VertexDeclaration's definition
	*/

	CD3D11VertexDeclaration::CD3D11VertexDeclaration() :
		CVertexDeclaration(), mpInputLayout(nullptr)
	{
	}

	E_RESULT_CODE CD3D11VertexDeclaration::_onFreeInternal()
	{
		return SafeReleaseCOMPtr<ID3D11InputLayout>(&mpInputLayout) | CVertexDeclaration::_onFreeInternal();
	}

	TResult<ID3D11InputLayout*> CD3D11VertexDeclaration::GetInputLayoutByShader(IGraphicsContext* pGraphicsContext, const IShader* pShader)
	{
		const CD3D11Shader* pD3D11Shader = dynamic_cast<const CD3D11Shader*>(pShader);

		if (!pGraphicsContext || !pD3D11Shader)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		ID3D11Device* p3dDevice = nullptr;

		ID3D11InputLayout* pInputLayout = nullptr;

#if _HAS_CXX17
		p3dDevice = std::get<TD3D11CtxInternalData>(pGraphicsContext->GetInternalData()).mp3dDevice;
#else
		p3dDevice = pGraphicsContext->GetInternalData().mD3D11.mp3dDevice;
#endif

		std::vector<D3D11_INPUT_ELEMENT_DESC> elements;

		D3D11_INPUT_ELEMENT_DESC currElement;

		std::unordered_map<E_VERTEX_ELEMENT_SEMANTIC_TYPE, U32> usedSemanticIndex;

		U32 currOffset = 0;

		U32 currInstancingElementIndex = (std::numeric_limits<U32>::max)();
		U32 currInstancesPerData = 0;

		TInstancingInfoArray::const_iterator instancingIter = mInstancingInfo.cbegin();

		if (!mInstancingInfo.empty())
		{
			std::tie(currInstancingElementIndex, currInstancesPerData) = *instancingIter;
		}

		U32 currIndex = 0;

		/// fill in elements vector
		for (auto iter = mElements.cbegin(); iter != mElements.cend(); ++iter, ++currIndex)
		{
			currElement.InstanceDataStepRate = (*iter).mIsPerInstanceData ? currElement.InstanceDataStepRate : 0;

			/// a new instancing division has found
			if ((currIndex == currInstancingElementIndex) /*&& (instancingIter + 1 != mInstancingInfo.cend())*/)
			{
				currOffset = 0;

				currElement.InstanceDataStepRate = currInstancesPerData;

				if (instancingIter + 1 == mInstancingInfo.cend())
				{
					currInstancingElementIndex = (std::numeric_limits<U32>::max)();
				}
				else
				{
					std::tie(currInstancingElementIndex, currInstancesPerData) = *(++instancingIter); /// retrieve next division's info
				}
			}

			currElement.SemanticName = CD3D11Mappings::GetSemanticTypeName((*iter).mSemanticType);
			currElement.SemanticIndex = usedSemanticIndex[(*iter).mSemanticType]++;
			currElement.Format = CD3D11Mappings::GetDXGIFormat((*iter).mFormatType);
			currElement.InputSlot = (*iter).mSource;
			currElement.AlignedByteOffset = currOffset;
			currElement.InputSlotClass = (*iter).mIsPerInstanceData ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;

			elements.push_back(currElement);

			currOffset += CD3D11Mappings::GetFormatSize((*iter).mFormatType);
		}

		auto&& vsBytecodeDesc = pD3D11Shader->GetVertexShaderBytecode();

		if (FAILED(p3dDevice->CreateInputLayout(&elements[0], static_cast<UINT>(elements.size()), vsBytecodeDesc.data(), vsBytecodeDesc.size(), &pInputLayout)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11InputLayout*>(pInputLayout);
	}

	void CD3D11VertexDeclaration::Bind(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray, IShader* pShader)
	{
		if (!mpInputLayout)
		{
			mpInputLayout = GetInputLayoutByShader(pGraphicsContext, pShader).Get();
		}

		ID3D11DeviceContext* p3dDeviceContext = nullptr;

#if _HAS_CXX17
		p3dDeviceContext = std::get<TD3D11CtxInternalData>(pGraphicsContext->GetInternalData()).mp3dDeviceContext;
#else
		p3dDeviceContext = pGraphicsContext->GetInternalData().mD3D11.mp3dDeviceContext;
#endif

		p3dDeviceContext->IASetInputLayout(mpInputLayout);
	}


	IVertexDeclaration* CreateD3D11VertexDeclaration(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IVertexDeclaration, CD3D11VertexDeclaration, result);
	}


	/*!
		\brief CD3D11GraphicsPipeline's edfinition
	*/

	/*!
		class CD3D11GraphicsPipeline
	*/

	class CD3D11GraphicsPipeline : public CBaseGraphicsPipeline
	{
		public:
			friend IGraphicsPipeline* CreateD3D11GraphicsPipeline(IGraphicsContext*, const TGraphicsPipelineConfigDesc&, E_RESULT_CODE&);
		public:
			E_RESULT_CODE Bind() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11GraphicsPipeline)
		private:
			CD3D11GraphicsObjectManager* mpD3D11GraphicsObjectManagerImpl = nullptr;

			TBlendStateId                mBlendStateHandle = TBlendStateId::Invalid;
			TDepthStencilStateId         mDepthStencilStateHandle = TDepthStencilStateId::Invalid;
			TRasterizerStateId           mRasterizerStateHandle = TRasterizerStateId::Invalid;
	};

	CD3D11GraphicsPipeline::CD3D11GraphicsPipeline():
		CBaseGraphicsPipeline()
	{
	}

	E_RESULT_CODE CD3D11GraphicsPipeline::Bind()
	{
		if (!mpGraphicsObjectManager)
		{
			return RC_FAIL;
		}

		if (!mpD3D11GraphicsObjectManagerImpl)
		{
			mpD3D11GraphicsObjectManagerImpl = dynamic_cast<CD3D11GraphicsObjectManager*>(mpGraphicsObjectManager);
		}

		if (mBlendStateHandle == TBlendStateId::Invalid)
		{
			mBlendStateHandle = mpD3D11GraphicsObjectManagerImpl->CreateBlendState(mConfig.mBlendStateParams).Get();
		}

		mpGraphicsContext->BindBlendState(mBlendStateHandle);

		if (mDepthStencilStateHandle == TDepthStencilStateId::Invalid)
		{
			mDepthStencilStateHandle = mpD3D11GraphicsObjectManagerImpl->CreateDepthStencilState(mConfig.mDepthStencilStateParams).Get();
		}

		mpGraphicsContext->BindDepthStencilState(mDepthStencilStateHandle, mConfig.mDepthStencilStateParams.mStencilRefValue);

		if (mRasterizerStateHandle == TRasterizerStateId::Invalid)
		{
			mRasterizerStateHandle = mpD3D11GraphicsObjectManagerImpl->CreateRasterizerState(mConfig.mRasterizerStateParams).Get();
		}

		mpGraphicsContext->BindRasterizerState(mRasterizerStateHandle);

		return RC_OK;
	}


	IGraphicsPipeline* CreateD3D11GraphicsPipeline(IGraphicsContext* pGraphicsContext, const TGraphicsPipelineConfigDesc& config, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsPipeline, CD3D11GraphicsPipeline, result, pGraphicsContext, config);
	}
}

#endif