#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/CBaseShader.h"
#include "../../include/graphics/ITexture.h"
#include "../../include/graphics/IShaderCompiler.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/platform/CYAMLFile.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/utils/Utils.h"
#include "../../include/graphics/CBaseTexture2D.h"
#include "../../include/metadata.h"
#include "../../include/math/MathUtils.h"
#include "stringUtils.hpp"
#include <cstring>


namespace TDEngine2
{
	struct TMaterialArchiveKeys
	{
		static const std::string mShaderIdKey;
		static const std::string mTransparencyKey;
		
		static const std::string mBlendStateGroup;

		struct TBlendStateKeys
		{
			static const std::string mSrcColorKey;
			static const std::string mDestColorKey;
			static const std::string mOpTypeKey;

			static const std::string mSrcAlphaColorKey;
			static const std::string mDestAlphaColorKey;
			static const std::string mOpAlphaTypeKey;
		};

		static const std::string mDepthStencilStateGroup;

		struct TDepthStencilStateKeys
		{
			static const std::string mDepthTestKey;
			static const std::string mDepthWriteKey;
			static const std::string mDepthCmpFuncKey;
			static const std::string mStencilTestKey;
			static const std::string mStencilReadMaskKey;
			static const std::string mStencilWriteMaskKey;
			static const std::string mStencilFrontOpGroup;
			static const std::string mStencilBackOpGroup;
		};

		struct TStencilOpDescKeys
		{
			static const std::string mComparisonFuncKey;
			static const std::string mStencilPassOpKey;
			static const std::string mStencilFailOpKey;
			static const std::string mDepthFailOpKey;
		};

		static const std::string mRasterizerStateGroup;

		struct TRasterizerStateKeys
		{
			static const std::string mCullModeKey;
			static const std::string mWireframeModeKey;
			static const std::string mFrontCCWModeKey;
			static const std::string mDepthBiasKey;
			static const std::string mMaxDepthBiasKey;
			static const std::string mDepthClippingKey;
			static const std::string mScissorTestKey;
		};

		static const std::string mTexturesGroup;
		
		struct TTextureKeys
		{
			static const std::string mSlotKey;
			static const std::string mTextureTypeKey;
			static const std::string mTextureKey;
		};
	};

	const std::string TMaterialArchiveKeys::mShaderIdKey     = "shader_id";
	const std::string TMaterialArchiveKeys::mTransparencyKey = "transparency_enabled";
	const std::string TMaterialArchiveKeys::mBlendStateGroup = "blend_state";

	const std::string TMaterialArchiveKeys::TBlendStateKeys::mSrcColorKey       = "src_color";
	const std::string TMaterialArchiveKeys::TBlendStateKeys::mDestColorKey      = "dest_color";
	const std::string TMaterialArchiveKeys::TBlendStateKeys::mOpTypeKey         = "op_type";
	const std::string TMaterialArchiveKeys::TBlendStateKeys::mSrcAlphaColorKey  = "src_alpha";
	const std::string TMaterialArchiveKeys::TBlendStateKeys::mDestAlphaColorKey = "dest_alpha";
	const std::string TMaterialArchiveKeys::TBlendStateKeys::mOpAlphaTypeKey    = "op_alpha_type";

	const std::string TMaterialArchiveKeys::mDepthStencilStateGroup = "depth_stencil_state";

	const std::string TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthTestKey        = "depth_test_enabled";
	const std::string TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthWriteKey       = "depth_write_enabled";
	const std::string TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthCmpFuncKey     = "depth_cmp_func";
	const std::string TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilTestKey      = "stencil_test_enabled";
	const std::string TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilReadMaskKey  = "stencil_read_mask";
	const std::string TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilWriteMaskKey = "stencil_write_mask";
	const std::string TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilFrontOpGroup = "stencil_front_op";
	const std::string TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilBackOpGroup  = "stencil_back_op";

	const std::string TMaterialArchiveKeys::TStencilOpDescKeys::mComparisonFuncKey = "cmp_func";
	const std::string TMaterialArchiveKeys::TStencilOpDescKeys::mStencilPassOpKey = "pass_op";
	const std::string TMaterialArchiveKeys::TStencilOpDescKeys::mStencilFailOpKey = "fail_op";
	const std::string TMaterialArchiveKeys::TStencilOpDescKeys::mDepthFailOpKey = "depth_fail_op";

	const std::string TMaterialArchiveKeys::mRasterizerStateGroup = "rasterizer_state";

	const std::string TMaterialArchiveKeys::TRasterizerStateKeys::mCullModeKey      = "cull_mode";
	const std::string TMaterialArchiveKeys::TRasterizerStateKeys::mWireframeModeKey = "wireframe_mode";
	const std::string TMaterialArchiveKeys::TRasterizerStateKeys::mFrontCCWModeKey  = "front_CCW_mode";
	const std::string TMaterialArchiveKeys::TRasterizerStateKeys::mDepthBiasKey     = "depth_bias";
	const std::string TMaterialArchiveKeys::TRasterizerStateKeys::mMaxDepthBiasKey  = "max_depth_bias";
	const std::string TMaterialArchiveKeys::TRasterizerStateKeys::mDepthClippingKey = "depth_clipping_enabled";
	const std::string TMaterialArchiveKeys::TRasterizerStateKeys::mScissorTestKey   = "scissor_test_enabled";

	const std::string TMaterialArchiveKeys::mTexturesGroup = "textures";

	const std::string TMaterialArchiveKeys::TTextureKeys::mSlotKey        = "slot_id";
	const std::string TMaterialArchiveKeys::TTextureKeys::mTextureTypeKey = "texture_type_id";
	const std::string TMaterialArchiveKeys::TTextureKeys::mTextureKey     = "texture_id";


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

		SetShader(pReader->GetString(TMaterialArchiveKeys::mShaderIdKey));
		SetTransparentState(pReader->GetBool(TMaterialArchiveKeys::mTransparencyKey));

		processGroup(TMaterialArchiveKeys::mBlendStateGroup, [pReader, this]
		{
			TBlendStateDesc blendStateDesc;

			blendStateDesc.mScrValue = Meta::EnumTrait<E_BLEND_FACTOR_VALUE>::FromString(pReader->GetString(TMaterialArchiveKeys::TBlendStateKeys::mSrcColorKey));
			blendStateDesc.mDestValue = Meta::EnumTrait<E_BLEND_FACTOR_VALUE>::FromString(pReader->GetString(TMaterialArchiveKeys::TBlendStateKeys::mDestColorKey));
			blendStateDesc.mOpType = Meta::EnumTrait<E_BLEND_OP_TYPE>::FromString(pReader->GetString(TMaterialArchiveKeys::TBlendStateKeys::mOpTypeKey));

			blendStateDesc.mScrAlphaValue = Meta::EnumTrait<E_BLEND_FACTOR_VALUE>::FromString(pReader->GetString(TMaterialArchiveKeys::TBlendStateKeys::mSrcAlphaColorKey));
			blendStateDesc.mDestAlphaValue = Meta::EnumTrait<E_BLEND_FACTOR_VALUE>::FromString(pReader->GetString(TMaterialArchiveKeys::TBlendStateKeys::mDestAlphaColorKey));
			blendStateDesc.mAlphaOpType = Meta::EnumTrait<E_BLEND_OP_TYPE>::FromString(pReader->GetString(TMaterialArchiveKeys::TBlendStateKeys::mOpAlphaTypeKey));

			SetBlendFactors(blendStateDesc.mScrValue, blendStateDesc.mDestValue, blendStateDesc.mScrAlphaValue, blendStateDesc.mDestAlphaValue);
			SetBlendOp(blendStateDesc.mOpType, blendStateDesc.mAlphaOpType);
		});

		processGroup(TMaterialArchiveKeys::mDepthStencilStateGroup, [pReader, this]
		{
			SetDepthComparisonFunc(Meta::EnumTrait<E_COMPARISON_FUNC>::FromString(pReader->GetString(TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthCmpFuncKey)));

			SetDepthBufferEnabled(pReader->GetBool(TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthTestKey));
			SetDepthWriteEnabled(pReader->GetBool(TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthWriteKey));

			SetStencilBufferEnabled(pReader->GetBool(TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilTestKey));
			SetStencilReadMask(pReader->GetUInt8(TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilReadMaskKey));
			SetStencilWriteMask(pReader->GetUInt8(TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilWriteMaskKey));


			auto readStencilGroup = [this, pReader](const std::string& groupId, TStencilOpDesc& desc)
			{
				pReader->BeginGroup(groupId);
				{
					desc.mFunc = Meta::EnumTrait<E_COMPARISON_FUNC>::FromString(pReader->GetString(TMaterialArchiveKeys::TStencilOpDescKeys::mComparisonFuncKey));
					desc.mPassOp = Meta::EnumTrait<E_STENCIL_OP>::FromString(pReader->GetString(TMaterialArchiveKeys::TStencilOpDescKeys::mStencilPassOpKey));
					desc.mFailOp = Meta::EnumTrait<E_STENCIL_OP>::FromString(pReader->GetString(TMaterialArchiveKeys::TStencilOpDescKeys::mStencilFailOpKey));
					desc.mDepthFailOp = Meta::EnumTrait<E_STENCIL_OP>::FromString(pReader->GetString(TMaterialArchiveKeys::TStencilOpDescKeys::mDepthFailOpKey));
				}
				pReader->EndGroup();
			};

			readStencilGroup(TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilFrontOpGroup, mDepthStencilStateParams.mStencilFrontFaceOp);
			readStencilGroup(TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilBackOpGroup, mDepthStencilStateParams.mStencilBackFaceOp);
		});

		processGroup(TMaterialArchiveKeys::mRasterizerStateGroup, [pReader, this]
		{
			SetCullMode(Meta::EnumTrait<E_CULL_MODE>::FromString(pReader->GetString(TMaterialArchiveKeys::TRasterizerStateKeys::mCullModeKey)));

			SetWireframeMode(pReader->GetBool(TMaterialArchiveKeys::TRasterizerStateKeys::mWireframeModeKey));
			SetScissorEnabled(pReader->GetBool(TMaterialArchiveKeys::TRasterizerStateKeys::mScissorTestKey));
			SetFrontCCWOrderEnabled(pReader->GetBool(TMaterialArchiveKeys::TRasterizerStateKeys::mFrontCCWModeKey));
			SetDepthBias(pReader->GetFloat(TMaterialArchiveKeys::TRasterizerStateKeys::mDepthBiasKey), pReader->GetFloat(TMaterialArchiveKeys::TRasterizerStateKeys::mMaxDepthBiasKey));
			SetDepthClippingEnabled(pReader->GetBool(TMaterialArchiveKeys::TRasterizerStateKeys::mDepthClippingKey));
			// \todo Add another parameters
		});

		processGroup(TMaterialArchiveKeys::mTexturesGroup, [pReader, this]
		{
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
					if (SetTextureResource(slotId, mpResourceManager->GetResource<ITexture2D>(mpResourceManager->Load(textureId, textureTypeId)), instanceId) != RC_OK)
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

		return RC_OK;
	}

	E_RESULT_CODE CBaseMaterial::Save(IArchiveWriter* pWriter)
	{
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

		pWriter->SetString(TMaterialArchiveKeys::mShaderIdKey, mpResourceManager->GetResource(mShaderHandle)->GetName());
		pWriter->SetBool(TMaterialArchiveKeys::mTransparencyKey, mBlendStateParams.mIsEnabled);

		pWriter->BeginGroup(TMaterialArchiveKeys::mBlendStateGroup);
		{
			pWriter->SetString(TMaterialArchiveKeys::TBlendStateKeys::mSrcColorKey, Meta::EnumTrait<E_BLEND_FACTOR_VALUE>::ToString(mBlendStateParams.mScrValue));
			pWriter->SetString(TMaterialArchiveKeys::TBlendStateKeys::mDestColorKey, Meta::EnumTrait<E_BLEND_FACTOR_VALUE>::ToString(mBlendStateParams.mDestValue));
			pWriter->SetString(TMaterialArchiveKeys::TBlendStateKeys::mOpTypeKey, Meta::EnumTrait<E_BLEND_OP_TYPE>::ToString(mBlendStateParams.mOpType));

			pWriter->SetString(TMaterialArchiveKeys::TBlendStateKeys::mSrcAlphaColorKey, Meta::EnumTrait<E_BLEND_FACTOR_VALUE>::ToString(mBlendStateParams.mScrAlphaValue));
			pWriter->SetString(TMaterialArchiveKeys::TBlendStateKeys::mDestAlphaColorKey, Meta::EnumTrait<E_BLEND_FACTOR_VALUE>::ToString(mBlendStateParams.mDestAlphaValue));
			pWriter->SetString(TMaterialArchiveKeys::TBlendStateKeys::mOpAlphaTypeKey, Meta::EnumTrait<E_BLEND_OP_TYPE>::ToString(mBlendStateParams.mAlphaOpType));
		}
		pWriter->EndGroup();

		pWriter->BeginGroup(TMaterialArchiveKeys::mDepthStencilStateGroup);
		{
			pWriter->SetBool(TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthTestKey, mDepthStencilStateParams.mIsDepthTestEnabled);
			pWriter->SetBool(TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthWriteKey, mDepthStencilStateParams.mIsDepthWritingEnabled);
			pWriter->SetString(TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthCmpFuncKey, Meta::EnumTrait<E_COMPARISON_FUNC>::ToString(mDepthStencilStateParams.mDepthCmpFunc));

			pWriter->SetBool(TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilTestKey, mDepthStencilStateParams.mIsStencilTestEnabled);
			pWriter->SetUInt8(TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilReadMaskKey, mDepthStencilStateParams.mStencilReadMaskValue);
			pWriter->SetUInt8(TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilWriteMaskKey, mDepthStencilStateParams.mStencilWriteMaskValue);

			auto writeStencilGroup = [this, pWriter](const std::string& groupId, const TStencilOpDesc& desc)
			{
				pWriter->BeginGroup(groupId);
				{
					pWriter->SetString(TMaterialArchiveKeys::TStencilOpDescKeys::mComparisonFuncKey, Meta::EnumTrait<E_COMPARISON_FUNC>::ToString(desc.mFunc));
					pWriter->SetString(TMaterialArchiveKeys::TStencilOpDescKeys::mStencilPassOpKey, Meta::EnumTrait<E_STENCIL_OP>::ToString(desc.mPassOp));
					pWriter->SetString(TMaterialArchiveKeys::TStencilOpDescKeys::mStencilFailOpKey, Meta::EnumTrait<E_STENCIL_OP>::ToString(desc.mFailOp));
					pWriter->SetString(TMaterialArchiveKeys::TStencilOpDescKeys::mDepthFailOpKey, Meta::EnumTrait<E_STENCIL_OP>::ToString(desc.mDepthFailOp));
				}
				pWriter->EndGroup();
			};

			writeStencilGroup(TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilFrontOpGroup, mDepthStencilStateParams.mStencilFrontFaceOp);
			writeStencilGroup(TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilBackOpGroup, mDepthStencilStateParams.mStencilBackFaceOp);
		}
		pWriter->EndGroup();

		pWriter->BeginGroup(TMaterialArchiveKeys::mRasterizerStateGroup);
		{
			pWriter->SetString(TMaterialArchiveKeys::TRasterizerStateKeys::mCullModeKey, Meta::EnumTrait<E_CULL_MODE>::ToString(mRasterizerStateParams.mCullMode));
			pWriter->SetBool(TMaterialArchiveKeys::TRasterizerStateKeys::mWireframeModeKey, mRasterizerStateParams.mIsWireframeModeEnabled);
			pWriter->SetBool(TMaterialArchiveKeys::TRasterizerStateKeys::mFrontCCWModeKey, mRasterizerStateParams.mIsFrontCCWEnabled);
			pWriter->SetFloat(TMaterialArchiveKeys::TRasterizerStateKeys::mDepthBiasKey, mRasterizerStateParams.mDepthBias);
			pWriter->SetFloat(TMaterialArchiveKeys::TRasterizerStateKeys::mMaxDepthBiasKey, mRasterizerStateParams.mMaxDepthBias);
			pWriter->SetBool(TMaterialArchiveKeys::TRasterizerStateKeys::mDepthClippingKey, mRasterizerStateParams.mIsDepthClippingEnabled);
			pWriter->SetBool(TMaterialArchiveKeys::TRasterizerStateKeys::mScissorTestKey, mRasterizerStateParams.mIsScissorTestEnabled);
		}
		pWriter->EndGroup();

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

		return RC_OK;
	}

	void CBaseMaterial::SetShader(const std::string& shaderName)
	{
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
		IShader* pShaderInstance = dynamic_cast<IShader*>(mpResourceManager->GetResource(mShaderHandle));

		if (!pShaderInstance || (instanceId == TMaterialInstanceId::Invalid))
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

	void CBaseMaterial::SetStencilReadMask(U8 value)
	{
		mDepthStencilStateParams.mStencilReadMaskValue = value;
	}

	void CBaseMaterial::SetStencilWriteMask(U8 value)
	{
		mDepthStencilStateParams.mStencilWriteMaskValue = value;
	}

	void CBaseMaterial::SetStencilFrontOp(const TStencilOperationDesc& op)
	{
		mDepthStencilStateParams.mStencilFrontFaceOp = op;
	}

	void CBaseMaterial::SetStencilBackOp(const TStencilOperationDesc& op)
	{
		mDepthStencilStateParams.mStencilBackFaceOp = op;
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

	void CBaseMaterial::SetFrontCCWOrderEnabled(bool state)
	{
		mRasterizerStateParams.mIsFrontCCWEnabled = state;
	}
	
	void CBaseMaterial::SetDepthBias(F32 bias, F32 maxBias)
	{
		TDE2_ASSERT(CMathUtils::IsLessOrEqual(bias, maxBias));

		mRasterizerStateParams.mDepthBias = bias;
		mRasterizerStateParams.mMaxDepthBias = maxBias;
	}

	void CBaseMaterial::SetDepthClippingEnabled(bool value)
	{
		mRasterizerStateParams.mIsDepthClippingEnabled = value;
	}

	void CBaseMaterial::SetGeometrySubGroupTag(const E_GEOMETRY_SUBGROUP_TAGS& tag)
	{
		mTag = tag;
	}

	U32 CBaseMaterial::GetVariableHash(const std::string& name) const
	{
		return TDE2_STRING_ID(name.c_str());
	}

	TResourceId CBaseMaterial::GetShaderHandle() const
	{
		return mShaderHandle;
	}

	bool CBaseMaterial::IsTransparent() const
	{
		return mBlendStateParams.mIsEnabled;
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

			USIZE variableBytesOffset = 0;

			for (const auto& currVariableDesc : currUniformBufferDesc.mVariables)
			{
				mUserVariablesHashTable[TDE2_STRING_ID(currVariableDesc.mName.c_str())] = { slotIndex, variableBytesOffset };
				variableBytesOffset += currVariableDesc.mSize;
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
		}

		return result;
	}

	const IResourceLoader* CBaseMaterial::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IMaterial>();
	}


	bool CBaseMaterial::AlphaBasedMaterialComparator(const IMaterial* pLeft, const IMaterial* pRight)
	{
		return (!pLeft || pRight->IsTransparent()) || (pRight && !pLeft->IsTransparent());
	}


	TDE2_API IMaterial* CreateBaseMaterial(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IMaterial, CBaseMaterial, result, pResourceManager, pGraphicsContext, name);
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

	E_RESULT_CODE CBaseMaterialLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

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
		E_RESULT_CODE result = RC_OK;

		const TMaterialParameters& matParams = dynamic_cast<const TMaterialParameters&>(params);

		return dynamic_cast<IResource*>(CreateBaseMaterial(mpResourceManager, mpGraphicsContext, name, matParams, result));
	}

	IResource* CBaseMaterialFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
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