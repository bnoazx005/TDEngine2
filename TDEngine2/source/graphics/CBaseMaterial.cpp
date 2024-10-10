#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/CBaseShader.h"
#include "../../include/graphics/ITexture.h"
#include "../../include/graphics/IShaderCompiler.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/CBaseGraphicsPipeline.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/platform/CYAMLFile.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/utils/Utils.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/graphics/CBaseTexture2D.h"
#define META_EXPORT_GRAPHICS_SECTION
#include "../../include/metadata.h"
#include "../../include/math/MathUtils.h"
#include "stringUtils.hpp"
#include <cstring>
#include <functional>


namespace TDEngine2
{
	struct TMaterialArchiveKeys
	{
		static const std::string mGeometryTagKey;
		static const std::string mTexturesGroup;
		
		struct TTextureKeys
		{
			static const std::string mSlotKey;
			static const std::string mTextureTypeKey;
			static const std::string mTextureKey;
		};

		static const std::string mVariablesGroup;

		struct TVariablesPerInstancesKeys
		{
			static const std::string mVariableIdKey;
			static const std::string mValueKey;
			static const std::string mValueTypeKey;
		};
	};

	const std::string TMaterialArchiveKeys::mGeometryTagKey  = "geom_tag";
	const std::string TMaterialArchiveKeys::mTexturesGroup = "textures";

	const std::string TMaterialArchiveKeys::TTextureKeys::mSlotKey        = "slot_id";
	const std::string TMaterialArchiveKeys::TTextureKeys::mTextureTypeKey = "texture_type_id";
	const std::string TMaterialArchiveKeys::TTextureKeys::mTextureKey     = "texture_id";

	const std::string TMaterialArchiveKeys::mVariablesGroup = "variables";

	const std::string TMaterialArchiveKeys::TVariablesPerInstancesKeys::mVariableIdKey = "variable_id";
	const std::string TMaterialArchiveKeys::TVariablesPerInstancesKeys::mValueKey = "value";
	const std::string TMaterialArchiveKeys::TVariablesPerInstancesKeys::mValueTypeKey = "value_type";


	/// The map is used to convert a blob object which is a uniform buffer into an underlying type
	/// All types that appear as keys refer to shaders ones (use CBaseShaderCompiler::GetBuiltinTypeId to convert between in-engine and shader language's types)
	static const std::unordered_map<TypeId, std::function<E_RESULT_CODE(IArchiveWriter*, const void*)>> TypedSerializers
	{
		{ TDE2_TYPE_ID(I32), [](IArchiveWriter* pWriter, const void* pObject) { return Serialize(pWriter, *static_cast<const I32*>(pObject)); }},
		{ TDE2_TYPE_ID(U32), [](IArchiveWriter* pWriter, const void* pObject) { return Serialize(pWriter, *static_cast<const U32*>(pObject)); }},

		{ TDE2_TYPE_ID(F32), [](IArchiveWriter* pWriter, const void* pObject) { return Serialize(pWriter, *static_cast<const F32*>(pObject)); }},
		{ TDE2_TYPE_ID(F64), [](IArchiveWriter* pWriter, const void* pObject) { return Serialize(pWriter, *static_cast<const F64*>(pObject)); }},

		{ TDE2_TYPE_ID(TVector2), [](IArchiveWriter* pWriter, const void* pObject) { return Serialize(pWriter, *static_cast<const TVector2*>(pObject)); }},
		{ TDE2_TYPE_ID(TVector3), [](IArchiveWriter* pWriter, const void* pObject) { return Serialize(pWriter, *static_cast<const TVector3*>(pObject)); }},
		{ TDE2_TYPE_ID(TVector4), [](IArchiveWriter* pWriter, const void* pObject) { return Serialize(pWriter, *static_cast<const TVector4*>(pObject)); }},
	};


	template <typename T>
	E_RESULT_CODE LoadAndAssignVariable(IArchiveReader* pReader, IMaterial* pMaterial)
	{
		auto&& loadResult = Deserialize<T>(pReader);
		if (loadResult.HasError())
		{
			return loadResult.GetError();
		}

		auto&& variableValue = loadResult.Get();

		return pMaterial->SetVariableForInstance<T>(DefaultMaterialInstanceId, pReader->GetString(TMaterialArchiveKeys::TVariablesPerInstancesKeys::mVariableIdKey), variableValue);
	}


	static const std::unordered_map<TypeId, std::function<E_RESULT_CODE(IArchiveReader*, IMaterial*)>> TypedDeserializers
	{
		{ TDE2_TYPE_ID(I32), [](IArchiveReader* pReader, IMaterial* pMaterial) { return LoadAndAssignVariable<I32>(pReader, pMaterial); }},
		{ TDE2_TYPE_ID(U32), [](IArchiveReader* pReader, IMaterial* pMaterial) { return LoadAndAssignVariable<U32>(pReader, pMaterial); }},

		{ TDE2_TYPE_ID(F32), [](IArchiveReader* pReader, IMaterial* pMaterial) { return LoadAndAssignVariable<F32>(pReader, pMaterial); }},
		{ TDE2_TYPE_ID(F64), [](IArchiveReader* pReader, IMaterial* pMaterial) { return LoadAndAssignVariable<F64>(pReader, pMaterial); }},

		{ TDE2_TYPE_ID(TVector2), [](IArchiveReader* pReader, IMaterial* pMaterial) { return LoadAndAssignVariable<TVector2>(pReader, pMaterial); }},
		{ TDE2_TYPE_ID(TVector3), [](IArchiveReader* pReader, IMaterial* pMaterial) { return LoadAndAssignVariable<TVector3>(pReader, pMaterial); }},
		{ TDE2_TYPE_ID(TVector4), [](IArchiveReader* pReader, IMaterial* pMaterial) { return LoadAndAssignVariable<TVector4>(pReader, pMaterial); }},
	};


	/*!
		\note The declaration of TMaterialParameters is placed at IMaterial.h
	*/

	TMaterialParameters::TMaterialParameters(const std::string& shaderName, bool isTransparent, const TDepthStencilStateDesc& depthStencilState,
											 const TRasterizerStateDesc& rasterizerState, const TBlendStateDesc& blendState):
		mShaderName(shaderName), mBlendingParams(blendState), mDepthStencilParams(depthStencilState), mRasterizerParams(rasterizerState)
	{
		mBlendingParams.mIsEnabled = isTransparent;
	}


	const std::string CBaseMaterial::mResourceExtension = ".material";


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

		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseMaterial::Reset()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	TPtr<IMaterialInstance> CBaseMaterial::CreateInstance()
	{
		if (!mIsInitialized)
		{
			return TPtr<IMaterialInstance>(nullptr);
		}

		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);

		if (auto newInstanceResult = _allocateNewInstance())
		{
			TMaterialInstanceId instanceId = newInstanceResult.Get();
			TDE2_ASSERT(instanceId != DefaultMaterialInstanceId && instanceId != TMaterialInstanceId::Invalid);

			LOG_MESSAGE(Wrench::StringUtils::Format("[Base Material] A new instance {1} of material {0} was created", mName, static_cast<U32>(instanceId)));

			// \note Allocate uniform buffers for the instance
			const auto& defaultInstanceUserUniformBuffers = mpInstancesUserUniformBuffers[DefaultMaterialInstanceId];

			TUserUniformsArray newInstanceUniformBuffers;

			for (U8 i = 0; i < MaxNumberOfUserConstantBuffers; ++i)
			{
				newInstanceUniformBuffers[i].resize(defaultInstanceUserUniformBuffers[i].size());
			}

			mpInstancesUserUniformBuffers[instanceId] = std::move(newInstanceUniformBuffers);

			/// \note Copy all assigned textures for the default instance into the slots of a new one
			auto& newInstanceTexturesSlots = mInstancesAssignedTextures[instanceId];

			for (auto&& currTextureResourceInfo : mInstancesAssignedTextures[DefaultMaterialInstanceId])
			{
				newInstanceTexturesSlots[currTextureResourceInfo.first] = currTextureResourceInfo.second;
			}

			return mpInstancesArray[static_cast<U32>(instanceId)].Get();
		}

		return TPtr<IMaterialInstance>(nullptr);
	}

	E_RESULT_CODE CBaseMaterial::Load(IArchiveReader* pReader)
	{
		TDE2_PROFILER_SCOPE("CBaseMaterial::Load");
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);

		if (!pReader)
		{
			return RC_FAIL;
		}

		auto processGroup = [pReader](const std::string& groupName, const std::function<void()>& actionCallback)
		{
			if (pReader->BeginGroup(groupName) == RC_OK)
			{
				actionCallback();
				pReader->EndGroup();

				return;
			}

			LOG_WARNING(Wrench::StringUtils::Format("[BaseMaterial] Missing \"{0}\" group of parameters", groupName));
		};

		SetGeometrySubGroupTag(Meta::EnumTrait<E_GEOMETRY_SUBGROUP_TAGS>::FromString(pReader->GetString(TMaterialArchiveKeys::mGeometryTagKey)));

		// \note Load graphics pipeline as a single object
		auto pipelineConfigLoadResult = Deserialize<TGraphicsPipelineConfigDesc>(pReader);
		if (pipelineConfigLoadResult.HasError())
		{
			return pipelineConfigLoadResult.GetError();
		}

		mGraphicsPipelineConfigDesc = pipelineConfigLoadResult.Get();
		SetShader(mGraphicsPipelineConfigDesc.mShaderIdStr);
		
		auto pipelineCreationResult = mpGraphicsObjectManager->CreateGraphicsPipelineState(mGraphicsPipelineConfigDesc);
		if (pipelineCreationResult.HasError())
		{
			return pipelineCreationResult.GetError();
		}

		mGraphicsPipelineHandle = pipelineCreationResult.Get();

		processGroup(TMaterialArchiveKeys::mTexturesGroup, [pReader, this]
		{
			TDE2_PROFILER_SCOPE("CBaseMaterial::ProcessTexturesGroup");

			E_RESULT_CODE result = RC_OK;

			while (pReader->HasNextItem())
			{
				if ((result = pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr())) != RC_OK)
				{
					return;
				}

				const TMaterialInstanceId instanceId = static_cast<TMaterialInstanceId>(pReader->GetUInt32("instance_id"));

				pReader->BeginGroup("texture_bindings");

				while (pReader->HasNextItem())
				{
					pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());

					std::string slotId = pReader->GetString(TMaterialArchiveKeys::TTextureKeys::mSlotKey);
					std::string textureId = pReader->GetString(TMaterialArchiveKeys::TTextureKeys::mTextureKey);

					TypeId textureTypeId = TypeId(pReader->GetUInt32(TMaterialArchiveKeys::TTextureKeys::mTextureTypeKey));
					if (SetTextureResource(slotId, mpResourceManager->GetResource<ITexture>(mpResourceManager->Load(textureId, textureTypeId)).Get(), instanceId) != RC_OK)
					{
						LOG_WARNING(Wrench::StringUtils::Format("[BaseMaterial] Couldn't load texture \"{0}\"", textureId));
						result = result | RC_FAIL;
					}

					pReader->EndGroup();
				}

				pReader->EndGroup();

				if ((result = pReader->EndGroup()) != RC_OK)
				{
					return;
				}
			}
		});

		processGroup(TMaterialArchiveKeys::mVariablesGroup, [pReader, this]
		{
			TDE2_PROFILER_SCOPE("CBaseMaterial::ProcessUniformsGroup");

			E_RESULT_CODE result = RC_OK;

			while (pReader->HasNextItem())
			{
				if ((result = pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr())) != RC_OK)
				{
					return;
				}

				const TypeId currVariableTypeId = static_cast<TypeId>(pReader->GetUInt32("type_id"));
				auto it = TypedDeserializers.find(currVariableTypeId);

				if (it == TypedDeserializers.end())
				{
					LOG_ERROR(Wrench::StringUtils::Format("[BaseMateria] Unknown variable type id found, type_id: {0}", static_cast<U32>(currVariableTypeId)));
					TDE2_ASSERT(false);

					if ((result = pReader->EndGroup()) != RC_OK)
					{
						return;
					}

					continue;
				}

				result = (it->second)(pReader, this);
				TDE2_ASSERT(RC_OK == result);

				if ((result = pReader->EndGroup()) != RC_OK)
				{
					return;
				}
			}
		});

		return RC_OK;
	}

	E_RESULT_CODE CBaseMaterial::Save(IArchiveWriter* pWriter)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);

		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("meta");
		{
			pWriter->SetString("resource_type", "material");
			pWriter->SetUInt16("version_tag", mVersionTag);
		}
		pWriter->EndGroup();

		pWriter->SetString(TMaterialArchiveKeys::mGeometryTagKey, Meta::EnumTrait<E_GEOMETRY_SUBGROUP_TAGS>::ToString(mTag));

		auto pipelineCreationResult = mpGraphicsObjectManager->CreateGraphicsPipelineState(mGraphicsPipelineConfigDesc);
		if (pipelineCreationResult.IsOk())
		{
			mGraphicsPipelineHandle = pipelineCreationResult.Get();
		}

		if (TGraphicsPipelineStateId::Invalid != mGraphicsPipelineHandle)
		{
			auto pGraphicsPipeline = mpGraphicsObjectManager->GetGraphicsPipeline(mGraphicsPipelineHandle);
			Serialize(pWriter, pGraphicsPipeline->GetConfig());
		}

		pWriter->BeginGroup(TMaterialArchiveKeys::mTexturesGroup, true);
		{
			for (auto instanceTexturesEntry : this->mInstancesAssignedTextures)
			{
				pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());

				pWriter->SetUInt32("instance_id", static_cast<U32>(instanceTexturesEntry.first));

				pWriter->BeginGroup("texture_bindings", true);
				
				for (auto&& textureEntry : instanceTexturesEntry.second)
				{
					pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
					{
						pWriter->SetString(TMaterialArchiveKeys::TTextureKeys::mSlotKey, textureEntry.first);

						if (auto pTexture = dynamic_cast<IResource*>(textureEntry.second))
						{
							pWriter->SetUInt32(TMaterialArchiveKeys::TTextureKeys::mTextureTypeKey, static_cast<U32>(pTexture->GetResourceTypeId()));
							pWriter->SetString(TMaterialArchiveKeys::TTextureKeys::mTextureKey, pTexture->GetName());
						}
					}
					pWriter->EndGroup();
				}

				pWriter->EndGroup();

				pWriter->EndGroup();
			}
		}
		pWriter->EndGroup();

		auto pShader = mpResourceManager->GetResource<IShader>(mShaderHandle);
		if (!pShader)
		{
			return RC_FAIL;
		}
		
		auto pShaderMetaData = pShader->GetShaderMetaData();

		const auto& pDefaultInstanceBuffers = mpInstancesUserUniformBuffers[DefaultMaterialInstanceId];
		
		/// \note Write down variables values (only values for default material's instance are saved)
		pWriter->BeginGroup(TMaterialArchiveKeys::mVariablesGroup, true);
		{
			for (auto&& uniformBufferInfo : pShaderMetaData->mUniformBuffersInfo)
			{				
				auto&& uniformBufferData = uniformBufferInfo.second;

				if (E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL == (uniformBufferData.mFlags & E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL)) /// \note Skip system uniform buffers
				{
					continue;
				}

				for (auto&& currVariable : uniformBufferData.mVariables)
				{
					pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
					{
						pWriter->SetString(TMaterialArchiveKeys::TVariablesPerInstancesKeys::mVariableIdKey, currVariable.mName);

						auto&& pUniformBufferContent = pDefaultInstanceBuffers[uniformBufferData.mSlot - TotalNumberOfInternalConstantBuffers];
						
						auto serializerIt = TypedSerializers.find(currVariable.mTypeId);
						if (serializerIt == TypedSerializers.cend())
						{
							TDE2_ASSERT(false);
							continue;
						}

						(serializerIt->second)(pWriter, &pUniformBufferContent[currVariable.mOffset]);
					}
					pWriter->EndGroup();
				}
			}
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	TPtr<IMaterial> CBaseMaterial::Clone() const
	{
		const TMaterialParameters params
		{
			(TResourceId::Invalid == mShaderHandle) ? Wrench::StringUtils::GetEmptyStr() : mpResourceManager->GetResource<IResource>(mShaderHandle)->GetName(),
			mGraphicsPipelineConfigDesc.mBlendStateParams.mIsEnabled, // \todo Refactor this later
			mGraphicsPipelineConfigDesc.mDepthStencilStateParams,
			mGraphicsPipelineConfigDesc.mRasterizerStateParams,
			mGraphicsPipelineConfigDesc.mBlendStateParams
		};

		U16 counter = 1;

		/// \note Find the first available name for the clone
		/// \todo Implement helper for this purpose inside CResourceManager's implementation
		const std::string baseIdentifierWithoutExtension = mName.substr(0, mName.find(mResourceExtension));
		std::string cloneIdentifier = baseIdentifierWithoutExtension + std::to_string(counter) + mResourceExtension;

		while (TResourceId::Invalid != mpResourceManager->GetResourceId(cloneIdentifier))
		{
			cloneIdentifier = baseIdentifierWithoutExtension + std::to_string(++counter) + mResourceExtension;
		}

		const TResourceId clonedMaterialResourceId = mpResourceManager->Create<IMaterial>(cloneIdentifier, params);
		TDE2_ASSERT(TResourceId::Invalid != clonedMaterialResourceId);

		return (TResourceId::Invalid == clonedMaterialResourceId) ? nullptr : mpResourceManager->GetResource<IMaterial>(clonedMaterialResourceId);
	}

	void CBaseMaterial::SetShader(const std::string& shaderName)
	{
		TDE2_PROFILER_SCOPE("CBaseMaterial::SetShader");

		mShaderIdStr = shaderName;

		mShaderHandle = mpResourceManager->Load<IShader>(shaderName); /// \todo replace it with Create and load only on demand within Load method
		if (mShaderHandle == TResourceId::Invalid)
		{
			TDE2_ASSERT(false);
			return;
		}

		PANIC_ON_FAILURE(_initDefaultInstance(*mpResourceManager->GetResource<IShader>(mShaderHandle)->GetShaderMetaData()));
	}

	void CBaseMaterial::SetTransparentState(bool isTransparent)
	{
		mGraphicsPipelineConfigDesc.mBlendStateParams.mIsEnabled = isTransparent;
		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}

	void CBaseMaterial::SetBlendFactors(const E_BLEND_FACTOR_VALUE& srcFactor, const E_BLEND_FACTOR_VALUE& destFactor,
										const E_BLEND_FACTOR_VALUE& srcAlphaFactor, const E_BLEND_FACTOR_VALUE& destAlphaFactor)
	{
		TBlendStateDesc& blendStateDesc = mGraphicsPipelineConfigDesc.mBlendStateParams;
		blendStateDesc.mScrValue       = srcFactor;
		blendStateDesc.mDestValue      = destFactor;
		blendStateDesc.mScrAlphaValue  = srcAlphaFactor;
		blendStateDesc.mDestAlphaValue = destAlphaFactor;

		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}

	void CBaseMaterial::SetBlendOp(const E_BLEND_OP_TYPE& opType, const E_BLEND_OP_TYPE& alphaOpType)
	{
		TBlendStateDesc& blendStateDesc = mGraphicsPipelineConfigDesc.mBlendStateParams;
		blendStateDesc.mOpType      = opType;
		blendStateDesc.mAlphaOpType = alphaOpType;

		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}

	void CBaseMaterial::Bind(TMaterialInstanceId instanceId)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);

		auto pShaderInstance = mpResourceManager->GetResource<IShader>(mShaderHandle);

		if (!pShaderInstance || (instanceId == TMaterialInstanceId::Invalid))
		{
			return;
		}

		if (TGraphicsPipelineStateId::Invalid == mGraphicsPipelineHandle)
		{
			auto pipelineCreationResult = mpGraphicsObjectManager->CreateGraphicsPipelineState(mGraphicsPipelineConfigDesc);
			if (pipelineCreationResult.IsOk())
			{
				mGraphicsPipelineHandle = pipelineCreationResult.Get();
			}
		}

		if (auto pGraphicsPipeline = mpGraphicsObjectManager->GetGraphicsPipeline(mGraphicsPipelineHandle))
		{
			pGraphicsPipeline->Bind();
		}

		auto iter = mpInstancesUserUniformBuffers.find(instanceId);
		if (iter != mpInstancesUserUniformBuffers.cend())
		{
			auto&& instanceUniformBuffers = iter->second;

			U8 userUniformBufferId = 0;
			for (const auto& currUserDataBuffer : instanceUniformBuffers)
			{
				if (!currUserDataBuffer.size())
				{
					continue;
				}

				PANIC_ON_FAILURE(pShaderInstance->SetUserUniformsBuffer(userUniformBufferId++, &currUserDataBuffer.front(), currUserDataBuffer.size()));
			}
		}

		auto&& instanceTexturesStorage = mInstancesAssignedTextures[instanceId];

		for (auto iter = instanceTexturesStorage.cbegin(); iter != instanceTexturesStorage.cend(); ++iter)
		{
			pShaderInstance->SetTextureResource(iter->first, iter->second);
		}

		pShaderInstance->Bind();
	}

	E_RESULT_CODE CBaseMaterial::SetTextureResource(const std::string& resourceName, ITexture* pTexture, TMaterialInstanceId instanceId)
	{
		if (resourceName.empty() || !pTexture || instanceId == TMaterialInstanceId::Invalid)
		{
			return RC_INVALID_ARGS;
		}

		mInstancesAssignedTextures[instanceId][resourceName] = pTexture;

		return RC_OK;
	}

	E_RESULT_CODE CBaseMaterial::SetVariableForInstance(TMaterialInstanceId instanceId, const std::string& name, const void* pValue, U32 size)
	{
		return _setVariableForInstance(instanceId, name, pValue, size);
	}

	void CBaseMaterial::SetDepthBufferEnabled(bool state)
	{
		mGraphicsPipelineConfigDesc.mDepthStencilStateParams.mIsDepthTestEnabled = state;
		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}

	void CBaseMaterial::SetStencilBufferEnabled(bool state)
	{
		mGraphicsPipelineConfigDesc.mDepthStencilStateParams.mIsStencilTestEnabled = state;
		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}

	void CBaseMaterial::SetDepthWriteEnabled(bool state)
	{
		mGraphicsPipelineConfigDesc.mDepthStencilStateParams.mIsDepthWritingEnabled = state;
		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}

	void CBaseMaterial::SetDepthComparisonFunc(const E_COMPARISON_FUNC& funcType)
	{
		mGraphicsPipelineConfigDesc.mDepthStencilStateParams.mDepthCmpFunc = funcType;
		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}

	void CBaseMaterial::SetStencilReadMask(U8 value)
	{
		mGraphicsPipelineConfigDesc.mDepthStencilStateParams.mStencilReadMaskValue = value;
		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}

	void CBaseMaterial::SetStencilWriteMask(U8 value)
	{
		mGraphicsPipelineConfigDesc.mDepthStencilStateParams.mStencilWriteMaskValue = value;
		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}

	void CBaseMaterial::SetStencilRefValue(U8 value)
	{
		mGraphicsPipelineConfigDesc.mDepthStencilStateParams.mStencilRefValue = value;
		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}

	void CBaseMaterial::SetStencilFrontOp(const TStencilOperationDesc& op)
	{
		mGraphicsPipelineConfigDesc.mDepthStencilStateParams.mStencilFrontFaceOp = op;
		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}

	void CBaseMaterial::SetStencilBackOp(const TStencilOperationDesc& op)
	{
		mGraphicsPipelineConfigDesc.mDepthStencilStateParams.mStencilBackFaceOp = op;
		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}

	void CBaseMaterial::SetCullMode(const E_CULL_MODE& cullMode)
	{
		mGraphicsPipelineConfigDesc.mRasterizerStateParams.mCullMode = cullMode;
		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}

	void CBaseMaterial::SetScissorEnabled(bool state)
	{
		mGraphicsPipelineConfigDesc.mRasterizerStateParams.mIsScissorTestEnabled = state;
		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}

	void CBaseMaterial::SetWireframeMode(bool state)
	{
		mGraphicsPipelineConfigDesc.mRasterizerStateParams.mIsWireframeModeEnabled = state;
		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}

	void CBaseMaterial::SetFrontCCWOrderEnabled(bool state)
	{
		mGraphicsPipelineConfigDesc.mRasterizerStateParams.mIsFrontCCWEnabled = state;
		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}
	
	void CBaseMaterial::SetDepthBias(F32 bias, F32 maxBias)
	{
		TDE2_ASSERT(CMathUtils::IsLessOrEqual(bias, maxBias));

		TRasterizerStateDesc& rasterizerStateDesc = mGraphicsPipelineConfigDesc.mRasterizerStateParams;

		rasterizerStateDesc.mDepthBias = bias;
		rasterizerStateDesc.mMaxDepthBias = maxBias;

		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}

	void CBaseMaterial::SetDepthClippingEnabled(bool value)
	{
		mGraphicsPipelineConfigDesc.mRasterizerStateParams.mIsDepthClippingEnabled = value;
		mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	}

	void CBaseMaterial::SetGeometrySubGroupTag(const E_GEOMETRY_SUBGROUP_TAGS& tag)
	{
		mTag = tag;
	}

	U32 CBaseMaterial::GetVariableHash(const std::string& name) const
	{
		return TDE2_STRING_ID(name.c_str());
	}

	const std::string& CBaseMaterial::GetShaderId() const
	{
		return mShaderIdStr;
	}

	TResourceId CBaseMaterial::GetShaderHandle() const
	{
		return mShaderHandle;
	}

	bool CBaseMaterial::IsTransparent() const
	{
		return mGraphicsPipelineConfigDesc.mBlendStateParams.mIsEnabled;
	}

	const E_GEOMETRY_SUBGROUP_TAGS& CBaseMaterial::GetGeometrySubGroupTag() const
	{
		return mTag;
	}

	TResult<TPtr<IMaterialInstance>> CBaseMaterial::GetMaterialInstance(TMaterialInstanceId instanceId) const
	{
		return mpInstancesArray[static_cast<U32>(instanceId)];
	}

	ITexture* CBaseMaterial::GetTextureResource(const std::string& id, TMaterialInstanceId instanceId) const
	{
		auto it = mInstancesAssignedTextures.find(instanceId);
		if (it == mInstancesAssignedTextures.cend())
		{
			return nullptr;
		}

		if (id.empty())
		{
			if (it->second.empty())
			{
				return nullptr;
			}

			return it->second.begin()->second;
		}

		auto textureResourceIt = it->second.find(id);
		return (textureResourceIt == it->second.cend()) ? nullptr : textureResourceIt->second;
	}

	bool CBaseMaterial::IsScissorTestEnabled() const
	{
		return mGraphicsPipelineConfigDesc.mRasterizerStateParams.mIsScissorTestEnabled;
	}

	const TBlendStateDesc& CBaseMaterial::GetBlendingParams() const
	{
		return mGraphicsPipelineConfigDesc.mBlendStateParams;
	}

	const TDepthStencilStateDesc& CBaseMaterial::GetDepthStencilParams() const
	{
		return mGraphicsPipelineConfigDesc.mDepthStencilStateParams;
	}

#if TDE2_EDITORS_ENABLED
	
	void CBaseMaterial::ForEachTextureSlot(const TTextureResourceVisitAction& action)
	{
		if (!action)
		{
			return;
		}

		auto& textures = mInstancesAssignedTextures[DefaultMaterialInstanceId];

		for (auto&& currTextureSlotEntity : textures)
		{
			action(currTextureSlotEntity.first, currTextureSlotEntity.second);
		}
	}

	void CBaseMaterial::ForEachVariable(const TVariableVisitAction& action)
	{
		if (!action)
		{
			return;
		}
		
		auto pShaderResource = mpResourceManager->GetResource<IShader>(mShaderHandle);
		if (!pShaderResource)
		{
			return;
		}

		auto& pDefaultInstanceBuffers = mpInstancesUserUniformBuffers[DefaultMaterialInstanceId];

		auto&& shaderMeta = pShaderResource->GetShaderMetaData();
		for (auto&& currUniformBufferInfo : shaderMeta->mUniformBuffersInfo)
		{
			if (E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL == (E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL & currUniformBufferInfo.second.mFlags))
			{
				continue;
			}

			auto&& pUniformBufferContent = pDefaultInstanceBuffers[currUniformBufferInfo.second.mSlot - TotalNumberOfInternalConstantBuffers];

			for (auto&& currVariableInfo : currUniformBufferInfo.second.mVariables)
			{
				action(currVariableInfo, &pUniformBufferContent[currVariableInfo.mOffset]);
			}
		}
	}

#endif

	TResult<TPtr<IMaterialInstance>> CBaseMaterial::_setVariable(const std::string& name, const void* pValue, U32 size)
	{
		E_RESULT_CODE result = RC_FAIL;

		auto pNewMaterialInstance = CreateInstance();

		if (!pNewMaterialInstance || (result = _setVariableForInstance(pNewMaterialInstance->GetInstanceId(), name, pValue, size)) != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<TPtr<IMaterialInstance>>(pNewMaterialInstance);
	}

	E_RESULT_CODE CBaseMaterial::_setVariableForInstance(TMaterialInstanceId instanceId, const std::string& name, const void* pValue, U32 size)
	{
		U32 variableHash = GetVariableHash(name);

		auto&& iter = mUserVariablesHashTable.find(variableHash);
		if (iter == mUserVariablesHashTable.cend())
		{
			LOG_ERROR(Wrench::StringUtils::Format("[Base Material] There is no a variable with corresponding name ({0})", name));
			return RC_FAIL;
		}

		U32 bufferIndex = 0;
		USIZE varOffset = 0;
		std::tie(bufferIndex, varOffset) = iter->second; // first index is a buffer's id, the second one is variable's offset in bytes

		auto instanceBuffersIter = mpInstancesUserUniformBuffers.find(instanceId);
		if (instanceBuffersIter != mpInstancesUserUniformBuffers.cend())
		{
			auto& instanceUniformBuffers = instanceBuffersIter->second;

			TDE2_ASSERT((bufferIndex) >= 0 && (instanceUniformBuffers[bufferIndex].size() - varOffset) >= size);
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

			for (const auto& currVariableDesc : currUniformBufferDesc.mVariables)
			{
				mUserVariablesHashTable[TDE2_STRING_ID(currVariableDesc.mName.c_str())] = { slotIndex, currVariableDesc.mOffset };
			}
		}		

		mpInstancesUserUniformBuffers[DefaultMaterialInstanceId] = std::move(defaultInstanceUserUniforms);

		return RC_OK;
	}

	TDE2_API TResult<TMaterialInstanceId> CBaseMaterial::_allocateNewInstance()
	{
		E_RESULT_CODE result = RC_OK;

		TMaterialInstanceId instanceId = static_cast<TMaterialInstanceId>(mpInstancesArray.Add(TPtr<IMaterialInstance>(nullptr)));

		auto pNewMaterialInstance = TPtr<IMaterialInstance>(CreateBaseMaterialInstance(this, instanceId, result));

		if ((result != RC_OK) || (result = mpInstancesArray.ReplaceAt(static_cast<U32>(instanceId), pNewMaterialInstance)) != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<TMaterialInstanceId>(instanceId);
	}

	E_RESULT_CODE CBaseMaterial::_initDefaultInstance(const TShaderCompilerOutput& metadata)
	{
		TDE2_PROFILER_SCOPE("CBaseMaterial::_initDefaultInstance");

		mpInstancesArray.RemoveAll();
		mpInstancesUserUniformBuffers.clear();
		
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

			auto& instanceTextures = mInstancesAssignedTextures[DefaultMaterialInstanceId]; 

			for (auto&& currResourceInfo : metadata.mShaderResourcesInfo)
			{
				instanceTextures[currResourceInfo.first] = nullptr;
			}
		}

		return result;
	}

	const TPtr<IResourceLoader> CBaseMaterial::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IMaterial>();
	}


	bool CBaseMaterial::AlphaBasedMaterialComparator(TPtr<IMaterial> pLeft, const TPtr<IMaterial> pRight)
	{
		//return (!pLeft || pRight->IsTransparent()) || (pRight && !pLeft->IsTransparent());
		return (pLeft && pRight && !pLeft->IsTransparent() && pRight->IsTransparent());
	}


	TDE2_API IMaterial* CreateBaseMaterial(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IMaterial, CBaseMaterial, result, pResourceManager, pGraphicsContext, name);
	}


	TDE2_API IMaterial* CreateBaseMaterial(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
										   const TMaterialParameters& params, E_RESULT_CODE& result)
	{
		TDE2_PROFILER_SCOPE("::CreateBaseMaterial");

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

			auto& graphicsPipelineConfig = pMaterialInstance->mGraphicsPipelineConfigDesc;
			graphicsPipelineConfig.mBlendStateParams        = params.mBlendingParams;
			graphicsPipelineConfig.mDepthStencilStateParams = params.mDepthStencilParams;
			graphicsPipelineConfig.mRasterizerStateParams   = params.mRasterizerParams;
			graphicsPipelineConfig.mShaderIdStr             = params.mShaderName;
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

	E_RESULT_CODE CBaseMaterialLoader::LoadResource(IResource* pResource) const
	{
		TDE2_PROFILER_SCOPE("CBaseMaterialLoader::LoadResource");

		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		pResource->SetState(E_RESOURCE_STATE_TYPE::RST_LOADING);

		if (TResult<TFileEntryId> materialFileId = mpFileSystem->Open<IYAMLFileReader>(pResource->GetName()))
		{
			return dynamic_cast<IMaterial*>(pResource)->Load(mpFileSystem->Get<IYAMLFileReader>(materialFileId.Get()));
		}

		return RC_FILE_NOT_FOUND;
	}

	TypeId CBaseMaterialLoader::GetResourceTypeId() const
	{
		return IMaterial::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateBaseMaterialLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CBaseMaterialLoader, result, pResourceManager, pGraphicsContext, pFileSystem);
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

	IResource* CBaseMaterialFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		TDE2_PROFILER_SCOPE("CBaseMaterialLoader::Create");

		E_RESULT_CODE result = RC_OK;

		const TMaterialParameters& matParams = dynamic_cast<const TMaterialParameters&>(params);

		return dynamic_cast<IResource*>(CreateBaseMaterial(mpResourceManager, mpGraphicsContext, name, matParams, result));
	}

	IResource* CBaseMaterialFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		TDE2_PROFILER_SCOPE("CBaseMaterialLoader::CreateDefault");

		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateBaseMaterial(mpResourceManager, mpGraphicsContext, name, result));
	}

	TypeId CBaseMaterialFactory::GetResourceTypeId() const
	{
		return IMaterial::GetTypeId();
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
		mId = id;

		mIsInitialized = true;

		return RC_OK;
	}

	void CBaseMaterialInstance::Bind()
	{
		mpSharedMaterial->Bind(mId);
	}

	E_RESULT_CODE CBaseMaterialInstance::SetTextureResource(const std::string& resourceName, ITexture* pTexture)
	{
		return mpSharedMaterial->SetTextureResource(resourceName, pTexture, mId);
	}
		
	TMaterialInstanceId CBaseMaterialInstance::GetInstanceId() const
	{
		return mId;
	}

	E_RESULT_CODE CBaseMaterialInstance::_setVariable(const std::string& name, const void* pValue, U32 size)
	{
		return mpSharedMaterial->SetVariableForInstance(mId, name, pValue, size);
	}


	TDE2_API IMaterialInstance* CreateBaseMaterialInstance(IMaterial* pMaterial, TMaterialInstanceId id, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IMaterialInstance, CBaseMaterialInstance, result, pMaterial, id);
	}
}