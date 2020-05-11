#include "./../../include/graphics/CBaseMaterial.h"
#include "./../../include/graphics/CBaseShader.h"
#include "./../../include/graphics/ITexture.h"
#include "./../../include/graphics/IShaderCompiler.h"
#include "./../../include/graphics/IGraphicsObjectManager.h"
#include "./../../include/core/IGraphicsContext.h"
#include "./../../include/core/IResourceManager.h"
#include "./../../include/core/IFileSystem.h"
#include "./../../include/core/IResourceHandler.h"
#include "./../../include/platform/CBinaryFileReader.h"
#include "./../../include/utils/CFileLogger.h"
#include "./../../include/utils/Utils.h"
#include <cstring>


namespace TDEngine2
{
	/*!
		\note The declaration of TMaterialParameters is placed at IMaterial.h
	*/

	TMaterialParameters::TMaterialParameters(const std::string& shaderName, bool isTransparent, const TDepthStencilStateDesc& depthStencilState,
											 const TRasterizerStateDesc& rasterizerState, const TBlendStateDesc& blendState):
		mShaderName(shaderName), mBlendingParams(blendState), mDepthStencilParams(depthStencilState), mRasterizerParams(rasterizerState)
	{
		mBlendingParams.mIsEnabled = isTransparent;
	}


	CBaseMaterial::CBaseMaterial() :
		CBaseResource()
	{
	}

	E_RESULT_CODE CBaseMaterial::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name)
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
		mpGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();
		
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseMaterial::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		const IResourceLoader* pResourceLoader = mpResourceManager->GetResourceLoader<CBaseMaterial>();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = pResourceLoader->LoadResource(this);

		if (result != RC_OK)
		{
			mState = RST_PENDING;

			return result;
		}

		mState = RST_LOADED;

		return result;
	}

	E_RESULT_CODE CBaseMaterial::Unload()
	{
		return Reset();
	}

	E_RESULT_CODE CBaseMaterial::Reset()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	IMaterialInstance* CBaseMaterial::CreateInstance()
	{
		if (!mIsInitialized)
		{
			return nullptr;
		}
		
		if (auto newInstanceResult = _allocateNewInstance())
		{
			TMaterialInstanceId instanceId = newInstanceResult.Get();
			TDE2_ASSERT(instanceId != DefaultMaterialInstanceId && instanceId != InvalidMaterialInstanceId);

			LOG_MESSAGE(CStringUtils::Format("[Base Material] A new instance {1} of material {0} was created", mName, instanceId));

			// \note Allocate uniform buffers for the instance
			auto&& defaultInstanceUserUniformBuffers = mpInstancesUserUniformBuffers[DefaultMaterialInstanceId].Get();

			TUserUniformsArray newInstanceUniformBuffers;

			for (U8 i = 0; i < MaxNumberOfUserConstantBuffers; ++i)
			{
				newInstanceUniformBuffers[i].resize(defaultInstanceUserUniformBuffers[i].size());
			}

			if (mpInstancesUserUniformBuffers.Add(newInstanceUniformBuffers) != instanceId)
			{
				TDE2_ASSERT(false);
				return nullptr;
			}

			return mpInstancesArray[instanceId].Get();
		}

		return nullptr;
	}

	void CBaseMaterial::SetShader(const std::string& shaderName)
	{
		//mpShader = mpResourceManager->Create<CBaseShader>(&shaderParams);
		IResourceHandler* pNewShaderHandler = mpResourceManager->Load<CBaseShader>(shaderName); /// \todo replace it with Create and load only on demand within Load method
		if (pNewShaderHandler == mpShader)
		{
			return;
		}

		mpShader = pNewShaderHandler;

		PANIC_ON_FAILURE(_initDefaultInstance(*mpShader->Get<CBaseShader>(RAT_BLOCKING)->GetShaderMetaData()));
	}

	void CBaseMaterial::SetTransparentState(bool isTransparent)
	{
		mBlendStateParams.mIsEnabled = isTransparent;
	}

	void CBaseMaterial::SetBlendFactors(const E_BLEND_FACTOR_VALUE& srcFactor, const E_BLEND_FACTOR_VALUE& destFactor,
										const E_BLEND_FACTOR_VALUE& srcAlphaFactor, const E_BLEND_FACTOR_VALUE& destAlphaFactor)
	{
		mBlendStateParams.mScrValue       = srcFactor;
		mBlendStateParams.mDestValue      = destFactor;
		mBlendStateParams.mScrAlphaValue  = srcAlphaFactor;
		mBlendStateParams.mDestAlphaValue = destAlphaFactor;
	}

	void CBaseMaterial::SetBlendOp(const E_BLEND_OP_TYPE& opType, const E_BLEND_OP_TYPE& alphaOpType)
	{
		mBlendStateParams.mOpType      = opType;
		mBlendStateParams.mAlphaOpType = alphaOpType;
	}

	void CBaseMaterial::Bind(TMaterialInstanceId instanceId)
	{
		IShader* pShaderInstance = mpShader->Get<IShader>(TDEngine2::RAT_BLOCKING);

		if (!pShaderInstance || (instanceId == InvalidMaterialInstanceId))
		{
			return;
		}

		if (mBlendStateHandle == TBlendStateId::Invalid)
		{
			mBlendStateHandle = mpGraphicsObjectManager->CreateBlendState(mBlendStateParams).Get();
		}

		mpGraphicsContext->BindBlendState(mBlendStateHandle);

		if (mDepthStencilStateHandle == TDepthStencilStateId::Invalid)
		{
			mDepthStencilStateHandle = mpGraphicsObjectManager->CreateDepthStencilState(mDepthStencilStateParams).Get();
		}

		mpGraphicsContext->BindDepthStencilState(mDepthStencilStateHandle);

		if (mRasterizerStateHandle == TRasterizerStateId::Invalid)
		{
			mRasterizerStateHandle = mpGraphicsObjectManager->CreateRasterizerState(mRasterizerStateParams).Get();
		}

		mpGraphicsContext->BindRasterizerState(mRasterizerStateHandle);

		if (auto userUniformBuffersResult = mpInstancesUserUniformBuffers[instanceId])
		{
			auto&& instanceUniformBuffers = userUniformBuffersResult.Get();

			U8 userUniformBufferId = 0;
			for (const auto& currUserDataBuffer : instanceUniformBuffers)
			{
				if (!currUserDataBuffer.size())
				{
					continue;
				}

				PANIC_ON_FAILURE(pShaderInstance->SetUserUniformsBuffer(userUniformBufferId++, &currUserDataBuffer[0], currUserDataBuffer.size()));
			}
		}

		pShaderInstance->Bind();

		for (auto iter = mpAssignedTextures.cbegin(); iter != mpAssignedTextures.cend(); ++iter)
		{
			pShaderInstance->SetTextureResource(iter->first, iter->second);
		}
	}

	E_RESULT_CODE CBaseMaterial::SetTextureResource(const std::string& resourceName, ITexture* pTexture)
	{
		if (resourceName.empty() || !pTexture)
		{
			return RC_INVALID_ARGS;
		}

		mpAssignedTextures[resourceName] = pTexture;

		return RC_OK;
	}

	void CBaseMaterial::SetDepthBufferEnabled(bool state)
	{
		mDepthStencilStateParams.mIsDepthTestEnabled = state;
	}

	void CBaseMaterial::SetStencilBufferEnabled(bool state)
	{
		mDepthStencilStateParams.mIsStencilTestEnabled = state;
	}

	void CBaseMaterial::SetDepthWriteEnabled(bool state)
	{
		mDepthStencilStateParams.mIsDepthWritingEnabled;
	}

	void CBaseMaterial::SetDepthComparisonFunc(const E_COMPARISON_FUNC& funcType)
	{
		mDepthStencilStateParams.mDepthCmpFunc = funcType;
	}

	void CBaseMaterial::SetCullMode(const E_CULL_MODE& cullMode)
	{
		mRasterizerStateParams.mCullMode = cullMode;
	}

	void CBaseMaterial::SetScissorEnabled(bool state)
	{
		mRasterizerStateParams.mIsScissorTestEnabled = state;
	}

	void CBaseMaterial::SetWireframeMode(bool state)
	{
		mRasterizerStateParams.mIsWireframeModeEnabled = state;
	}

	void CBaseMaterial::SetGeometrySubGroupTag(const E_GEOMETRY_SUBGROUP_TAGS& tag)
	{
		mTag = tag;
	}

	U32 CBaseMaterial::GetVariableHash(const std::string& name) const
	{
		return TDE2_STRING_ID(name.c_str());
	}

	IResourceHandler* CBaseMaterial::GetShaderHandler() const
	{
		return mpShader;
	}

	bool CBaseMaterial::IsTransparent() const
	{
		return mBlendStateParams.mIsEnabled;
	}

	const E_GEOMETRY_SUBGROUP_TAGS& CBaseMaterial::GetGeometrySubGroupTag() const
	{
		return mTag;
	}

	TResult<IMaterialInstance*> CBaseMaterial::GetMaterialInstance(TMaterialInstanceId instanceId) const
	{
		return mpInstancesArray[instanceId];
	}

	TResult<IMaterialInstance*> CBaseMaterial::_setVariable(const std::string& name, const void* pValue, U32 size)
	{
		E_RESULT_CODE result = RC_FAIL;

		IMaterialInstance* pNewMaterialInstance = CreateInstance();

		if (!pNewMaterialInstance || (result = _setVariable(pNewMaterialInstance->GetInstanceId(), name, pValue, size)) != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		return TOkValue<IMaterialInstance*>(pNewMaterialInstance);
	}

	E_RESULT_CODE CBaseMaterial::_setVariable(TMaterialInstanceId instanceId, const std::string& name, const void* pValue, U32 size)
	{
		U32 variableHash = GetVariableHash(name);

		auto&& iter = mUserVariablesHashTable.find(variableHash);
		if (iter == mUserVariablesHashTable.cend())
		{
			LOG_ERROR(CStringUtils::Format("[Base Material] There is no a variable with corresponding name ({0})", name));
			return RC_FAIL;
		}

		U32 bufferIndex = 0;
		U32 varOffset = 0;
		std::tie(bufferIndex, varOffset) = iter->second; // first index is a buffer's id, the second one is variable's offset in bytes

		if (auto userUniformBuffersResult = mpInstancesUserUniformBuffers[instanceId])
		{
			auto&& instanceUniformBuffers = userUniformBuffersResult.Get();
		
			TDE2_ASSERT((instanceUniformBuffers[bufferIndex].size() - varOffset) > size);
			memcpy(&instanceUniformBuffers[bufferIndex][varOffset], pValue, size);
		}

		return RC_OK;
	}

	E_RESULT_CODE CBaseMaterial::_allocateUserDataBuffers(const TShaderCompilerOutput& metadata)
	{
		mUserVariablesHashTable.clear();
		
		TUserUniformsArray defaultInstanceUserUniforms;

		U32 slotIndex = 0;

		// \note assume that this code was invoked from SetShader, so we need to reallocate user uniform buffers
		// also regenerate hash table of user uniforms
		for (const auto& currEntryDesc : metadata.mUniformBuffersInfo)
		{
			const auto& currUniformBufferDesc = currEntryDesc.second;

			if (currUniformBufferDesc.mFlags == E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL)
			{
				continue;
			}

			slotIndex = currUniformBufferDesc.mSlot - TotalNumberOfInternalConstantBuffers;
			TDE2_ASSERT((slotIndex >= 0) && (currUniformBufferDesc.mSize >= 0));

			defaultInstanceUserUniforms[slotIndex].resize(currUniformBufferDesc.mSize);

			U32 variableBytesOffset = 0;

			for (const auto& currVariableDesc : currUniformBufferDesc.mVariables)
			{
				mUserVariablesHashTable[TDE2_STRING_ID(currVariableDesc.mName.c_str())] = { slotIndex, variableBytesOffset };
				variableBytesOffset += currVariableDesc.mSize;
			}
		}		

		// \note At this moment mpInstancesUserUniformBuffers should be empty
		TDE2_ASSERT(mpInstancesUserUniformBuffers.Add(defaultInstanceUserUniforms) == 0);

		return RC_OK;
	}

	TDE2_API TResult<TMaterialInstanceId> CBaseMaterial::_allocateNewInstance()
	{
		E_RESULT_CODE result = RC_OK;

		TMaterialInstanceId instanceId = mpInstancesArray.Add(nullptr);

		IMaterialInstance* pNewMaterialInstance = CreateBaseMaterialInstance(this, instanceId, result);

		if ((result != RC_OK) || (result = mpInstancesArray.ReplaceAt(instanceId, pNewMaterialInstance)) != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		return TOkValue<TMaterialInstanceId>(instanceId);
	}

	E_RESULT_CODE CBaseMaterial::_initDefaultInstance(const TShaderCompilerOutput& metadata)
	{
		mpInstancesArray.RemoveAll();
		mpInstancesUserUniformBuffers.RemoveAll();
		
		E_RESULT_CODE result = RC_OK;

		if ((result = _allocateUserDataBuffers(metadata)) != RC_OK)
		{
			return result;
		}

		if (auto defaultInstanceResult = _allocateNewInstance())
		{
			TDE2_ASSERT(defaultInstanceResult.Get() == DefaultMaterialInstanceId);
			if (defaultInstanceResult.Get() != DefaultMaterialInstanceId)
			{
				return RC_FAIL;
			}
		}

		return result;
	}


	bool CBaseMaterial::AlphaBasedMaterialComparator(const IMaterial* pLeft, const IMaterial* pRight)
	{
		return (!pLeft || pRight->IsTransparent()) || (pRight && !pLeft->IsTransparent());
	}


	TDE2_API IMaterial* CreateBaseMaterial(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		CBaseMaterial* pMaterialInstance = new (std::nothrow) CBaseMaterial();

		if (!pMaterialInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pMaterialInstance->Init(pResourceManager, pGraphicsContext, name);

		if (result != RC_OK)
		{
			delete pMaterialInstance;

			pMaterialInstance = nullptr;
		}

		return pMaterialInstance;
	}


	TDE2_API IMaterial* CreateBaseMaterial(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
										   const TMaterialParameters& params, E_RESULT_CODE& result)
	{
		CBaseMaterial* pMaterialInstance = new (std::nothrow) CBaseMaterial();

		if (!pMaterialInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pMaterialInstance->Init(pResourceManager, pGraphicsContext, name);
		
		if (result != RC_OK)
		{
			delete pMaterialInstance;

			pMaterialInstance = nullptr;
		}
		else
		{
			pMaterialInstance->SetShader(params.mShaderName);

			// \note blending group
			auto&& blendingParams = params.mBlendingParams;

			pMaterialInstance->SetTransparentState(blendingParams.mIsEnabled);
			pMaterialInstance->SetBlendFactors(blendingParams.mScrValue, blendingParams.mDestValue, 
											   blendingParams.mScrAlphaValue, blendingParams.mDestAlphaValue);
			pMaterialInstance->SetBlendOp(blendingParams.mOpType, blendingParams.mAlphaOpType);

			// \note depth-stencil group of parameters
			auto&& depthStencilParams = params.mDepthStencilParams;

			pMaterialInstance->SetDepthBufferEnabled(depthStencilParams.mIsDepthTestEnabled);
			pMaterialInstance->SetDepthWriteEnabled(depthStencilParams.mIsDepthWritingEnabled);
			pMaterialInstance->SetDepthComparisonFunc(depthStencilParams.mDepthCmpFunc);
			pMaterialInstance->SetStencilBufferEnabled(depthStencilParams.mIsStencilTestEnabled);

			// \note rasterizer state parameters
			auto&& rasterizerStateParams = params.mRasterizerParams;

			pMaterialInstance->SetCullMode(rasterizerStateParams.mCullMode);
			pMaterialInstance->SetWireframeMode(rasterizerStateParams.mIsWireframeModeEnabled);
			pMaterialInstance->SetScissorEnabled(rasterizerStateParams.mIsScissorTestEnabled);
		}

		return pMaterialInstance;
	}
	

	CBaseMaterialLoader::CBaseMaterialLoader() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseMaterialLoader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || !pGraphicsContext || !pFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mpFileSystem = pFileSystem;

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseMaterialLoader::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CBaseMaterialLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		TResult<TFileEntryId> materialFileId = mpFileSystem->Open<CBinaryFileReader>(pResource->GetName());

		if (materialFileId.HasError())
		{
			return materialFileId.GetError();
		}

		IBinaryFileReader* pMaterialFile = dynamic_cast<IBinaryFileReader*>(mpFileSystem->Get<CBinaryFileReader>(materialFileId.Get()));
		
		/// try to read the file's header
		TBaseMaterialFileHeader header = _readMaterialFileHeader(pMaterialFile).Get();

		if ((result = pMaterialFile->Close()) != RC_OK)
		{
			return result;
		}

		return RC_OK;
	}

	U32 CBaseMaterialLoader::GetResourceTypeId() const
	{
		return CBaseMaterial::GetTypeId();
	}

	TResult<TBaseMaterialFileHeader> CBaseMaterialLoader::_readMaterialFileHeader(IBinaryFileReader* pFileReader) const
	{
		TBaseMaterialFileHeader header;

		memset(&header, 0, sizeof(TBaseMaterialFileHeader));

		E_RESULT_CODE result = RC_OK;

		/// read the file's header
		if ((result = pFileReader->Read(static_cast<void*>(header.mTag), sizeof(C8) * BaseMaterialFileTagLength)) != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		if (strncmp(header.mTag, "MAT", BaseMaterialFileTagLength))
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		/// read endianness of the file
		if ((result = pFileReader->Read(static_cast<void*>(&header.mEndianType), sizeof(U8))) != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		/// read offsets' values of data blocks
		if ((result = pFileReader->Read(static_cast<void*>(&header.mShaderEntriesBlockOffset), sizeof(U8))) != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		if ((result = pFileReader->Read(static_cast<void*>(&header.mMaterialPropertiesBlockOffset), sizeof(U8))) != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		return TOkValue<TBaseMaterialFileHeader>(header);
	}


	TDE2_API IResourceLoader* CreateBaseMaterialLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		CBaseMaterialLoader* pMaterialLoaderInstance = new (std::nothrow) CBaseMaterialLoader();

		if (!pMaterialLoaderInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pMaterialLoaderInstance->Init(pResourceManager, pGraphicsContext, pFileSystem);

		if (result != RC_OK)
		{
			delete pMaterialLoaderInstance;

			pMaterialLoaderInstance = nullptr;
		}

		return pMaterialLoaderInstance;
	}


	CBaseMaterialFactory::CBaseMaterialFactory():
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CBaseMaterialFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	E_RESULT_CODE CBaseMaterialFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IResource* CBaseMaterialFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TMaterialParameters& matParams = dynamic_cast<const TMaterialParameters&>(params);

		return dynamic_cast<IResource*>(CreateBaseMaterial(mpResourceManager, mpGraphicsContext, name, matParams, result));
	}

	IResource* CBaseMaterialFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateBaseMaterial(mpResourceManager, mpGraphicsContext, name, result));
	}

	U32 CBaseMaterialFactory::GetResourceTypeId() const
	{
		return CBaseMaterial::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateBaseMaterialFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		CBaseMaterialFactory* pMaterialFactoryInstance = new (std::nothrow) CBaseMaterialFactory();

		if (!pMaterialFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pMaterialFactoryInstance->Init(pResourceManager, pGraphicsContext);

		if (result != RC_OK)
		{
			delete pMaterialFactoryInstance;

			pMaterialFactoryInstance = nullptr;
		}

		return pMaterialFactoryInstance;
	}


	CBaseMaterialInstance::CBaseMaterialInstance():
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseMaterialInstance::Init(IMaterial* pMaterial, TMaterialInstanceId id)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pMaterial)
		{
			return RC_INVALID_ARGS;
		}

		mpSharedMaterial = pMaterial;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseMaterialInstance::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CBaseMaterialInstance::Bind()
	{
		//mpSharedMaterial->Bind(mId);
	}

	TMaterialInstanceId CBaseMaterialInstance::GetInstanceId() const
	{
		return mId;
	}


	TDE2_API IMaterialInstance* CreateBaseMaterialInstance(IMaterial* pMaterial, TMaterialInstanceId id, E_RESULT_CODE& result)
	{
		CBaseMaterialInstance* pMaterialInstance = new (std::nothrow) CBaseMaterialInstance();

		if (!pMaterialInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pMaterialInstance->Init(pMaterial, id);

		if (result != RC_OK)
		{
			delete pMaterialInstance;

			pMaterialInstance = nullptr;
		}

		return pMaterialInstance;
	}
}