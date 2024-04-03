#include "../../include/graphics/CBaseShader.h"
#include "../../include/graphics/IShaderCompiler.h"
#include "../../include/graphics/ITexture.h"
#include "../../include/graphics/IBuffer.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/CBaseShaderCompiler.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/core/IGraphicsContext.h"
#include <algorithm>


namespace TDEngine2
{
	/*!
		\brief CBaseShader's definition
	*/


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
		TDE2_PROFILER_SCOPE("CBaseShader::Compile");

		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pShaderCompiler)
		{
			return RC_INVALID_ARGS;
		}

		mSourceCode = sourceCode;

		TResult<TShaderCompilerOutput*> compilerOutput = pShaderCompiler->Compile(mName, sourceCode);

		if (compilerOutput.HasError())
		{
			return compilerOutput.GetError();
		}

		TShaderCompilerOutput* pCompilerOutput = compilerOutput.Get();

		return _initShaderInternal(pCompilerOutput);
	}

	E_RESULT_CODE CBaseShader::LoadFromShaderCache(IShaderCache* pShaderCache)
	{
		TDE2_PROFILER_SCOPE("CBaseShader::LoadFromShaderCache");
		return _initShaderInternal(pShaderCache->GetShaderMetaData(mName));
	}

	E_RESULT_CODE CBaseShader::SetUserUniformsBuffer(U8 slot, const U8* pData, USIZE dataSize)
	{
		if (slot >= MaxNumberOfUserConstantBuffers)
		{
			return RC_INVALID_ARGS;
		}

		/// \todo add checking for sizes of input data
		auto pCurrUniformBuffer = mpGraphicsContext->GetGraphicsObjectManager()->GetBufferPtr(mUniformBuffers[slot]);
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
		mpGraphicsContext->SetConstantBuffer(TotalNumberOfInternalConstantBuffers + slot, mUniformBuffers[slot]);

		return RC_OK;
	}

	void CBaseShader::Bind()
	{
		ITexture* pCurrTexture = nullptr;
		U8 currSlot = 0;
		bool isWritable = false;

		/// \note Bind textures
		for (U32 i = 0; i < mpTextures.size(); ++i)
		{
			std::tie(pCurrTexture, currSlot, isWritable) = mpTextures[i];
			if (!pCurrTexture)
			{
				continue;
			}

			pCurrTexture->SetWriteable(isWritable);
			pCurrTexture->Bind(currSlot);
		}

#if 0
		/// \note Binds structured buffers
		IStructuredBuffer* pCurrBuffer = nullptr;

		for (U32 i = 0; i < mpBuffers.size(); ++i)
		{
			pCurrBuffer = mpBuffers[i];

			if (!pCurrBuffer)
			{
				continue;
			}

			pCurrBuffer->Bind(i);
		}
#endif

		U8 currUserBufferId = 0;
		for (auto& currUniformBufferHandle : mUniformBuffers)
		{
			_bindUniformBuffer(TotalNumberOfInternalConstantBuffers + currUserBufferId++, currUniformBufferHandle);
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

		std::get<ITexture*>(mpTextures[hashIter->second]) = pTexture;

		return RC_OK;
	}

	E_RESULT_CODE CBaseShader::SetStructuredBufferResource(const std::string& resourceName, TBufferHandleId bufferHandle)
	{
		if (resourceName.empty() || TBufferHandleId::Invalid == bufferHandle)
		{
			return RC_INVALID_ARGS;
		}

		auto hashIter = mStructuredBuffersHashTable.find(resourceName);
		if (hashIter == mStructuredBuffersHashTable.cend())
		{
			return RC_FAIL;
		}

		//mBufferHandles[std::get<0>(hashIter->second)] = bufferHandle;

		return RC_OK;
	}

	const TShaderCompilerOutput* CBaseShader::GetShaderMetaData() const
	{
		return mpShaderMeta;
	}

	E_RESULT_CODE CBaseShader::_initShaderInternal(TShaderCompilerOutput* pShaderMetaData)
	{
		mpShaderMeta = pShaderMetaData;

		E_RESULT_CODE result = _createInternalHandlers(mpShaderMeta); /// reimplement this method in a derived class to do some extra work

		if (result != RC_OK)
		{
			return result;
		}

		result = result | _createTexturesHashTable(mpShaderMeta);
		result = result | _createStructuredBuffersHashTable(mpShaderMeta);

		return result;
	}


	static bool IsTextureShaderResource(E_SHADER_RESOURCE_TYPE type)
	{
		return E_SHADER_RESOURCE_TYPE::SRT_SAMPLER_STATE != type
			&& E_SHADER_RESOURCE_TYPE::SRT_RW_STRUCTURED_BUFFER != type
			&& E_SHADER_RESOURCE_TYPE::SRT_STRUCTURED_BUFFER != type;
	}


	E_RESULT_CODE CBaseShader::_createTexturesHashTable(const TShaderCompilerOutput* pCompilerData)
	{
		auto shaderResourcesMap = pCompilerData->mShaderResourcesInfo;
		if (shaderResourcesMap.empty())
		{
			return RC_OK;
		}

		mpTextures.clear();

		USIZE currSlotIndex = 0;
		
		for (auto currShaderResourceInfo : shaderResourcesMap)
		{
			const TShaderResourceDesc& desc = currShaderResourceInfo.second;

			if (!IsTextureShaderResource(desc.mType))
			{
				continue;
			}

			currSlotIndex = mpTextures.size();

			mTexturesHashTable[currShaderResourceInfo.first] = currSlotIndex;
			mpTextures.emplace_back(std::make_tuple(nullptr, desc.mSlot, desc.mIsWriteable));
		}

		return RC_OK;
	}

	E_RESULT_CODE CBaseShader::_createStructuredBuffersHashTable(const TShaderCompilerOutput* pCompilerData)
	{
		auto shaderResourcesMap = pCompilerData->mShaderResourcesInfo;

		if (shaderResourcesMap.empty())
		{
			return RC_OK;
		}

		mBufferHandles.clear();

		USIZE currSlotIndex = 0;

		for (auto currShaderResourceInfo : shaderResourcesMap)
		{
			const TShaderResourceDesc& desc = currShaderResourceInfo.second;

			if (IsTextureShaderResource(desc.mType))
			{
				continue;
			}

			currSlotIndex = mBufferHandles.size();

			mStructuredBuffersHashTable[currShaderResourceInfo.first] = currSlotIndex;
			mBufferHandles.emplace_back(std::make_tuple(TBufferHandleId::Invalid, desc.mSlot, desc.mIsWriteable));
		}

		return RC_OK;
	}

	void CBaseShader::_bindUniformBuffer(U32 slot, TBufferHandleId uniformsBufferHandle)
	{
		mpGraphicsContext->SetConstantBuffer(slot, uniformsBufferHandle);
	}

	const TPtr<IResourceLoader> CBaseShader::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IShader>();
	}


	/*!
		\brief CShaderCache's definition
	*/


	TDE2_STATIC_CONSTEXPR U32 ShaderCacheTag = 'SDRC';
	TDE2_STATIC_CONSTEXPR U32 ShaderEntryTag = 'SDEN';
	TDE2_STATIC_CONSTEXPR U32 UniformsBlockTag = 'UB';
	TDE2_STATIC_CONSTEXPR U32 ResourcesBlockTag = 'RB';
	TDE2_STATIC_CONSTEXPR U32 StagesBlockTag = 'SB';


	CShaderCache::CShaderCache():
		CBaseObject()
	{
	}

	E_RESULT_CODE CShaderCache::Init(IBinaryFileReader* pCacheReader, IBinaryFileWriter* pCacheWriter)
	{
		if (!pCacheReader)
		{
			return RC_INVALID_ARGS;
		}

		mpCacheFileReader = pCacheReader;
		mpCacheFileWriter = pCacheWriter;

		mShadersInfoTable.clear();

		E_RESULT_CODE result = _readShadersMetaTable();
		if (RC_OK != result)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}


	static void CloneShaderCompilerData(TShaderCompilerOutput* pDest, const TShaderCompilerOutput* pSource)
	{
		for (auto&& currShaderResourceInfo : pSource->mShaderResourcesInfo)
		{
			pDest->mShaderResourcesInfo.emplace(currShaderResourceInfo.first, currShaderResourceInfo.second);
		}

		for (auto&& currUniformBufferInfo : pSource->mUniformBuffersInfo)
		{
			pDest->mUniformBuffersInfo.emplace(currUniformBufferInfo.first, currUniformBufferInfo.second);
		}

		for (auto&& currStageInfo : pSource->mStagesInfo)
		{
			TShaderStageInfoDesc stageInfo;

			stageInfo.mEntrypointName = currStageInfo.second.mEntrypointName;

			auto& bytecode = currStageInfo.second.mBytecode;
			std::copy(bytecode.begin(), bytecode.end(), std::back_inserter(stageInfo.mBytecode));

			pDest->mStagesInfo.emplace(currStageInfo.first, stageInfo);
		}
	}


	E_RESULT_CODE CShaderCache::AddShaderEntity(const std::string& shaderId, const TShaderCompilerOutput* pShaderCompiledData)
	{
		if (shaderId.empty() || !pShaderCompiledData)
		{
			return RC_INVALID_ARGS;
		}

		auto pInternalShaderDataCopy = std::make_unique<TShaderCompilerOutput>();
		CloneShaderCompilerData(pInternalShaderDataCopy.get(), pShaderCompiledData);

		mShadersInfoTable.emplace(shaderId, std::move(pInternalShaderDataCopy));

		mIsDirty = true;

		return RC_OK;
	}

	TShaderCompilerOutput* CShaderCache::GetShaderMetaData(const std::string& shaderId)
	{
		auto it = mShadersInfoTable.find(shaderId);
		
		auto pShaderMetadata = it == mShadersInfoTable.cend() ? nullptr : it->second.get();
		if (!pShaderMetadata)
		{
			return nullptr;
		}

		TShaderCompilerOutput* pShaderMetadataCopy = new(std::nothrow) TShaderCompilerOutput();
		if (!pShaderMetadataCopy)
		{
			return nullptr;
		}

		CloneShaderCompilerData(pShaderMetadataCopy, pShaderMetadata);

		return pShaderMetadataCopy;
	}

	bool CShaderCache::HasShaderMetaData(const std::string& shaderId) const
	{
		return mShadersInfoTable.find(shaderId) != mShadersInfoTable.cend();
	}


	struct TShaderCacheBytecodeEntry
	{
		USIZE mOffset = 0; ///< start position of the bytecode within the shader cache blob
		USIZE mSize = 0; ///< size of the bytecode
	};


	struct TShaderStageDesc
	{
		TShaderCacheBytecodeEntry mBytecode;
		std::string mEntrypoint;
	};


	struct TShaderDataEntry
	{
		std::string                                               mName;
		std::unordered_map<std::string, TUniformBufferDesc>       mUniformBuffersInfo;		/// first key is a buffer's name, the value is the buffer's slot index and its size
		std::unordered_map<std::string, TShaderResourceDesc>      mShaderResourcesInfo;	/// the key is a resource's name, the value is an information about resource
		std::unordered_map<E_SHADER_STAGE_TYPE, TShaderStageDesc> mStages;
	};


	static E_RESULT_CODE ReadUniformsBlockEntry(IBinaryFileReader* pFileReader, TShaderDataEntry& entry)
	{
		if (UniformsBlockTag != pFileReader->ReadUInt32())
		{
			TDE2_ASSERT(false);
			return RC_FAIL;
		}

		const U32 uniformBlocksCount = pFileReader->ReadUInt32();		
		for (U32 i = 0; i < uniformBlocksCount; i++)
		{
			std::string bufferName = pFileReader->ReadString();

			TUniformBufferDesc uniformBufferDesc;

			uniformBufferDesc.mSlot = pFileReader->ReadUInt8();
			uniformBufferDesc.mSize = static_cast<USIZE>(pFileReader->ReadUInt32());
			uniformBufferDesc.mFlags = static_cast<E_UNIFORM_BUFFER_DESC_FLAGS>(pFileReader->ReadUInt32());
			uniformBufferDesc.mBufferIndex = pFileReader->ReadUInt32();

			const U32 variablesCount = pFileReader->ReadUInt32();

			for (U32 currVariableIndex = 0; currVariableIndex < variablesCount; currVariableIndex++)
			{
				TShaderUniformDesc variableDesc;

				variableDesc.mName = pFileReader->ReadString();
				variableDesc.mOffset = static_cast<USIZE>(pFileReader->ReadUInt32());
				variableDesc.mSize = static_cast<USIZE>(pFileReader->ReadUInt32());
				variableDesc.mTypeId = static_cast<TypeId>(pFileReader->ReadUInt32());
				variableDesc.mIsArray = pFileReader->ReadBool();

				uniformBufferDesc.mVariables.emplace_back(variableDesc);
			}

			entry.mUniformBuffersInfo.emplace(bufferName, uniformBufferDesc);
		}

		return RC_OK;
	}


	static E_RESULT_CODE ReadResourcesBlockEntry(IBinaryFileReader* pFileReader, TShaderDataEntry& entry)
	{
		if (ResourcesBlockTag != pFileReader->ReadUInt32())
		{
			TDE2_ASSERT(false);
			return RC_FAIL;
		}

		const U32 resourcesBlocksCount = pFileReader->ReadUInt32();
		for (U32 i = 0; i < resourcesBlocksCount; i++)
		{
			std::string resourceName = pFileReader->ReadString();

			TShaderResourceDesc desc;
			desc.mSlot = pFileReader->ReadUInt8();
			desc.mType = static_cast<E_SHADER_RESOURCE_TYPE>(pFileReader->ReadUInt32());
			desc.mIsWriteable = 
				E_SHADER_RESOURCE_TYPE::SRT_RW_STRUCTURED_BUFFER == desc.mType || 
				E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE2D == desc.mType || 
				E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE3D == desc.mType;

			entry.mShaderResourcesInfo.emplace(resourceName, desc);
		}

		return RC_OK;
	}


	static E_RESULT_CODE ReadStagesBlockEntry(IBinaryFileReader* pFileReader, TShaderDataEntry& entry)
	{
		if (StagesBlockTag != pFileReader->ReadUInt32())
		{
			TDE2_ASSERT(false);
			return RC_FAIL;
		}

		const U32 stagesBlocksCount = pFileReader->ReadUInt32();
		for (U32 i = 0; i < stagesBlocksCount; i++)
		{
			const E_SHADER_STAGE_TYPE stage = static_cast<E_SHADER_STAGE_TYPE>(pFileReader->ReadUInt32());

			TShaderStageDesc desc;
			desc.mEntrypoint = pFileReader->ReadString();
			desc.mBytecode.mOffset = static_cast<USIZE>(pFileReader->ReadUInt64());
			desc.mBytecode.mSize = static_cast<USIZE>(pFileReader->ReadUInt64());

			entry.mStages.emplace(stage, desc);
		}

		return RC_OK;
	}


	static E_RESULT_CODE ReadShaderEntry(IBinaryFileReader* pFileReader, std::vector<TShaderDataEntry>& shaders)
	{
		TShaderDataEntry newShaderEntry;

		E_RESULT_CODE result = RC_OK;

		newShaderEntry.mName = pFileReader->ReadString();

		result = result | ReadUniformsBlockEntry(pFileReader, newShaderEntry);
		result = result | ReadResourcesBlockEntry(pFileReader, newShaderEntry);
		result = result | ReadStagesBlockEntry(pFileReader, newShaderEntry);

		shaders.emplace_back(newShaderEntry);

		return RC_OK;
	}


	static E_RESULT_CODE WriteUniformsBlock(IBinaryFileWriter* pFileWriter, const TShaderCompilerOutput* pShaderMetadata)
	{
		pFileWriter->WriteUInt32(UniformsBlockTag);
		
		const U32 systemBuffersCount = 
			static_cast<U32>(std::count_if(pShaderMetadata->mUniformBuffersInfo.begin(), pShaderMetadata->mUniformBuffersInfo.end(), [](auto&& uniformBufferEntry)
			{
				return E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL == (uniformBufferEntry.second.mFlags & E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL);
			}));

		pFileWriter->WriteUInt32(static_cast<U32>(pShaderMetadata->mUniformBuffersInfo.size()) - systemBuffersCount);

		for (auto&& currUniformBufferInfo : pShaderMetadata->mUniformBuffersInfo)
		{
			if (E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL == (currUniformBufferInfo.second.mFlags & E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL))
			{
				continue;
			}

			pFileWriter->WriteString(currUniformBufferInfo.first);
			
			auto& uniformBufferDesc = currUniformBufferInfo.second;
			pFileWriter->WriteUInt8(uniformBufferDesc.mSlot);
			pFileWriter->WriteUInt32(static_cast<U32>(uniformBufferDesc.mSize));
			pFileWriter->WriteUInt32(static_cast<U32>(uniformBufferDesc.mFlags));
			pFileWriter->WriteUInt32(uniformBufferDesc.mBufferIndex);

			pFileWriter->WriteUInt32(static_cast<U32>(uniformBufferDesc.mVariables.size()));

			for (auto&& currVariableDesc : uniformBufferDesc.mVariables)
			{
				pFileWriter->WriteString(currVariableDesc.mName);
				pFileWriter->WriteUInt32(static_cast<U32>(currVariableDesc.mOffset));
				pFileWriter->WriteUInt32(static_cast<U32>(currVariableDesc.mSize));
				pFileWriter->WriteUInt32(static_cast<U32>(currVariableDesc.mTypeId));
				pFileWriter->WriteBool(currVariableDesc.mIsArray);
			}
		}

		return RC_OK;
	}


	static E_RESULT_CODE WriteResourcesBlock(IBinaryFileWriter* pFileWriter, const TShaderCompilerOutput* pShaderMetadata)
	{
		pFileWriter->WriteUInt32(ResourcesBlockTag);
		pFileWriter->WriteUInt32(static_cast<U32>(pShaderMetadata->mShaderResourcesInfo.size()));

		for (auto&& currResourceBindingInfo : pShaderMetadata->mShaderResourcesInfo)
		{
			pFileWriter->WriteString(currResourceBindingInfo.first);
			
			pFileWriter->WriteUInt8(currResourceBindingInfo.second.mSlot);
			pFileWriter->WriteUInt32(static_cast<U32>(currResourceBindingInfo.second.mType));
		}

		return RC_OK;
	}

	static E_RESULT_CODE WriteStagesBlock(IBinaryFileWriter* pFileWriter, const TShaderCompilerOutput* pShaderMetadata, std::vector<U8>& shadersBytecode)
	{
		pFileWriter->WriteUInt32(StagesBlockTag);
		pFileWriter->WriteUInt32(static_cast<U32>(pShaderMetadata->mStagesInfo.size()));

		for (auto&& currStageInfo : pShaderMetadata->mStagesInfo)
		{
			pFileWriter->WriteUInt32(static_cast<U32>(currStageInfo.first));
			
			pFileWriter->WriteString(currStageInfo.second.mEntrypointName);
			
			const USIZE currOffset = shadersBytecode.size();
			std::copy(currStageInfo.second.mBytecode.begin(), currStageInfo.second.mBytecode.end(), std::back_inserter(shadersBytecode));

			pFileWriter->WriteUInt64(static_cast<U64>(currOffset));
			pFileWriter->WriteUInt64(static_cast<U64>(currStageInfo.second.mBytecode.size()));
		}

		return RC_OK;
	}


	static E_RESULT_CODE WriteShadersEntriesTable(IBinaryFileWriter* pFileWriter, const CShaderCache::TShadersDataTable& shaders, std::vector<U8>& shadersBytecode)
	{
		E_RESULT_CODE result = RC_OK;

		for (auto&& currShaderEntry : shaders)
		{
			result = result | pFileWriter->WriteUInt32(ShaderEntryTag);
			result = result | pFileWriter->WriteString(currShaderEntry.first);

			result = result | WriteUniformsBlock(pFileWriter, currShaderEntry.second.get());
			result = result | WriteResourcesBlock(pFileWriter, currShaderEntry.second.get());
			result = result | WriteStagesBlock(pFileWriter, currShaderEntry.second.get(), shadersBytecode);
		}

		return result;
	}


	E_RESULT_CODE CShaderCache::Dump()
	{
		if (!mIsDirty)
		{
			return RC_OK;
		}

		if (!mpCacheFileWriter)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = mpCacheFileWriter->SetPosition(0);
		result = result | mpCacheFileWriter->WriteUInt32(ShaderCacheTag);
		result = result | mpCacheFileWriter->WriteUInt32(static_cast<U32>(mShadersInfoTable.size()));
		result = result | mpCacheFileWriter->WriteUInt64(0);

		std::vector<U8> shadersBytecode;

		result = result | WriteShadersEntriesTable(mpCacheFileWriter, mShadersInfoTable, shadersBytecode);
		if (RC_OK != result)
		{
			return result;
		}

		const U64 currFilePosition = static_cast<U64>(mpCacheFileWriter->GetPosition());

		result = result | mpCacheFileWriter->Write(shadersBytecode.data(), shadersBytecode.size());
		mpCacheFileWriter->SetPosition(sizeof(U32) * 2);
		mpCacheFileWriter->WriteUInt64(currFilePosition);

		mIsDirty = false;

		return result;
	}


	static E_RESULT_CODE ConstructShadersMetadataFromEntries(const std::vector<TShaderDataEntry>& shaders, const std::vector<U8>& shadersBytecode, CShaderCache::TShadersDataTable& compiledShadersData)
	{
		for (auto&& currShaderEntry : shaders)
		{
			auto pShaderMetadata = std::make_unique<TShaderCompilerOutput>();

			for (auto&& currShaderResourceInfo : currShaderEntry.mShaderResourcesInfo)
			{
				pShaderMetadata->mShaderResourcesInfo.emplace(currShaderResourceInfo.first, currShaderResourceInfo.second);
			}

			for (auto&& currUniformBufferInfo : CBaseShaderCompiler::GetSystemUniformBuffersTable()) // firstly add system buffers
			{
				pShaderMetadata->mUniformBuffersInfo.emplace(currUniformBufferInfo.first, currUniformBufferInfo.second);
			}

			for (auto&& currUniformBufferInfo : currShaderEntry.mUniformBuffersInfo)
			{
				pShaderMetadata->mUniformBuffersInfo.emplace(currUniformBufferInfo.first, currUniformBufferInfo.second);
			}

			for (auto&& currStageInfo : currShaderEntry.mStages)
			{
				TShaderStageInfoDesc stageInfo;

				stageInfo.mEntrypointName = currStageInfo.second.mEntrypoint;

				stageInfo.mBytecode.resize(static_cast<USIZE>(currStageInfo.second.mBytecode.mSize));
				memcpy(stageInfo.mBytecode.data(), &shadersBytecode[currStageInfo.second.mBytecode.mOffset], stageInfo.mBytecode.size());

				pShaderMetadata->mStagesInfo.emplace(currStageInfo.first, stageInfo);
			}

			compiledShadersData.emplace(currShaderEntry.mName, std::move(pShaderMetadata));
		}

		return RC_OK;
	}


	E_RESULT_CODE CShaderCache::_readShadersMetaTable()
	{
		if (mpCacheFileReader->GetFileLength() < 1)
		{
			return RC_OK;
		}

		if (ShaderCacheTag != mpCacheFileReader->ReadUInt32())
		{
			return RC_INVALID_FILE;
		}

		// \note header
		const U32 shadersCount = mpCacheFileReader->ReadUInt32();
		const U64 shadersBytecodeOffset = mpCacheFileReader->ReadUInt64();

		E_RESULT_CODE result = RC_OK;

		std::vector<TShaderDataEntry> shaders;

		for (U32 i = 0; i < shadersCount; i++)
		{
			if (ShaderEntryTag != mpCacheFileReader->ReadUInt32())
			{
				continue;
			}

			result = result | ReadShaderEntry(mpCacheFileReader, shaders);
		}
		
		std::vector<U8> shadersBytecode;

		result = result | mpCacheFileReader->SetPosition(static_cast<USIZE>(shadersBytecodeOffset));
		shadersBytecode.resize(mpCacheFileReader->GetFileLength() - static_cast<USIZE>(shadersBytecodeOffset));
		result = result | mpCacheFileReader->Read(shadersBytecode.data(), shadersBytecode.size());

		result = result | mpCacheFileReader->Close();

		if (RC_OK != result)
		{
			return result;
		}

		// \note Construct TShaderCompilerOutput from shaders descriptions entries
		result = result | ConstructShadersMetadataFromEntries(shaders, shadersBytecode, mShadersInfoTable);

		return result;
	}


	TDE2_API IShaderCache* CreateShaderCache(IBinaryFileReader* pCacheReader, IBinaryFileWriter* pCacheWriter, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IShaderCache, CShaderCache, result, pCacheReader, pCacheWriter);
	}
}