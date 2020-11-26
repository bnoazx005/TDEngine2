#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/CBaseShader.h"
#include "../../include/graphics/ITexture.h"
#include "../../include/graphics/IShaderCompiler.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IResourceHandler.h"
#include "../../include/platform/CYAMLFile.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/utils/Utils.h"
#include "../../include/graphics/CBaseTexture2D.h"
#include "../../include/metadata.h"
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

	const std::string TMaterialArchiveKeys::mShaderIdKey     = "shader-id";
	const std::string TMaterialArchiveKeys::mTransparencyKey = "transparency-enabled";
	const std::string TMaterialArchiveKeys::mBlendStateGroup = "blend-state";

	const std::string TMaterialArchiveKeys::TBlendStateKeys::mSrcColorKey       = "src-color";
	const std::string TMaterialArchiveKeys::TBlendStateKeys::mDestColorKey      = "dest-color";
	const std::string TMaterialArchiveKeys::TBlendStateKeys::mOpTypeKey         = "op-type";
	const std::string TMaterialArchiveKeys::TBlendStateKeys::mSrcAlphaColorKey  = "src-alpha";
	const std::string TMaterialArchiveKeys::TBlendStateKeys::mDestAlphaColorKey = "dest-alpha";
	const std::string TMaterialArchiveKeys::TBlendStateKeys::mOpAlphaTypeKey    = "op-alpha-type";

	const std::string TMaterialArchiveKeys::mDepthStencilStateGroup = "depth-stencil-state";

	const std::string TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthTestKey        = "depth-test-enabled";
	const std::string TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthWriteKey       = "depth-write-enabled";
	const std::string TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthCmpFuncKey     = "depth-cmp-func";
	const std::string TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilTestKey      = "stencil-test-enabled";
	const std::string TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilReadMaskKey  = "stencil-read-mask";
	const std::string TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilWriteMaskKey = "stencil-write-mask";

	const std::string TMaterialArchiveKeys::mRasterizerStateGroup = "rasterizer-state";

	const std::string TMaterialArchiveKeys::TRasterizerStateKeys::mCullModeKey      = "cull-mode";
	const std::string TMaterialArchiveKeys::TRasterizerStateKeys::mWireframeModeKey = "wireframe-mode";
	const std::string TMaterialArchiveKeys::TRasterizerStateKeys::mFrontCCWModeKey  = "front-CCW-mode";
	const std::string TMaterialArchiveKeys::TRasterizerStateKeys::mDepthBiasKey     = "depth-bias";
	const std::string TMaterialArchiveKeys::TRasterizerStateKeys::mMaxDepthBiasKey  = "max-depth-bias";
	const std::string TMaterialArchiveKeys::TRasterizerStateKeys::mDepthClippingKey = "depth-clipping-enabled";
	const std::string TMaterialArchiveKeys::TRasterizerStateKeys::mScissorTestKey   = "scissor-test-enabled";

	const std::string TMaterialArchiveKeys::mTexturesGroup = "textures";

	const std::string TMaterialArchiveKeys::TTextureKeys::mSlotKey        = "slot-id";
	const std::string TMaterialArchiveKeys::TTextureKeys::mTextureTypeKey = "texture-type-id";
	const std::string TMaterialArchiveKeys::TTextureKeys::mTextureKey     = "texture-id";


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
			TDE2_ASSERT(instanceId != DefaultMaterialInstanceId && instanceId != TMaterialInstanceId::Invalid);

			LOG_MESSAGE(Wrench::StringUtils::Format("[Base Material] A new instance {1} of material {0} was created", mName, static_cast<U32>(instanceId)));

			// \note Allocate uniform buffers for the instance
			auto&& defaultInstanceUserUniformBuffers = mpInstancesUserUniformBuffers[static_cast<U32>(DefaultMaterialInstanceId)].Get();

			TUserUniformsArray newInstanceUniformBuffers;

			for (U8 i = 0; i < MaxNumberOfUserConstantBuffers; ++i)
			{
				newInstanceUniformBuffers[i].resize(defaultInstanceUserUniformBuffers[i].size());
			}

			if (mpInstancesUserUniformBuffers.Add(newInstanceUniformBuffers) != static_cast<U32>(instanceId))
			{
				TDE2_ASSERT(false);
				return nullptr;
			}

			return mpInstancesArray[static_cast<U32>(instanceId)].Get();
		}

		return nullptr;
	}

	E_RESULT_CODE CBaseMaterial::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		static const std::unordered_map<std::string, E_BLEND_FACTOR_VALUE> str2blendFactor // \todo Reimplement later when Enum::ToString will be implemented
		{
			{ "ZERO", E_BLEND_FACTOR_VALUE::ZERO },
			{ "ONE", E_BLEND_FACTOR_VALUE::ONE },
			{ "SOURCE_ALPHA", E_BLEND_FACTOR_VALUE::SOURCE_ALPHA },
			{ "ONE_MINUS_SOURCE_ALPHA", E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_ALPHA },
			{ "DEST_ALPHA", E_BLEND_FACTOR_VALUE::DEST_ALPHA },
			{ "ONE_MINUS_DEST_ALPHA", E_BLEND_FACTOR_VALUE::ONE_MINUS_DEST_ALPHA },
			{ "CONSTANT_ALPHA", E_BLEND_FACTOR_VALUE::CONSTANT_ALPHA },
			{ "ONE_MINUS_CONSTANT_ALPHA", E_BLEND_FACTOR_VALUE::ONE_MINUS_CONSTANT_ALPHA },
			{ "SOURCE_COLOR", E_BLEND_FACTOR_VALUE::SOURCE_COLOR },
			{ "ONE_MINUS_SOURCE_COLOR", E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_COLOR },
			{ "DEST_COLOR", E_BLEND_FACTOR_VALUE::DEST_COLOR },
			{ "ONE_MINUS_DEST_COLOR", E_BLEND_FACTOR_VALUE::ONE_MINUS_DEST_COLOR },
		};

		static const std::unordered_map<std::string, E_BLEND_OP_TYPE> str2blendOp
		{
			{ "ADD", E_BLEND_OP_TYPE::ADD },
			{ "SUB", E_BLEND_OP_TYPE::SUBT },
			{ "RSUB", E_BLEND_OP_TYPE::REVERSED_SUBT },
		};

		static const std::unordered_map<std::string, E_CULL_MODE> str2cullMode
		{
			{ "FRONT", E_CULL_MODE::FRONT },
			{ "BACK", E_CULL_MODE::BACK },
			{ "NONE", E_CULL_MODE::NONE },
		};

		static const std::unordered_map<std::string, E_COMPARISON_FUNC> str2comparisonFunc
		{
			{ "NEVER", E_COMPARISON_FUNC::NEVER },
			{ "LESS", E_COMPARISON_FUNC::LESS },
			{ "EQUAL", E_COMPARISON_FUNC::EQUAL },
			{ "LESS_EQUAL", E_COMPARISON_FUNC::LESS_EQUAL },
			{ "GREATER", E_COMPARISON_FUNC::GREATER },
			{ "NOT_EQUAL", E_COMPARISON_FUNC::NOT_EQUAL },
			{ "GREATER_EQUAL", E_COMPARISON_FUNC::GREATER_EQUAL },
			{ "ALWAYS", E_COMPARISON_FUNC::ALWAYS },
		};

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

		auto applyValue = [](auto valuesMap, const std::string& value, auto&& actionCallback)
		{
			auto&& iter = valuesMap.find(value);
			if (iter != valuesMap.cend())
			{
				actionCallback(iter->second);
			}
		};

		SetShader(pReader->GetString(TMaterialArchiveKeys::mShaderIdKey));
		SetTransparentState(pReader->GetBool(TMaterialArchiveKeys::mTransparencyKey));

		processGroup(TMaterialArchiveKeys::mBlendStateGroup, [pReader, applyValue, this]
		{
			TBlendStateDesc blendStateDesc;

			applyValue(str2blendFactor, pReader->GetString(TMaterialArchiveKeys::TBlendStateKeys::mSrcColorKey), [&blendStateDesc](auto&& value) { blendStateDesc.mScrValue = value; });
			applyValue(str2blendFactor, pReader->GetString(TMaterialArchiveKeys::TBlendStateKeys::mDestColorKey), [&blendStateDesc](auto&& value) { blendStateDesc.mDestValue = value; });
			applyValue(str2blendOp, pReader->GetString(TMaterialArchiveKeys::TBlendStateKeys::mOpTypeKey), [&blendStateDesc](auto&& value) { blendStateDesc.mOpType = value; });

			applyValue(str2blendFactor, pReader->GetString(TMaterialArchiveKeys::TBlendStateKeys::mSrcAlphaColorKey), [&blendStateDesc](auto&& value) { blendStateDesc.mScrAlphaValue = value; });
			applyValue(str2blendFactor, pReader->GetString(TMaterialArchiveKeys::TBlendStateKeys::mDestAlphaColorKey), [&blendStateDesc](auto&& value) { blendStateDesc.mDestAlphaValue = value; });
			applyValue(str2blendOp, pReader->GetString(TMaterialArchiveKeys::TBlendStateKeys::mOpAlphaTypeKey), [&blendStateDesc](auto&& value) { blendStateDesc.mAlphaOpType = value; });

			SetBlendFactors(blendStateDesc.mScrValue, blendStateDesc.mDestValue, blendStateDesc.mScrAlphaValue, blendStateDesc.mDestAlphaValue);
			SetBlendOp(blendStateDesc.mOpType, blendStateDesc.mAlphaOpType);
		});

		processGroup(TMaterialArchiveKeys::mDepthStencilStateGroup, [pReader, applyValue, this]
		{
			applyValue(str2comparisonFunc, pReader->GetString(TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthCmpFuncKey), [this](auto&& value)
			{
				SetDepthComparisonFunc(value);
			});

			SetDepthBufferEnabled(pReader->GetBool(TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthTestKey));
			SetDepthWriteEnabled(pReader->GetBool(TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthWriteKey));

			SetStencilBufferEnabled(pReader->GetBool(TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilTestKey));
			// \todo Add parametrization of stencil test
		});

		processGroup(TMaterialArchiveKeys::mRasterizerStateGroup, [pReader, applyValue, this]
		{
			applyValue(str2cullMode, pReader->GetString(TMaterialArchiveKeys::TRasterizerStateKeys::mCullModeKey), [this](auto&& value)
			{
				SetCullMode(value);
			});

			SetWireframeMode(pReader->GetBool(TMaterialArchiveKeys::TRasterizerStateKeys::mWireframeModeKey));
			SetScissorEnabled(pReader->GetBool(TMaterialArchiveKeys::TRasterizerStateKeys::mScissorTestKey));
			// \todo Add another parameters
		});

		processGroup(TMaterialArchiveKeys::mTexturesGroup, [pReader, applyValue, this]
		{
			E_RESULT_CODE result = RC_OK;

			while (pReader->HasNextItem())
			{
				if ((result = pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr())) != RC_OK)
				{
					return;
				}

				const TMaterialInstanceId instanceId = static_cast<TMaterialInstanceId>(pReader->GetUInt32("instance-id"));

				pReader->BeginGroup("texture-bindings");

				while (pReader->HasNextItem())
				{
					pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());

					std::string slotId = pReader->GetString(TMaterialArchiveKeys::TTextureKeys::mSlotKey);
					std::string textureId = pReader->GetString(TMaterialArchiveKeys::TTextureKeys::mTextureKey);

					TypeId textureTypeId = TypeId(pReader->GetUInt32(TMaterialArchiveKeys::TTextureKeys::mTextureTypeKey));
					if (SetTextureResource(slotId, mpResourceManager->Load(textureId, textureTypeId)->Get<ITexture>(RAT_BLOCKING), instanceId) != RC_OK)
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
		static const std::unordered_map<E_BLEND_FACTOR_VALUE, std::string> blendFactor2Str // \todo Reimplement later when Enum::ToString will be implemented
		{
			{ E_BLEND_FACTOR_VALUE::ZERO, "ZERO" },
			{ E_BLEND_FACTOR_VALUE::ONE, "ONE" },
			{ E_BLEND_FACTOR_VALUE::SOURCE_ALPHA, "SOURCE_ALPHA" },
			{ E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_ALPHA, "ONE_MINUS_SOURCE_ALPHA" },
			{ E_BLEND_FACTOR_VALUE::DEST_ALPHA, "DEST_ALPHA" },
			{ E_BLEND_FACTOR_VALUE::ONE_MINUS_DEST_ALPHA, "ONE_MINUS_DEST_ALPHA" },
			{ E_BLEND_FACTOR_VALUE::CONSTANT_ALPHA, "CONSTANT_ALPHA" },
			{ E_BLEND_FACTOR_VALUE::ONE_MINUS_CONSTANT_ALPHA, "ONE_MINUS_CONSTANT_ALPHA" },
			{ E_BLEND_FACTOR_VALUE::SOURCE_COLOR, "SOURCE_COLOR" },
			{ E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_COLOR, "ONE_MINUS_SOURCE_COLOR" },
			{ E_BLEND_FACTOR_VALUE::DEST_COLOR, "DEST_COLOR" },
			{ E_BLEND_FACTOR_VALUE::ONE_MINUS_DEST_COLOR, "ONE_MINUS_DEST_COLOR" },
		};

		static const std::unordered_map<E_BLEND_OP_TYPE, std::string> blendOp2Str
		{
			{ E_BLEND_OP_TYPE::ADD, "ADD" },
			{ E_BLEND_OP_TYPE::SUBT, "SUB" },
			{ E_BLEND_OP_TYPE::REVERSED_SUBT, "RSUB" },
		};

		static const std::unordered_map<E_CULL_MODE, std::string> cullMode2Str
		{
			{ E_CULL_MODE::FRONT, "FRONT" },
			{ E_CULL_MODE::BACK, "BACK" },
			{ E_CULL_MODE::NONE, "NONE" },
		};

		static const std::unordered_map<E_COMPARISON_FUNC, std::string> comparisonFunc2Str
		{
			{ E_COMPARISON_FUNC::NEVER, "NEVER" },
			{ E_COMPARISON_FUNC::LESS, "LESS" },
			{ E_COMPARISON_FUNC::EQUAL, "EQUAL" },
			{ E_COMPARISON_FUNC::LESS_EQUAL, "LESS_EQUAL" },
			{ E_COMPARISON_FUNC::GREATER, "GREATER" },
			{ E_COMPARISON_FUNC::NOT_EQUAL, "NOT_EQUAL" },
			{ E_COMPARISON_FUNC::GREATER_EQUAL, "GREATER_EQUAL" },
			{ E_COMPARISON_FUNC::ALWAYS, "ALWAYS" },
		};

		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("meta");
		{
			pWriter->SetString("resource-type", "material");
			pWriter->SetUInt16("version-tag", mVersionTag);
		}
		pWriter->EndGroup();

		pWriter->SetString(TMaterialArchiveKeys::mShaderIdKey, mpShader->Get<IResource>(RAT_BLOCKING)->GetName());
		pWriter->SetBool(TMaterialArchiveKeys::mTransparencyKey, mBlendStateParams.mIsEnabled);

		pWriter->BeginGroup(TMaterialArchiveKeys::mBlendStateGroup);
		{
			pWriter->SetString(TMaterialArchiveKeys::TBlendStateKeys::mSrcColorKey, blendFactor2Str.at(mBlendStateParams.mScrValue));
			pWriter->SetString(TMaterialArchiveKeys::TBlendStateKeys::mDestColorKey, blendFactor2Str.at(mBlendStateParams.mDestValue));
			pWriter->SetString(TMaterialArchiveKeys::TBlendStateKeys::mOpTypeKey, blendOp2Str.at(mBlendStateParams.mOpType));

			pWriter->SetString(TMaterialArchiveKeys::TBlendStateKeys::mSrcAlphaColorKey, blendFactor2Str.at(mBlendStateParams.mScrAlphaValue));
			pWriter->SetString(TMaterialArchiveKeys::TBlendStateKeys::mDestAlphaColorKey, blendFactor2Str.at(mBlendStateParams.mDestAlphaValue));
			pWriter->SetString(TMaterialArchiveKeys::TBlendStateKeys::mOpAlphaTypeKey, blendOp2Str.at(mBlendStateParams.mAlphaOpType));
		}
		pWriter->EndGroup();

		pWriter->BeginGroup(TMaterialArchiveKeys::mDepthStencilStateGroup);
		{
			pWriter->SetBool(TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthTestKey, mDepthStencilStateParams.mIsDepthTestEnabled);
			pWriter->SetBool(TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthWriteKey, mDepthStencilStateParams.mIsDepthWritingEnabled);
			pWriter->SetString(TMaterialArchiveKeys::TDepthStencilStateKeys::mDepthCmpFuncKey, comparisonFunc2Str.at(mDepthStencilStateParams.mDepthCmpFunc));

			pWriter->SetBool(TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilTestKey, mDepthStencilStateParams.mIsStencilTestEnabled);
			pWriter->SetUInt8(TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilReadMaskKey, mDepthStencilStateParams.mStencilReadMaskValue);
			pWriter->SetUInt8(TMaterialArchiveKeys::TDepthStencilStateKeys::mStencilWriteMaskKey, mDepthStencilStateParams.mStencilWriteMaskValue);

			// \todo
		}
		pWriter->EndGroup();

		pWriter->BeginGroup(TMaterialArchiveKeys::mRasterizerStateGroup);
		{
			pWriter->SetString(TMaterialArchiveKeys::TRasterizerStateKeys::mCullModeKey, cullMode2Str.at(mRasterizerStateParams.mCullMode));
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

				pWriter->SetUInt32("instance-id", static_cast<U32>(instanceTexturesEntry.first));

				pWriter->BeginGroup("texture-bindings", true);
				
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

		if (auto userUniformBuffersResult = mpInstancesUserUniformBuffers[static_cast<U32>(instanceId)])
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

		auto&& instanceTexturesStorage = mInstancesAssignedTextures[instanceId];

		for (auto iter = instanceTexturesStorage.cbegin(); iter != instanceTexturesStorage.cend(); ++iter)
		{
			pShaderInstance->SetTextureResource(iter->first, iter->second);
		}
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
		return mpInstancesArray[static_cast<U32>(instanceId)];
	}

	TResult<IMaterialInstance*> CBaseMaterial::_setVariable(const std::string& name, const void* pValue, U32 size)
	{
		E_RESULT_CODE result = RC_FAIL;

		IMaterialInstance* pNewMaterialInstance = CreateInstance();

		if (!pNewMaterialInstance || (result = _setVariableForInstance(pNewMaterialInstance->GetInstanceId(), name, pValue, size)) != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<IMaterialInstance*>(pNewMaterialInstance);
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
		U32 varOffset = 0;
		std::tie(bufferIndex, varOffset) = iter->second; // first index is a buffer's id, the second one is variable's offset in bytes

		if (auto userUniformBuffersResult = mpInstancesUserUniformBuffers[static_cast<U32>(instanceId)])
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

		TMaterialInstanceId instanceId = static_cast<TMaterialInstanceId>(mpInstancesArray.Add(nullptr));

		IMaterialInstance* pNewMaterialInstance = CreateBaseMaterialInstance(this, instanceId, result);

		if ((result != RC_OK) || (result = mpInstancesArray.ReplaceAt(static_cast<U32>(instanceId), pNewMaterialInstance)) != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<TMaterialInstanceId>(instanceId);
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

		if (TResult<TFileEntryId> materialFileId = mpFileSystem->Open<IYAMLFileReader>(pResource->GetName()))
		{
			return dynamic_cast<IMaterial*>(pResource)->Load(mpFileSystem->Get<IYAMLFileReader>(materialFileId.Get()));
		}

		return RC_FILE_NOT_FOUND;
	}

	TypeId CBaseMaterialLoader::GetResourceTypeId() const
	{
		return CBaseMaterial::GetTypeId();
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

	TypeId CBaseMaterialFactory::GetResourceTypeId() const
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
		mId = id;

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