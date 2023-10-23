#include "../../include/graphics/CBaseShader.h"
#include "../../include/graphics/IShaderCompiler.h"
#include "../../include/graphics/IConstantBuffer.h"
#include "../../include/graphics/IStructuredBuffer.h"
#include "../../include/graphics/ITexture.h"
#include "../../include/graphics/CBaseShaderCompiler.h"
#include "../../include/editor/CPerfProfiler.h"


namespace TDEngine2
{
	struct TShaderParametersArchiveKeys
	{
		static const std::string mUniformsInfoGroupKeyId;
		static const std::string mShaderResourcesGroupKeyId;
		static const std::string mStagesInfoGroupKeyId;
		static const std::string mSingleUniformBufferInfoGroupKeyId;
		static const std::string mSingleResourceInfoGroupKeyId;
		static const std::string mSingleStageInfoGroupKeyId;

		struct TUniformBufferGroupKeys
		{
			static const std::string mNameKeyId;
			static const std::string mSlotKeyId;
			static const std::string mSizeKeyId;
			static const std::string mFlagsKeyId;
			static const std::string mBufferIndexKeyId;
			static const std::string mVariablesKeyId;
			static const std::string mSingleVariableKeyId;

			struct TUniformGroupKeys
			{
				static const std::string mIdKeyId;
				static const std::string mSizeKeyId;
				static const std::string mOffsetKeyId;
				static const std::string mTypeKeyId;
				static const std::string mIsArrayKeyId;
			};
		};

		struct TShaderResourceGroupKeys
		{
			static const std::string mIdKeyId;
			static const std::string mTypeKeyId;
			static const std::string mSlotKeyId;
		};

		struct TShaderStageGroupKeys
		{
			static const std::string mTypeKeyId;
			static const std::string mBytecodeEntryKeyId;
			static const std::string mBytecodeEntriesTableKeyId;
			static const std::string mBytecodeOffsetKeyId;
			static const std::string mBytecodeSizeKeyId;
			static const std::string mBytecodeTypeKeyId;
			static const std::string mEntrypointNameKeyId;
		};
	};


	const std::string TShaderParametersArchiveKeys::mUniformsInfoGroupKeyId = "uniforms_info";
	const std::string TShaderParametersArchiveKeys::mShaderResourcesGroupKeyId = "resources_info";
	const std::string TShaderParametersArchiveKeys::mStagesInfoGroupKeyId = "stages_info";
	const std::string TShaderParametersArchiveKeys::mSingleUniformBufferInfoGroupKeyId = "uniform_buffer";
	const std::string TShaderParametersArchiveKeys::mSingleResourceInfoGroupKeyId = "shader_resource";
	const std::string TShaderParametersArchiveKeys::mSingleStageInfoGroupKeyId = "shader_stage";

	const std::string TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mNameKeyId = "id";
	const std::string TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mSlotKeyId = "slot";
	const std::string TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mSizeKeyId = "size";
	const std::string TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mFlagsKeyId = "flags";
	const std::string TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mBufferIndexKeyId = "buffer_index";
	const std::string TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mVariablesKeyId = "variables";
	const std::string TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mSingleVariableKeyId = "uniform";

	const std::string TShaderParametersArchiveKeys::TUniformBufferGroupKeys::TUniformGroupKeys::mIdKeyId = "id";
	const std::string TShaderParametersArchiveKeys::TUniformBufferGroupKeys::TUniformGroupKeys::mSizeKeyId = "size";
	const std::string TShaderParametersArchiveKeys::TUniformBufferGroupKeys::TUniformGroupKeys::mOffsetKeyId = "offset";
	const std::string TShaderParametersArchiveKeys::TUniformBufferGroupKeys::TUniformGroupKeys::mTypeKeyId = "type_id";
	const std::string TShaderParametersArchiveKeys::TUniformBufferGroupKeys::TUniformGroupKeys::mIsArrayKeyId = "is_array";

	const std::string TShaderParametersArchiveKeys::TShaderResourceGroupKeys::mIdKeyId = "id";
	const std::string TShaderParametersArchiveKeys::TShaderResourceGroupKeys::mTypeKeyId = "type";
	const std::string TShaderParametersArchiveKeys::TShaderResourceGroupKeys::mSlotKeyId = "slot";

	const std::string TShaderParametersArchiveKeys::TShaderStageGroupKeys::mTypeKeyId = "type";
	const std::string TShaderParametersArchiveKeys::TShaderStageGroupKeys::mBytecodeEntriesTableKeyId = "bytecode_entries";
	const std::string TShaderParametersArchiveKeys::TShaderStageGroupKeys::mBytecodeEntryKeyId = "bytecode_entry";
	const std::string TShaderParametersArchiveKeys::TShaderStageGroupKeys::mBytecodeOffsetKeyId = "offset";
	const std::string TShaderParametersArchiveKeys::TShaderStageGroupKeys::mBytecodeSizeKeyId = "size";
	const std::string TShaderParametersArchiveKeys::TShaderStageGroupKeys::mBytecodeTypeKeyId = "type";
	const std::string TShaderParametersArchiveKeys::TShaderStageGroupKeys::mEntrypointNameKeyId = "entrypoint";


	E_RESULT_CODE TShaderParameters::Load(IArchiveReader* pReader)
	{
		E_RESULT_CODE result = RC_OK;

		for (auto&& currSystemUniformBufferDesc : CBaseShaderCompiler::GetSystemUniformBuffersTable())
		{
			mUniformBuffersInfo.emplace(currSystemUniformBufferDesc.first, currSystemUniformBufferDesc.second);
		}

		// \note Uniforms info
		result = result | pReader->BeginGroup(TShaderParametersArchiveKeys::mUniformsInfoGroupKeyId);
		{
			TUniformBufferDesc uniformBufferDesc;
			TShaderUniformDesc uniformDesc;

			while (pReader->HasNextItem())
			{
				result = result | pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					result = result | pReader->BeginGroup(TShaderParametersArchiveKeys::mSingleUniformBufferInfoGroupKeyId);
					{
						std::string name = pReader->GetString(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mNameKeyId);
						
						uniformBufferDesc.mSlot  = pReader->GetUInt8(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mSlotKeyId);
						uniformBufferDesc.mSize  = pReader->GetUInt64(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mSizeKeyId);
						uniformBufferDesc.mFlags = static_cast<E_UNIFORM_BUFFER_DESC_FLAGS>(pReader->GetUInt32(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mFlagsKeyId));
						uniformBufferDesc.mBufferIndex = pReader->GetUInt32(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mBufferIndexKeyId);
						
						// variables
						result = result | pReader->BeginGroup(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mVariablesKeyId);
						{
							while (pReader->HasNextItem())
							{
								result = result | pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
								{
									result = result | pReader->BeginGroup(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mSingleVariableKeyId);
									{
										uniformDesc.mName = pReader->GetString(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::TUniformGroupKeys::mIdKeyId);
										uniformDesc.mSize = pReader->GetUInt64(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::TUniformGroupKeys::mSizeKeyId);
										uniformDesc.mOffset = pReader->GetUInt64(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::TUniformGroupKeys::mOffsetKeyId);
										uniformDesc.mTypeId = static_cast<TypeId>(pReader->GetInt32(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::TUniformGroupKeys::mTypeKeyId));
										uniformDesc.mIsArray = pReader->GetBool(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::TUniformGroupKeys::mIsArrayKeyId);

										uniformBufferDesc.mVariables.emplace_back(uniformDesc);
									}
									result = result | pReader->EndGroup();
								}
								result = result | pReader->EndGroup();
							}
						}
						result = result | pReader->EndGroup();

						mUniformBuffersInfo.emplace(name, uniformBufferDesc);
					}
					result = result | pReader->EndGroup();
				}
				result = result | pReader->EndGroup();
			}
		}
		result = result | pReader->EndGroup();

		// \note Resources info
		result = result | pReader->BeginGroup(TShaderParametersArchiveKeys::mShaderResourcesGroupKeyId);
		{
			TShaderResourceDesc resourceDesc;

			while (pReader->HasNextItem())
			{
				result = result | pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					result = result | pReader->BeginGroup(TShaderParametersArchiveKeys::mSingleResourceInfoGroupKeyId);
					{
						std::string name = pReader->GetString(TShaderParametersArchiveKeys::TShaderResourceGroupKeys::mIdKeyId);

						resourceDesc.mSlot = pReader->GetUInt8(TShaderParametersArchiveKeys::TShaderResourceGroupKeys::mSlotKeyId);
						resourceDesc.mType = static_cast<E_SHADER_RESOURCE_TYPE>(pReader->GetInt32(TShaderParametersArchiveKeys::TShaderResourceGroupKeys::mTypeKeyId));

						mShaderResourcesInfo.emplace(name, resourceDesc);
					}
					result = result | pReader->EndGroup();
				}
				result = result | pReader->EndGroup();
			}
		}
		result = result | pReader->EndGroup();

		// \note Stages info
		result = result | pReader->BeginGroup(TShaderParametersArchiveKeys::mStagesInfoGroupKeyId);
		{
			while (pReader->HasNextItem())
			{
				TShaderStageInfo stageDesc;

				result = result | pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					result = result | pReader->BeginGroup(TShaderParametersArchiveKeys::mSingleStageInfoGroupKeyId);
					{
						E_SHADER_STAGE_TYPE stageType = static_cast<E_SHADER_STAGE_TYPE>(pReader->GetUInt32(TShaderParametersArchiveKeys::TShaderStageGroupKeys::mTypeKeyId));

						// bytecodes per shader type
						result = result | pReader->BeginGroup(TShaderParametersArchiveKeys::TShaderStageGroupKeys::mBytecodeEntriesTableKeyId);
						{
							while (pReader->HasNextItem())
							{
								result = result | pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
								{
									result = result | pReader->BeginGroup(TShaderParametersArchiveKeys::TShaderStageGroupKeys::mBytecodeEntryKeyId);

									TShaderCacheBytecodeEntry bytecodeInfo;
									bytecodeInfo.mSize = pReader->GetUInt64(TShaderParametersArchiveKeys::TShaderStageGroupKeys::mBytecodeSizeKeyId);
									bytecodeInfo.mOffset = pReader->GetUInt64(TShaderParametersArchiveKeys::TShaderStageGroupKeys::mBytecodeOffsetKeyId);

									stageDesc.mBytecodeInfo.emplace(pReader->GetString(TShaderParametersArchiveKeys::TShaderStageGroupKeys::mBytecodeTypeKeyId), bytecodeInfo);

									result = result | pReader->EndGroup();
								}
								result = result | pReader->EndGroup();
							}
						}
						result = result | pReader->EndGroup();

						stageDesc.mEntrypoint = pReader->GetString(TShaderParametersArchiveKeys::TShaderStageGroupKeys::mEntrypointNameKeyId);

						mStages.emplace(stageType, stageDesc);
					}
					result = result | pReader->EndGroup();
				}
				result = result | pReader->EndGroup();
			}
		}
		result = result | pReader->EndGroup();

		return result;
	}
	
	E_RESULT_CODE TShaderParameters::Save(IArchiveWriter* pWriter)
	{
		E_RESULT_CODE result = pWriter->SetUInt32("type_id", static_cast<U32>(TDE2_TYPE_ID(TShaderParameters)));

		// \note Uniforms info
		result = result | pWriter->BeginGroup(TShaderParametersArchiveKeys::mUniformsInfoGroupKeyId, true);
		{
			for (auto&& currUniformEntry : mUniformBuffersInfo)
			{
				if (E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL == (E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL & currUniformEntry.second.mFlags))
				{
					continue;
				}

				result = result | pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					result = result | pWriter->BeginGroup(TShaderParametersArchiveKeys::mSingleUniformBufferInfoGroupKeyId);
					{
						result = result | pWriter->SetString(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mNameKeyId, currUniformEntry.first);
						result = result | pWriter->SetUInt8(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mSlotKeyId, currUniformEntry.second.mSlot);
						result = result | pWriter->SetUInt64(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mSizeKeyId, currUniformEntry.second.mSize);
						result = result | pWriter->SetUInt32(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mFlagsKeyId, static_cast<U32>(currUniformEntry.second.mFlags));
						result = result | pWriter->SetUInt32(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mBufferIndexKeyId, currUniformEntry.second.mBufferIndex);

						// variables
						result = result | pWriter->BeginGroup(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mVariablesKeyId, true);
						{
							for (auto&& currVariableInfo : currUniformEntry.second.mVariables)
							{
								result = result | pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
								{
									result = result | pWriter->BeginGroup(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::mSingleVariableKeyId);
									{
										result = result | pWriter->SetString(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::TUniformGroupKeys::mIdKeyId, currVariableInfo.mName);
										result = result | pWriter->SetUInt64(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::TUniformGroupKeys::mSizeKeyId, currVariableInfo.mSize);
										result = result | pWriter->SetUInt64(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::TUniformGroupKeys::mOffsetKeyId, currVariableInfo.mOffset);
										result = result | pWriter->SetInt32(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::TUniformGroupKeys::mTypeKeyId, static_cast<U32>(currVariableInfo.mTypeId));
										result = result | pWriter->SetBool(TShaderParametersArchiveKeys::TUniformBufferGroupKeys::TUniformGroupKeys::mIsArrayKeyId, currVariableInfo.mIsArray);
									}
									result = result | pWriter->EndGroup();
								}
								result = result | pWriter->EndGroup();
							}
						}
						result = result | pWriter->EndGroup();
					}
					result = result | pWriter->EndGroup();
				}
				result = result | pWriter->EndGroup();
			}
		}
		result = result | pWriter->EndGroup();

		// \note Resources info
		result = result | pWriter->BeginGroup(TShaderParametersArchiveKeys::mShaderResourcesGroupKeyId, true);
		{
			for (auto&& currResourceInfo : mShaderResourcesInfo)
			{
				result = result | pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					result = result | pWriter->BeginGroup(TShaderParametersArchiveKeys::mSingleResourceInfoGroupKeyId);
					{
						result = result | pWriter->SetString(TShaderParametersArchiveKeys::TShaderResourceGroupKeys::mIdKeyId, currResourceInfo.first);
						result = result | pWriter->SetUInt8(TShaderParametersArchiveKeys::TShaderResourceGroupKeys::mSlotKeyId, currResourceInfo.second.mSlot);
						result = result | pWriter->SetInt32(TShaderParametersArchiveKeys::TShaderResourceGroupKeys::mTypeKeyId, static_cast<U32>(currResourceInfo.second.mType));
					}
					result = result | pWriter->EndGroup();
				}
				result = result | pWriter->EndGroup();
			}
		}
		result = result | pWriter->EndGroup();

		// \note Stages info
		result = result | pWriter->BeginGroup(TShaderParametersArchiveKeys::mStagesInfoGroupKeyId, true);
		{
			for (auto&& currStageInfo : mStages)
			{
				result = result | pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					result = result | pWriter->BeginGroup(TShaderParametersArchiveKeys::mSingleStageInfoGroupKeyId);
					{
						result = result | pWriter->SetUInt32(TShaderParametersArchiveKeys::TShaderStageGroupKeys::mTypeKeyId, static_cast<U32>(currStageInfo.first));

						result = result | pWriter->BeginGroup(TShaderParametersArchiveKeys::TShaderStageGroupKeys::mBytecodeEntriesTableKeyId);

						for (auto&& currBytecodeEntry : currStageInfo.second.mBytecodeInfo)
						{
							result = result | pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
							{
								result = result | pWriter->BeginGroup(TShaderParametersArchiveKeys::TShaderStageGroupKeys::mBytecodeEntryKeyId);
								{
									result = result | pWriter->SetString(TShaderParametersArchiveKeys::TShaderStageGroupKeys::mBytecodeTypeKeyId, currBytecodeEntry.first);
									result = result | pWriter->SetUInt64(TShaderParametersArchiveKeys::TShaderStageGroupKeys::mBytecodeOffsetKeyId, currBytecodeEntry.second.mOffset);
									result = result | pWriter->SetUInt64(TShaderParametersArchiveKeys::TShaderStageGroupKeys::mBytecodeSizeKeyId, currBytecodeEntry.second.mSize);
								}
								result = result | pWriter->EndGroup();
							}
							result = result | pWriter->EndGroup();
						}

						result = result | pWriter->EndGroup();

						result = result | pWriter->SetString(TShaderParametersArchiveKeys::TShaderStageGroupKeys::mEntrypointNameKeyId, currStageInfo.second.mEntrypoint);
					}
					result = result | pWriter->EndGroup();
				}
				result = result | pWriter->EndGroup();
			}
		}
		result = result | pWriter->EndGroup();

		return result;
	}

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

		TResult<TShaderCompilerOutput*> compilerOutput = pShaderCompiler->Compile(sourceCode);

		if (compilerOutput.HasError())
		{
			return compilerOutput.GetError();
		}

		TShaderCompilerOutput* pCompilerOutput = compilerOutput.Get();

		return _initShaderInternal(pCompilerOutput);
	}

	E_RESULT_CODE CBaseShader::LoadFromShaderCache(IShaderCache* pShaderCache, const TShaderParameters* pShaderMetaData)
	{
		TDE2_PROFILER_SCOPE("CBaseShader::LoadFromShaderCache");

		auto pResult = _createMetaDataFromShaderParams(pShaderCache, pShaderMetaData);
		if (!pResult || pResult->mVSByteCode.empty() || pResult->mPSByteCode.empty())
		{
			return RC_FAIL;
		}

		return _initShaderInternal(pResult);
	}

	E_RESULT_CODE CBaseShader::SetUserUniformsBuffer(U8 slot, const U8* pData, USIZE dataSize)
	{
		if (slot >= MaxNumberOfUserConstantBuffers)
		{
			return RC_INVALID_ARGS;
		}

		/// \todo add checking for sizes of input data
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

		if ((result = pCurrUniformBuffer->Write(pData, dataSize)) != RC_OK)
		{
			return result;
		}

		pCurrUniformBuffer->Unmap();

		/// \note add the offset because all user-defined buffers go after the internal ones
		_bindUniformBuffer(TotalNumberOfInternalConstantBuffers + slot, pCurrUniformBuffer);

		return RC_OK;
	}

	void CBaseShader::Bind()
	{
		ITexture* pCurrTexture = nullptr;

		/// \note Bind textures
		for (U32 i = 0; i < mpTextures.size(); ++i)
		{
			pCurrTexture = mpTextures[i];

			if (!pCurrTexture)
			{
				continue;
			}

			pCurrTexture->Bind(i);
		}

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

		U8 currUserBufferId = 0;
		for (auto& pCurrUniformBuffer : mUniformBuffers)
		{
			_bindUniformBuffer(TotalNumberOfInternalConstantBuffers + currUserBufferId++, pCurrUniformBuffer);
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

		mpTextures[hashIter->second] = pTexture;

		return RC_OK;
	}

	E_RESULT_CODE CBaseShader::SetStructuredBufferResource(const std::string& resourceName, IStructuredBuffer* pBuffer)
	{
		if (resourceName.empty() || !pBuffer)
		{
			return RC_INVALID_ARGS;
		}

		auto hashIter = mStructuredBuffersHashTable.find(resourceName);
		if (hashIter == mStructuredBuffersHashTable.cend())
		{
			return RC_FAIL;
		}

		mpBuffers[hashIter->second] = pBuffer;

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
		result = result | _createTexturesHashTable(mpShaderMeta);

		return result;
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

	E_RESULT_CODE CBaseShader::_createTexturesHashTable(const TShaderCompilerOutput* pCompilerData)
	{
		auto shaderResourcesMap = pCompilerData->mShaderResourcesInfo;

		if (shaderResourcesMap.empty())
		{
			return RC_OK;
		}

		U8 currSlotIndex = 0;
		
		for (auto currShaderResourceInfo : shaderResourcesMap)
		{
			currSlotIndex = currShaderResourceInfo.second.mSlot;

			mTexturesHashTable[currShaderResourceInfo.first] = currSlotIndex;

			mpTextures.resize(currSlotIndex + 1);

			mpTextures[currSlotIndex] = nullptr;
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

		U8 currSlotIndex = 0;

		for (auto currShaderResourceInfo : shaderResourcesMap)
		{
			currSlotIndex = currShaderResourceInfo.second.mSlot;

			mStructuredBuffersHashTable[currShaderResourceInfo.first] = currSlotIndex;

			mpBuffers.resize(currSlotIndex + 1);

			mpBuffers[currSlotIndex] = nullptr;
		}

		return RC_OK;
	}

	const TPtr<IResourceLoader> CBaseShader::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IShader>();
	}


	/*!
		\brief CShaderCache's definition
	*/

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

		mIntermediateCacheBuffer.resize(mpCacheFileReader->GetFileLength());

		mpCacheFileReader->Read(mIntermediateCacheBuffer.data(), mIntermediateCacheBuffer.size());
		mpCacheFileReader->Close();

		mIsInitialized = true;

		return RC_OK;
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
		result = result | mpCacheFileWriter->Write(mIntermediateCacheBuffer.data(), mIntermediateCacheBuffer.size());
		result = result | mpCacheFileWriter->Flush();

		mIsDirty = false;

		return result;
	}

	TResult<TShaderCacheBytecodeEntry> CShaderCache::AddShaderBytecode(const std::vector<U8>& bytecode)
	{
		if (bytecode.empty())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		mIsDirty = true;
		
		TShaderCacheBytecodeEntry info;
		info.mOffset = mIntermediateCacheBuffer.size();
		info.mSize = bytecode.size();

		std::copy(bytecode.begin(), bytecode.end(), std::back_inserter(mIntermediateCacheBuffer));

		return Wrench::TOkValue<TShaderCacheBytecodeEntry>(info);
	}

	std::vector<U8> CShaderCache::GetBytecode(const TShaderCacheBytecodeEntry& info)
	{
		if (info.mOffset + info.mSize >= mIntermediateCacheBuffer.size())
		{
			return {};
		}

		std::vector<U8> buffer;
		buffer.resize(info.mSize);

		memcpy(buffer.data(), &mIntermediateCacheBuffer[info.mOffset], info.mSize);

		return buffer;
	}


	TDE2_API IShaderCache* CreateShaderCache(IBinaryFileReader* pCacheReader, IBinaryFileWriter* pCacheWriter, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IShaderCache, CShaderCache, result, pCacheReader, pCacheWriter);
	}
}