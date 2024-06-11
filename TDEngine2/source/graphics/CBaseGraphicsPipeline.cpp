#include "../../include/graphics/CBaseGraphicsPipeline.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/utils/CFileLogger.h"
#define META_EXPORT_GRAPHICS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	struct TGraphicsPipelineArchiveKeys
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

			static const std::string mWriteMaskKey;
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
			static const std::string mStencilRefKey;
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
	};

	const std::string TGraphicsPipelineArchiveKeys::mShaderIdKey = "shader_id";
	const std::string TGraphicsPipelineArchiveKeys::mTransparencyKey = "transparency_enabled";
	const std::string TGraphicsPipelineArchiveKeys::mBlendStateGroup = "blend_state";

	const std::string TGraphicsPipelineArchiveKeys::TBlendStateKeys::mSrcColorKey = "src_color";
	const std::string TGraphicsPipelineArchiveKeys::TBlendStateKeys::mDestColorKey = "dest_color";
	const std::string TGraphicsPipelineArchiveKeys::TBlendStateKeys::mOpTypeKey = "op_type";
	const std::string TGraphicsPipelineArchiveKeys::TBlendStateKeys::mSrcAlphaColorKey = "src_alpha";
	const std::string TGraphicsPipelineArchiveKeys::TBlendStateKeys::mDestAlphaColorKey = "dest_alpha";
	const std::string TGraphicsPipelineArchiveKeys::TBlendStateKeys::mOpAlphaTypeKey = "op_alpha_type";
	const std::string TGraphicsPipelineArchiveKeys::TBlendStateKeys::mWriteMaskKey = "write_mask";

	const std::string TGraphicsPipelineArchiveKeys::mDepthStencilStateGroup = "depth_stencil_state";

	const std::string TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mDepthTestKey = "depth_test_enabled";
	const std::string TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mDepthWriteKey = "depth_write_enabled";
	const std::string TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mDepthCmpFuncKey = "depth_cmp_func";
	const std::string TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilTestKey = "stencil_test_enabled";
	const std::string TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilReadMaskKey = "stencil_read_mask";
	const std::string TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilWriteMaskKey = "stencil_write_mask";
	const std::string TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilFrontOpGroup = "stencil_front_op";
	const std::string TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilBackOpGroup = "stencil_back_op";
	const std::string TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilRefKey = "stencil_ref";

	const std::string TGraphicsPipelineArchiveKeys::TStencilOpDescKeys::mComparisonFuncKey = "cmp_func";
	const std::string TGraphicsPipelineArchiveKeys::TStencilOpDescKeys::mStencilPassOpKey = "pass_op";
	const std::string TGraphicsPipelineArchiveKeys::TStencilOpDescKeys::mStencilFailOpKey = "fail_op";
	const std::string TGraphicsPipelineArchiveKeys::TStencilOpDescKeys::mDepthFailOpKey = "depth_fail_op";

	const std::string TGraphicsPipelineArchiveKeys::mRasterizerStateGroup = "rasterizer_state";

	const std::string TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mCullModeKey = "cull_mode";
	const std::string TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mWireframeModeKey = "wireframe_mode";
	const std::string TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mFrontCCWModeKey = "front_CCW_mode";
	const std::string TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mDepthBiasKey = "depth_bias";
	const std::string TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mMaxDepthBiasKey = "max_depth_bias";
	const std::string TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mDepthClippingKey = "depth_clipping_enabled";
	const std::string TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mScissorTestKey = "scissor_test_enabled";


	CBaseGraphicsPipeline::CBaseGraphicsPipeline():
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseGraphicsPipeline::Init(IGraphicsContext* pGraphicsContext, const TGraphicsPipelineConfigDesc& pipelineConfig)
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
		mpGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();

		mConfig = pipelineConfig;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseGraphicsPipeline::Bind()
	{
		if (!mpGraphicsObjectManager)
		{
			return RC_FAIL;
		}

		if (mBlendStateHandle == TBlendStateId::Invalid)
		{
			mBlendStateHandle = mpGraphicsObjectManager->CreateBlendState(mConfig.mBlendStateParams).Get();
		}

		mpGraphicsContext->BindBlendState(mBlendStateHandle);

		if (mDepthStencilStateHandle == TDepthStencilStateId::Invalid)
		{
			mDepthStencilStateHandle = mpGraphicsObjectManager->CreateDepthStencilState(mConfig.mDepthStencilStateParams).Get();
		}

		mpGraphicsContext->BindDepthStencilState(mDepthStencilStateHandle, mConfig.mDepthStencilStateParams.mStencilRefValue);

		if (mRasterizerStateHandle == TRasterizerStateId::Invalid)
		{
			mRasterizerStateHandle = mpGraphicsObjectManager->CreateRasterizerState(mConfig.mRasterizerStateParams).Get();
		}

		mpGraphicsContext->BindRasterizerState(mRasterizerStateHandle);

		return RC_OK;
	}

	const TGraphicsPipelineConfigDesc& CBaseGraphicsPipeline::GetConfig() const
	{
		return mConfig;
	}


	TDE2_API IGraphicsPipeline* CreateBaseGraphicsPipeline(IGraphicsContext* pGraphicsContext, const TGraphicsPipelineConfigDesc& pipelineConfig, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsPipeline, CBaseGraphicsPipeline, result, pGraphicsContext, pipelineConfig);
	}


	template <> TDE2_API TResult<TGraphicsPipelineConfigDesc> Deserialize<TGraphicsPipelineConfigDesc>(IArchiveReader* pReader)
	{
		TDE2_PROFILER_SCOPE("Deserialize<TGraphicsPipelineConfigDesc>");

		if (!pReader)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		auto processGroup = [pReader](const std::string& groupName, const std::function<void()>& actionCallback)
		{
			if (pReader->BeginGroup(groupName) == RC_OK)
			{
				actionCallback();
				pReader->EndGroup();

				return;
			}

			LOG_WARNING(Wrench::StringUtils::Format("[Deserialize<TGraphicsPipelineConfigDesc>] Missing \"{0}\" group of parameters", groupName));
		};

		TGraphicsPipelineConfigDesc config {};
		config.mShaderIdStr = pReader->GetString(TGraphicsPipelineArchiveKeys::mShaderIdKey);

		const bool isBlendingEnabled = pReader->GetBool(TGraphicsPipelineArchiveKeys::mTransparencyKey);

		processGroup(TGraphicsPipelineArchiveKeys::mBlendStateGroup, [pReader, isBlendingEnabled, &config]
			{
				TDE2_PROFILER_SCOPE("CBaseGraphicsPipeline::ProcessBlendGroup");
				TBlendStateDesc blendStateDesc;

				blendStateDesc.mScrValue = Meta::EnumTrait<E_BLEND_FACTOR_VALUE>::FromString(pReader->GetString(TGraphicsPipelineArchiveKeys::TBlendStateKeys::mSrcColorKey));
				blendStateDesc.mDestValue = Meta::EnumTrait<E_BLEND_FACTOR_VALUE>::FromString(pReader->GetString(TGraphicsPipelineArchiveKeys::TBlendStateKeys::mDestColorKey));
				blendStateDesc.mOpType = Meta::EnumTrait<E_BLEND_OP_TYPE>::FromString(pReader->GetString(TGraphicsPipelineArchiveKeys::TBlendStateKeys::mOpTypeKey));

				blendStateDesc.mScrAlphaValue = Meta::EnumTrait<E_BLEND_FACTOR_VALUE>::FromString(pReader->GetString(TGraphicsPipelineArchiveKeys::TBlendStateKeys::mSrcAlphaColorKey));
				blendStateDesc.mDestAlphaValue = Meta::EnumTrait<E_BLEND_FACTOR_VALUE>::FromString(pReader->GetString(TGraphicsPipelineArchiveKeys::TBlendStateKeys::mDestAlphaColorKey));
				blendStateDesc.mAlphaOpType = Meta::EnumTrait<E_BLEND_OP_TYPE>::FromString(pReader->GetString(TGraphicsPipelineArchiveKeys::TBlendStateKeys::mOpAlphaTypeKey));

				blendStateDesc.mWriteMask = pReader->GetUInt8(TGraphicsPipelineArchiveKeys::TBlendStateKeys::mWriteMaskKey, 0xF);
				blendStateDesc.mIsEnabled = isBlendingEnabled;

				config.mBlendStateParams = blendStateDesc;
			});

		processGroup(TGraphicsPipelineArchiveKeys::mDepthStencilStateGroup, [pReader, &config]
			{
				TDE2_PROFILER_SCOPE("CBaseGraphicsPipeline::ProcessDepthStencilGroup");

				TDepthStencilStateDesc depthStencilStateDesc;

				depthStencilStateDesc.mDepthCmpFunc = Meta::EnumTrait<E_COMPARISON_FUNC>::FromString(pReader->GetString(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mDepthCmpFuncKey));

				depthStencilStateDesc.mIsDepthTestEnabled = pReader->GetBool(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mDepthTestKey);
				depthStencilStateDesc.mIsDepthWritingEnabled = pReader->GetBool(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mDepthWriteKey);

				depthStencilStateDesc.mIsStencilTestEnabled = pReader->GetBool(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilTestKey);
				depthStencilStateDesc.mStencilReadMaskValue = pReader->GetUInt8(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilReadMaskKey);
				depthStencilStateDesc.mStencilWriteMaskValue = pReader->GetUInt8(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilWriteMaskKey);
				depthStencilStateDesc.mStencilRefValue = pReader->GetUInt8(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilRefKey);

				auto readStencilGroup = [pReader, &config](const std::string& groupId, TStencilOpDesc& desc)
				{
					pReader->BeginGroup(groupId);
					{
						desc.mFunc = Meta::EnumTrait<E_COMPARISON_FUNC>::FromString(pReader->GetString(TGraphicsPipelineArchiveKeys::TStencilOpDescKeys::mComparisonFuncKey));
						desc.mPassOp = Meta::EnumTrait<E_STENCIL_OP>::FromString(pReader->GetString(TGraphicsPipelineArchiveKeys::TStencilOpDescKeys::mStencilPassOpKey));
						desc.mFailOp = Meta::EnumTrait<E_STENCIL_OP>::FromString(pReader->GetString(TGraphicsPipelineArchiveKeys::TStencilOpDescKeys::mStencilFailOpKey));
						desc.mDepthFailOp = Meta::EnumTrait<E_STENCIL_OP>::FromString(pReader->GetString(TGraphicsPipelineArchiveKeys::TStencilOpDescKeys::mDepthFailOpKey));
					}
					pReader->EndGroup();
				};

				readStencilGroup(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilFrontOpGroup, depthStencilStateDesc.mStencilFrontFaceOp);
				readStencilGroup(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilBackOpGroup, depthStencilStateDesc.mStencilBackFaceOp);

				config.mDepthStencilStateParams = depthStencilStateDesc;
			});

		processGroup(TGraphicsPipelineArchiveKeys::mRasterizerStateGroup, [pReader, &config]
			{
				TDE2_PROFILER_SCOPE("CBaseGraphicsPipeline::ProcessRasterizerGroup");

				TRasterizerStateDesc rasterizerStateDesc;

				rasterizerStateDesc.mCullMode = Meta::EnumTrait<E_CULL_MODE>::FromString(pReader->GetString(TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mCullModeKey));

				rasterizerStateDesc.mIsWireframeModeEnabled = pReader->GetBool(TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mWireframeModeKey);
				rasterizerStateDesc.mIsScissorTestEnabled = pReader->GetBool(TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mScissorTestKey);
				rasterizerStateDesc.mIsFrontCCWEnabled = pReader->GetBool(TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mFrontCCWModeKey);
				rasterizerStateDesc.mDepthBias = pReader->GetFloat(TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mDepthBiasKey), pReader->GetFloat(TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mMaxDepthBiasKey);
				rasterizerStateDesc.mIsDepthClippingEnabled = pReader->GetBool(TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mDepthClippingKey);

				config.mRasterizerStateParams = rasterizerStateDesc;
			});

		return Wrench::TOkValue<TGraphicsPipelineConfigDesc>(config);
	}


	template <> TDE2_API E_RESULT_CODE Serialize<TGraphicsPipelineConfigDesc>(class IArchiveWriter* pWriter, TGraphicsPipelineConfigDesc value)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->SetString(TGraphicsPipelineArchiveKeys::mShaderIdKey, value.mShaderIdStr);
		pWriter->SetBool(TGraphicsPipelineArchiveKeys::mTransparencyKey, value.mBlendStateParams.mIsEnabled);

		const TBlendStateDesc& blendStateDesc = value.mBlendStateParams;

		pWriter->BeginGroup(TGraphicsPipelineArchiveKeys::mBlendStateGroup);
		{
			pWriter->SetString(TGraphicsPipelineArchiveKeys::TBlendStateKeys::mSrcColorKey, Meta::EnumTrait<E_BLEND_FACTOR_VALUE>::ToString(blendStateDesc.mScrValue));
			pWriter->SetString(TGraphicsPipelineArchiveKeys::TBlendStateKeys::mDestColorKey, Meta::EnumTrait<E_BLEND_FACTOR_VALUE>::ToString(blendStateDesc.mDestValue));
			pWriter->SetString(TGraphicsPipelineArchiveKeys::TBlendStateKeys::mOpTypeKey, Meta::EnumTrait<E_BLEND_OP_TYPE>::ToString(blendStateDesc.mOpType));

			pWriter->SetString(TGraphicsPipelineArchiveKeys::TBlendStateKeys::mSrcAlphaColorKey, Meta::EnumTrait<E_BLEND_FACTOR_VALUE>::ToString(blendStateDesc.mScrAlphaValue));
			pWriter->SetString(TGraphicsPipelineArchiveKeys::TBlendStateKeys::mDestAlphaColorKey, Meta::EnumTrait<E_BLEND_FACTOR_VALUE>::ToString(blendStateDesc.mDestAlphaValue));
			pWriter->SetString(TGraphicsPipelineArchiveKeys::TBlendStateKeys::mOpAlphaTypeKey, Meta::EnumTrait<E_BLEND_OP_TYPE>::ToString(blendStateDesc.mAlphaOpType));

			pWriter->SetUInt8(TGraphicsPipelineArchiveKeys::TBlendStateKeys::mWriteMaskKey, blendStateDesc.mWriteMask);
		}
		pWriter->EndGroup();

		const TDepthStencilStateDesc& depthStencilStateDesc = value.mDepthStencilStateParams;

		pWriter->BeginGroup(TGraphicsPipelineArchiveKeys::mDepthStencilStateGroup);
		{
			pWriter->SetBool(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mDepthTestKey, depthStencilStateDesc.mIsDepthTestEnabled);
			pWriter->SetBool(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mDepthWriteKey, depthStencilStateDesc.mIsDepthWritingEnabled);
			pWriter->SetString(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mDepthCmpFuncKey, Meta::EnumTrait<E_COMPARISON_FUNC>::ToString(depthStencilStateDesc.mDepthCmpFunc));

			pWriter->SetBool(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilTestKey, depthStencilStateDesc.mIsStencilTestEnabled);
			pWriter->SetUInt8(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilReadMaskKey, depthStencilStateDesc.mStencilReadMaskValue);
			pWriter->SetUInt8(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilWriteMaskKey, depthStencilStateDesc.mStencilWriteMaskValue);
			pWriter->SetUInt8(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilRefKey, depthStencilStateDesc.mStencilRefValue);

			auto writeStencilGroup = [pWriter](const std::string& groupId, const TStencilOpDesc& desc)
			{
				pWriter->BeginGroup(groupId);
				{
					pWriter->SetString(TGraphicsPipelineArchiveKeys::TStencilOpDescKeys::mComparisonFuncKey, Meta::EnumTrait<E_COMPARISON_FUNC>::ToString(desc.mFunc));
					pWriter->SetString(TGraphicsPipelineArchiveKeys::TStencilOpDescKeys::mStencilPassOpKey, Meta::EnumTrait<E_STENCIL_OP>::ToString(desc.mPassOp));
					pWriter->SetString(TGraphicsPipelineArchiveKeys::TStencilOpDescKeys::mStencilFailOpKey, Meta::EnumTrait<E_STENCIL_OP>::ToString(desc.mFailOp));
					pWriter->SetString(TGraphicsPipelineArchiveKeys::TStencilOpDescKeys::mDepthFailOpKey, Meta::EnumTrait<E_STENCIL_OP>::ToString(desc.mDepthFailOp));
				}
				pWriter->EndGroup();
			};

			writeStencilGroup(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilFrontOpGroup, depthStencilStateDesc.mStencilFrontFaceOp);
			writeStencilGroup(TGraphicsPipelineArchiveKeys::TDepthStencilStateKeys::mStencilBackOpGroup, depthStencilStateDesc.mStencilBackFaceOp);
		}
		pWriter->EndGroup();

		const TRasterizerStateDesc& rasterizerStateDesc = value.mRasterizerStateParams;

		pWriter->BeginGroup(TGraphicsPipelineArchiveKeys::mRasterizerStateGroup);
		{
			pWriter->SetString(TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mCullModeKey, Meta::EnumTrait<E_CULL_MODE>::ToString(rasterizerStateDesc.mCullMode));
			pWriter->SetBool(TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mWireframeModeKey, rasterizerStateDesc.mIsWireframeModeEnabled);
			pWriter->SetBool(TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mFrontCCWModeKey, rasterizerStateDesc.mIsFrontCCWEnabled);
			pWriter->SetFloat(TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mDepthBiasKey, rasterizerStateDesc.mDepthBias);
			pWriter->SetFloat(TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mMaxDepthBiasKey, rasterizerStateDesc.mMaxDepthBias);
			pWriter->SetBool(TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mDepthClippingKey, rasterizerStateDesc.mIsDepthClippingEnabled);
			pWriter->SetBool(TGraphicsPipelineArchiveKeys::TRasterizerStateKeys::mScissorTestKey, rasterizerStateDesc.mIsScissorTestEnabled);
		}
		pWriter->EndGroup();

		return RC_OK;
	}
}