#include "../../include/graphics/CForwardRenderer.h"
#include "../../include/graphics/ICamera.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/CGlobalShaderProperties.h"
#include "../../include/graphics/InternalShaderData.h"
#include "../../include/graphics/CDebugUtility.h"
#include "../../include/graphics/IMaterial.h"
#include "../../include/graphics/IShader.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/IPostProcessingProfile.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/CBaseRenderTarget.h"
#include "../../include/graphics/CBaseCubemapTexture.h"
#include "../../include/graphics/CFrameGraph.h"
#include "../../include/graphics/CFrameGraphResources.h"
#include "../../include/graphics/IGraphicsPipeline.h"
#include "../../include/core/memory/IAllocator.h"
#include "../../include/core/memory/CLinearAllocator.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/core/IEventManager.h"
#include "../../include/core/CGameUserSettings.h"
#include "../../include/core/IJobManager.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/editor/CStatsCounters.h"
#include "deferOperation.hpp"
#if TDE2_EDITORS_ENABLED
	#include "../../include/platform/CBaseFile.h"
	#include "../../include/platform/IOStreams.h"
	#include "../../include/platform/CTextFileWriter.h"
	#include "../../include/editor/ISelectionManager.h"
#endif


namespace TDEngine2
{
	CInt32ConsoleVarDecl EnableLightsHeatMapCfgVar("graphics.lights_heatmap_enabled", "", 0);
	CInt32ConsoleVarDecl TriggetDumpFrameGraph("graphics.dump_frame_graph", "", 0); // \todo For now triggers are not supported so mimic them using usual cvars


	constexpr U32 SUN_LIGHT_SHADOW_MAP_TEXTURE_SLOT = 12;
	constexpr U32 OMNI_LIGHT_SHADOW_MAP_START_SLOT = 13;

	constexpr U32 TILE_FRUSTUMS_BUFFER_SLOT = 0;
	constexpr U32 FRUSTUM_TILES_PER_GROUP = 16;

	constexpr U32 OPAQUE_VISIBLE_LIGHTS_BUFFER_SLOT = 11;
	constexpr U32 TRANSPARENT_VISIBLE_LIGHTS_BUFFER_SLOT = 12;
	constexpr U32 LIGHT_INDEX_COUNTERS_BUFFER_SLOT = 13;

	constexpr U32 LIGHT_GRID_TEXTURE_SLOT = 10;
	constexpr U32 READ_ONLY_DEPTH_TEXTURE_SLOT = 15;
	constexpr U32 VISIBLE_LIGHTS_BUFFER_SLOT = 11;

	constexpr U32 SUN_SHADOWS_MAP_SLOT = 0;
	constexpr U32 POINT_LIGHT_0_SHADOWS_MAP_SLOT = 1;


	static inline void ExecuteDrawCommands(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, 
		TPtr<IGlobalShaderProperties> pGlobalShaderProperties, TPtr<CRenderQueue> pCommandsBuffer, bool shouldClearBuffers, 
		U32 startRange = 0, U32 endRange = (std::numeric_limits<U32>::max)());


	struct TLightCullData
	{
		TFrameGraphResourceHandle mTileFrustumsBufferHandle = TFrameGraphResourceHandle::Invalid;

		TFrameGraphResourceHandle mOpaqueLightGridTextureHandle = TFrameGraphResourceHandle::Invalid;
		TFrameGraphResourceHandle mOpaqueVisibleLightsBufferHandle = TFrameGraphResourceHandle::Invalid;

		TFrameGraphResourceHandle mTransparentLightGridTextureHandle = TFrameGraphResourceHandle::Invalid;
		TFrameGraphResourceHandle mTransparentVisibleLightsBufferHandle = TFrameGraphResourceHandle::Invalid;

		TFrameGraphResourceHandle mLightIndexCountersBufferHandle = TFrameGraphResourceHandle::Invalid;
		TFrameGraphResourceHandle mLightIndexCountersInitializerBufferHandle = TFrameGraphResourceHandle::Invalid;
	};


	struct TFrameGraphBlackboard // \todo For now just use hardcoded struct with fields instead of special storage type
	{
#if TDE2_EDITORS_ENABLED
		TFrameGraphResourceHandle              mSelectionMapTargetHandle = TFrameGraphResourceHandle::Invalid;
#endif

		TFrameGraphResourceHandle              mBackBufferHandle = TFrameGraphResourceHandle::Invalid;

		TFrameGraphResourceHandle              mMainRenderTargetHandle = TFrameGraphResourceHandle::Invalid;
		TFrameGraphResourceHandle              mLDRMainRenderTargetHandle = TFrameGraphResourceHandle::Invalid;
		TFrameGraphResourceHandle              mDepthBufferHandle = TFrameGraphResourceHandle::Invalid;
		TFrameGraphResourceHandle              mReadonlyDepthBufferHandle = TFrameGraphResourceHandle::Invalid;

		TFrameGraphResourceHandle              mUIRenderTargetHandle = TFrameGraphResourceHandle::Invalid;

		TFrameGraphResourceHandle              mSunLightShadowMapHandle = TFrameGraphResourceHandle::Invalid;
		std::vector<TFrameGraphResourceHandle> mOmniLightShadowMapHandles;

		TFrameGraphResourceHandle              mLightsBufferHandle = TFrameGraphResourceHandle::Invalid;
		TLightCullData                         mLightCullingData;

		TFrameGraphResourceHandle              mVolumetricCloudsMainTargetHandle = TFrameGraphResourceHandle::Invalid;
		TFrameGraphResourceHandle              mVolumetricCloudsFullSizeTargetHandle = TFrameGraphResourceHandle::Invalid;

		TFrameGraphResourceHandle              mLuminanceTargetHandle = TFrameGraphResourceHandle::Invalid;
		TFrameGraphResourceHandle              mPrevAvgLuminanceTargetHandle = TFrameGraphResourceHandle::Invalid;
		TFrameGraphResourceHandle              mCurrAvgLuminanceTargetHandle = TFrameGraphResourceHandle::Invalid;

		TFrameGraphResourceHandle              mBloomThresholdTargetHandle = TFrameGraphResourceHandle::Invalid;
		TFrameGraphResourceHandle              mColorGradingLUTHandle = TFrameGraphResourceHandle::Invalid;
	};


	struct TPassInvokeContext
	{
		TPtr<IGraphicsContext>        mpGraphicsContext = nullptr;
		TPtr<IResourceManager>        mpResourceManager = nullptr;
		TPtr<IGlobalShaderProperties> mpGlobalShaderProperties = nullptr;

		U32                           mWindowWidth = 0;
		U32                           mWindowHeight = 0;
	};


	static const std::string FRONT_FRAME_TEXTURE_UNIFORM_ID = "FrameTexture";
	static const std::string BACK_FRAME_TEXTURE_UNIFORM_ID = "FrameTexture1";


	struct TFullScreenShaderInvokationConfig
	{
		TPtr<IGraphicsContext>   mpGraphicsContext = nullptr;
		TPtr<IResourceManager>   mpResourceManager = nullptr;
		
		TTextureHandleId         mSourceTarget = TTextureHandleId::Invalid;
		TTextureHandleId         mExtraTarget = TTextureHandleId::Invalid;		
		TTextureHandleId         mDestTarget = TTextureHandleId::Invalid;

		TGraphicsPipelineStateId mPipelineHandle = TGraphicsPipelineStateId::Invalid;
		
		std::string              mShaderId;

		U32                      mScreenWidth = 0;
		U32                      mScreenHeight = 0;

		TTextureSamplerId*       mpFrameTexture0SamplerId = nullptr; // \todo replace with std::optional later when C++20 standard will be enabled on the project
	};


	static void ExecuteFullScreenShader(const TFullScreenShaderInvokationConfig& config)
	{
		TDE2_ASSERT(config.mpGraphicsContext);
		TDE2_ASSERT(config.mpResourceManager);
		TDE2_ASSERT(TTextureHandleId::Invalid != config.mSourceTarget);
		TDE2_ASSERT(TGraphicsPipelineStateId::Invalid != config.mPipelineHandle);
		TDE2_ASSERT(!config.mShaderId.empty());
		TDE2_ASSERT(config.mScreenWidth > 0);
		TDE2_ASSERT(config.mScreenHeight > 0);

		auto pGraphicsContext = config.mpGraphicsContext;
		auto pResourceManager = config.mpResourceManager;

		pGraphicsContext->SetDepthBufferEnabled(false);
		pGraphicsContext->BindRenderTarget(0, config.mDestTarget);
		pGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(config.mScreenWidth), static_cast<F32>(config.mScreenHeight), 0.0f, 1.0f);

		const TTextureSamplerId linearSamplerHandle = pGraphicsContext->GetGraphicsObjectManager()->GetDefaultTextureSampler(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

		if (auto pShader = pResourceManager->GetResource<IShader>(pResourceManager->Load<IShader>(config.mShaderId)))
		{
			pShader->Bind();

			pGraphicsContext->SetTexture(pShader->GetResourceBindingSlot(FRONT_FRAME_TEXTURE_UNIFORM_ID), config.mSourceTarget);
			pGraphicsContext->SetSampler(pShader->GetResourceBindingSlot(FRONT_FRAME_TEXTURE_UNIFORM_ID), config.mpFrameTexture0SamplerId ? *config.mpFrameTexture0SamplerId : linearSamplerHandle);

			const U32 extraTextureSlotId = pShader->GetResourceBindingSlot(BACK_FRAME_TEXTURE_UNIFORM_ID);
			if (extraTextureSlotId < std::numeric_limits<U32>::max())
			{
				pGraphicsContext->SetTexture(extraTextureSlotId, TTextureHandleId::Invalid != config.mExtraTarget ? config.mExtraTarget : TTextureHandleId::Invalid);
				pGraphicsContext->SetSampler(extraTextureSlotId, linearSamplerHandle);
			}

			if (auto pDefaultPositionOnlyVertDeclaration = pGraphicsContext->GetGraphicsObjectManager()->GetDefaultPositionOnlyVertexDeclaration())
			{
				pDefaultPositionOnlyVertDeclaration->Bind(pGraphicsContext.Get(), { TBufferHandleId::Invalid }, pShader.Get());
			}
		}

		auto pGraphicsPipeline = pGraphicsContext->GetGraphicsObjectManager()->GetGraphicsPipeline(config.mPipelineHandle);
		if (!pGraphicsPipeline)
		{
			return;
		}

		pGraphicsPipeline->Bind();

		pGraphicsContext->Draw(E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST, 0, 3);

		pGraphicsContext->SetDepthBufferEnabled(true);
		pGraphicsContext->BindRenderTarget(0, nullptr);
	}


	class CBaseRenderPass
	{
		public:
			explicit CBaseRenderPass(const TPassInvokeContext& context, TPtr<CRenderQueue> pCommandsBuffer = nullptr) :
				mContext(context), mpCommandsBuffer(pCommandsBuffer)
			{
			}

		protected:
			TPtr<CRenderQueue> mpCommandsBuffer = nullptr;
			TPassInvokeContext mContext;
	};


#if TDE2_EDITORS_ENABLED

	class CRenderSelectionBufferPass : public CBaseRenderPass
	{
		public:
			explicit CRenderSelectionBufferPass(const TPassInvokeContext& context, TPtr<CRenderQueue> pCommandsBuffer) :
				CBaseRenderPass(context, pCommandsBuffer)
			{
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard, ISelectionManager* pSelectionManager)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mSelectionMapTargetHandle = TFrameGraphResourceHandle::Invalid;
				};

				auto&& output = pFrameGraph->AddPass<TPassData>("RenderSelectionBuffer", [&](CFrameGraphBuilder& builder, TPassData& data)
					{
						TFrameGraphTexture::TDesc selectionMapTargetParams{};

						selectionMapTargetParams.mWidth = mContext.mWindowWidth;
						selectionMapTargetParams.mHeight = mContext.mWindowHeight;
						selectionMapTargetParams.mFormat = FT_UINT1;
						selectionMapTargetParams.mNumOfMipLevels = 1;
						selectionMapTargetParams.mNumOfSamples = 1;
						selectionMapTargetParams.mSamplingQuality = 0;
						selectionMapTargetParams.mType = E_TEXTURE_IMPL_TYPE::TEXTURE_2D;
						selectionMapTargetParams.mUsageType = E_TEXTURE_IMPL_USAGE_TYPE::STATIC;
						selectionMapTargetParams.mBindFlags = E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE | E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET;
						selectionMapTargetParams.mName = "SelectionMapTarget";
						selectionMapTargetParams.mFlags = E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT;

						data.mSelectionMapTargetHandle = builder.Create<TFrameGraphTexture>(selectionMapTargetParams.mName, selectionMapTargetParams);
						data.mSelectionMapTargetHandle = builder.Write(data.mSelectionMapTargetHandle);

						builder.MarkAsPersistent(); // Mark as persistent because the output of the pass isn't used in other passes of the graph
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);

						TDE2_PROFILER_SCOPE("RenderSelectionBuffer");
						TDE_RENDER_SECTION(pGraphicsContext, renderPassName);

						TFrameGraphTexture& selectionMapTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mSelectionMapTargetHandle);

						pGraphicsContext->SetDepthBufferEnabled(false);
						pGraphicsContext->BindRenderTarget(0, selectionMapTarget.mTextureHandle);
						pGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(mContext.mWindowWidth), static_cast<F32>(mContext.mWindowHeight), 0.0f, 1.0f);

						pGraphicsContext->ClearRenderTarget(static_cast<U8>(0), TColor32F(0.0f));

						ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mpCommandsBuffer, true);

						pGraphicsContext->BindDepthBufferTarget(TTextureHandleId::Invalid);
						pGraphicsContext->BindRenderTarget(0, TTextureHandleId::Invalid);

						pSelectionManager->UpdateSelectionsBuffer(selectionMapTarget.mTextureHandle);
					});

				frameGraphBlackboard.mSelectionMapTargetHandle = output.mSelectionMapTargetHandle;
			}
	};

#endif


	class CDepthPrePass: public CBaseRenderPass
	{
		public:
			explicit CDepthPrePass(const TPassInvokeContext& context, TPtr<CRenderQueue> pCommandsBuffer):
				CBaseRenderPass(context, pCommandsBuffer)
			{
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard)
			{
				constexpr const C8* DEPTH_PRE_PASS_BUFFER_ID = "PrePassDepthBuffer";

				struct TPassData
				{
					TFrameGraphResourceHandle mDepthBufferHandle = TFrameGraphResourceHandle::Invalid;
					TFrameGraphResourceHandle mReadonlyDepthBufferHandle = TFrameGraphResourceHandle::Invalid;
				};

				auto&& output = pFrameGraph->AddPass<TPassData>("DepthPrePass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
				{
					TFrameGraphTexture::TDesc depthBufferParams{};

					depthBufferParams.mWidth           = mContext.mWindowWidth;
					depthBufferParams.mHeight          = mContext.mWindowHeight;
					depthBufferParams.mFormat          = FT_D32;
					depthBufferParams.mNumOfMipLevels  = 1;
					depthBufferParams.mNumOfSamples    = 1;
					depthBufferParams.mSamplingQuality = 0;
					depthBufferParams.mType            = E_TEXTURE_IMPL_TYPE::TEXTURE_2D;
					depthBufferParams.mUsageType       = E_TEXTURE_IMPL_USAGE_TYPE::STATIC;
					depthBufferParams.mBindFlags       = E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE | E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER;
					depthBufferParams.mName            = DEPTH_PRE_PASS_BUFFER_ID;
					depthBufferParams.mFlags           = E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT;

					data.mDepthBufferHandle = builder.Create<TFrameGraphTexture>(DEPTH_PRE_PASS_BUFFER_ID, depthBufferParams);
					data.mDepthBufferHandle = builder.Write(data.mDepthBufferHandle);

					depthBufferParams.mName = "ReadonlyDepthBufferCopy";

					data.mReadonlyDepthBufferHandle = builder.Create<TFrameGraphTexture>(depthBufferParams.mName, depthBufferParams);
					data.mReadonlyDepthBufferHandle = builder.Write(data.mReadonlyDepthBufferHandle);

					TDE2_ASSERT(data.mDepthBufferHandle != TFrameGraphResourceHandle::Invalid);
					TDE2_ASSERT(data.mReadonlyDepthBufferHandle != TFrameGraphResourceHandle::Invalid);
				}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
				{
					auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);

#if TDE2_DEBUG_MODE
					pGraphicsContext->EndSectionMarker();
#endif
					TDE2_PROFILER_SCOPE("DepthPrePass");
					TDE_RENDER_SECTION(pGraphicsContext, renderPassName);
					
					TFrameGraphTexture& depthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mDepthBufferHandle);
					TFrameGraphTexture& readOnlyDepthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mReadonlyDepthBufferHandle);

					pGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(mContext.mWindowWidth), static_cast<F32>(mContext.mWindowHeight), 0.0f, 1.0f);
					pGraphicsContext->BindDepthBufferTarget(depthBufferTarget.mTextureHandle, true);
					pGraphicsContext->ClearDepthBuffer(1.0f);

					ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mpCommandsBuffer, true);

					pGraphicsContext->BindDepthBufferTarget(TTextureHandleId::Invalid);

					pGraphicsContext->CopyResource(depthBufferTarget.mTextureHandle, readOnlyDepthBufferTarget.mTextureHandle);
				});

				frameGraphBlackboard.mDepthBufferHandle = output.mDepthBufferHandle;
				frameGraphBlackboard.mReadonlyDepthBufferHandle = output.mReadonlyDepthBufferHandle;
			}
	};


	class CSunLightShadowPass: public CBaseRenderPass
	{
		public:
			explicit CSunLightShadowPass(const TPassInvokeContext& context, TPtr<CRenderQueue> pCommandsBuffer) :
				CBaseRenderPass(context, pCommandsBuffer)
			{
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mShadowMapHandle = TFrameGraphResourceHandle::Invalid;
				};

				const U32 shadowMapSizes = static_cast<U32>(CGameUserSettings::Get()->mpShadowMapSizesCVar->Get());
				TDE2_ASSERT(shadowMapSizes > 0 && shadowMapSizes < 65536);

				auto&& output = pFrameGraph->AddPass<TPassData>("SunLightShadowPass", [&](CFrameGraphBuilder& builder, TPassData& data)
				{
					TFrameGraphTexture::TDesc shadowMapParams{};

					shadowMapParams.mWidth           = shadowMapSizes;
					shadowMapParams.mHeight          = shadowMapSizes;
					shadowMapParams.mFormat          = FT_D32;
					shadowMapParams.mNumOfMipLevels  = 1;
					shadowMapParams.mNumOfSamples    = 1;
					shadowMapParams.mSamplingQuality = 0;
					shadowMapParams.mType            = E_TEXTURE_IMPL_TYPE::TEXTURE_2D_ARRAY;
					shadowMapParams.mUsageType       = E_TEXTURE_IMPL_USAGE_TYPE::STATIC;
					shadowMapParams.mBindFlags       = E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE | E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER;
					shadowMapParams.mName            = "SunLightShadowMap";
					shadowMapParams.mFlags           = E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT;
					shadowMapParams.mArraySize       = CGameUserSettings::Get()->mpShadowCascadesCountCVar->Get();

					data.mShadowMapHandle = builder.Create<TFrameGraphTexture>(shadowMapParams.mName, shadowMapParams);
					data.mShadowMapHandle = builder.Write(data.mShadowMapHandle);

					TDE2_ASSERT(data.mShadowMapHandle != TFrameGraphResourceHandle::Invalid);
				}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
				{
					auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);

					TDE2_PROFILER_SCOPE("RenderSunLightShadows");

#if TDE2_DEBUG_MODE
					pGraphicsContext->BeginSectionMarker("RenderShadows");
#endif
					TDE_RENDER_SECTION(pGraphicsContext, "RenderSunLightShadows");

					TFrameGraphTexture& shadowMapTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mShadowMapHandle);

					pGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(shadowMapSizes), static_cast<F32>(shadowMapSizes), 0.0f, 1.0f);
					pGraphicsContext->BindDepthBufferTarget(shadowMapTarget.mTextureHandle, true);
					pGraphicsContext->ClearDepthBuffer(1.0f);

					TPtr<IResourceManager> pResourceManager = mContext.mpResourceManager;

					/// \todo Replace hardcoded identifiers
					const TResourceId shadowPassMaterialHandle = pResourceManager->Load<IMaterial>("ShadowPassMaterial.material");
					
					/// \todo Assign "ShadowPass" material's variables that defines which type of a light casts shadows
					if (auto pMaterial = pResourceManager->GetResource<IMaterial>(shadowPassMaterialHandle))
					{
						pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, "mIsSunLight", 1);
					}

					ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mpCommandsBuffer, false);

					pGraphicsContext->BindDepthBufferTarget(TTextureHandleId::Invalid);
				});

				frameGraphBlackboard.mSunLightShadowMapHandle = output.mShadowMapHandle;
			}
	};


	class COmniLightShadowPass: public CBaseRenderPass
	{
		public:
			explicit COmniLightShadowPass(const TPassInvokeContext& context, TPtr<CRenderQueue> pCommandsBuffer, USIZE pointLightsCount) :
				CBaseRenderPass(context, pCommandsBuffer)
			{
				mShadowMapsIdentifiers.resize(pointLightsCount);
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard, U32 lightIndex = 0)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mShadowMapHandle = TFrameGraphResourceHandle::Invalid;
				};

				const U32 shadowMapSizes = static_cast<U32>(CGameUserSettings::Get()->mpShadowMapSizesCVar->Get());
				TDE2_ASSERT(shadowMapSizes > 0 && shadowMapSizes < 65536);

				auto&& output = pFrameGraph->AddPass<TPassData>(Wrench::StringUtils::Format("RenderOmniLightShadows{0}", lightIndex), [&](CFrameGraphBuilder& builder, TPassData& data)
					{
						TFrameGraphTexture::TDesc shadowMapParams{};

						mShadowMapsIdentifiers[lightIndex] = Wrench::StringUtils::Format("OmniLightShadowMap_{0}", lightIndex);

						shadowMapParams.mWidth           = shadowMapSizes;
						shadowMapParams.mHeight          = shadowMapSizes;
						shadowMapParams.mFormat          = FT_D32;
						shadowMapParams.mNumOfMipLevels  = 1;
						shadowMapParams.mNumOfSamples    = 1;
						shadowMapParams.mSamplingQuality = 0;
						shadowMapParams.mType            = E_TEXTURE_IMPL_TYPE::CUBEMAP;
						shadowMapParams.mUsageType       = E_TEXTURE_IMPL_USAGE_TYPE::STATIC;
						shadowMapParams.mBindFlags       = E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE | E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER;
						shadowMapParams.mName            = mShadowMapsIdentifiers[lightIndex].c_str();
						shadowMapParams.mFlags           = E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT;

						data.mShadowMapHandle = builder.Create<TFrameGraphTexture>(shadowMapParams.mName, shadowMapParams);
						data.mShadowMapHandle = builder.Write(data.mShadowMapHandle);

						TDE2_ASSERT(data.mShadowMapHandle != TFrameGraphResourceHandle::Invalid);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);

						TDE2_PROFILER_SCOPE("RenderOmniLightShadows" + std::to_string(lightIndex));
						TDE_RENDER_SECTION(pGraphicsContext, renderPassName);

						TFrameGraphTexture& shadowMapTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mShadowMapHandle);

						pGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(shadowMapSizes), static_cast<F32>(shadowMapSizes), 0.0f, 1.0f);
						pGraphicsContext->BindDepthBufferTarget(shadowMapTarget.mTextureHandle, true);
						pGraphicsContext->ClearDepthBuffer(1.0f);

						TPtr<IResourceManager> pResourceManager = mContext.mpResourceManager;

						/// \todo Replace hardcoded identifiers
						const TResourceId shadowPassMaterialHandle = pResourceManager->Load<IMaterial>("ShadowPassMaterial.material");

						/// \todo Assign "ShadowPass" material's variables that defines which type of a light casts shadows
						if (auto pMaterial = pResourceManager->GetResource<IMaterial>(shadowPassMaterialHandle))
						{
							pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, "mIsSunLight", 0);
							pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, "mPointLightIndex", lightIndex);
						}

						ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mpCommandsBuffer, false);

						pGraphicsContext->BindDepthBufferTarget(TTextureHandleId::Invalid);
					});

				frameGraphBlackboard.mOmniLightShadowMapHandles.emplace_back(output.mShadowMapHandle);
			}

		private:
			static std::vector<std::string> mShadowMapsIdentifiers;
	};

	std::vector<std::string> COmniLightShadowPass::mShadowMapsIdentifiers {};


	class CUploadLightsPass : public CBaseRenderPass
	{
		public:
			explicit CUploadLightsPass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
			{
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard, const std::vector<TLightData>& lights)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mLightsBufferHandle = TFrameGraphResourceHandle::Invalid;
				};

				auto&& output = pFrameGraph->AddPass<TPassData>("UploadLightsPass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{
						TFrameGraphBuffer::TDesc lightsBufferParams{};
						lightsBufferParams.mBufferType                = E_BUFFER_TYPE::STRUCTURED;
						lightsBufferParams.mDataSize                  = MaxLightsCount * sizeof(TLightData);
						lightsBufferParams.mElementStrideSize         = sizeof(TLightData);
						lightsBufferParams.mFlags                     = E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT;
						lightsBufferParams.mIsUnorderedAccessResource = false;
						lightsBufferParams.mName                      = "ActiveLightsBuffer";
						lightsBufferParams.mpDataPtr                  = nullptr;
						lightsBufferParams.mStructuredBufferType      = E_STRUCTURED_BUFFER_TYPE::DEFAULT;
						lightsBufferParams.mTotalBufferSize           = lightsBufferParams.mDataSize;
						lightsBufferParams.mUsageType                 = E_BUFFER_USAGE_TYPE::DYNAMIC;

						data.mLightsBufferHandle = builder.Create<TFrameGraphBuffer>(lightsBufferParams.mName, lightsBufferParams);
						data.mLightsBufferHandle = builder.Write(data.mLightsBufferHandle);

						TDE2_ASSERT(data.mLightsBufferHandle != TFrameGraphResourceHandle::Invalid);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);

						TDE2_PROFILER_SCOPE("UploadLightsPass");

						constexpr U32 LIGHT_SLOT_INDEX = static_cast<U32>(E_INTERNAL_SHADER_BUFFERS_REGISTERS::LIGHTS_SLOT);

						TFrameGraphBuffer& lightsBuffer = executionContext.mpOwnerGraph->GetResource<TFrameGraphBuffer>(data.mLightsBufferHandle);
						
						if (auto pCurrTypedBuffer = mContext.mpGraphicsContext->GetGraphicsObjectManager()->GetBufferPtr(lightsBuffer.mBufferHandle))
						{
							E_RESULT_CODE result = pCurrTypedBuffer->Map(BMT_WRITE_DISCARD);
							if (result != RC_OK)
							{
								return;
							}

							if ((result = pCurrTypedBuffer->Write(reinterpret_cast<const U8*>(lights.data()), static_cast<U32>(lights.size() * sizeof(TLightData)))) != RC_OK)
							{
								return;
							}

							pCurrTypedBuffer->Unmap();

							pGraphicsContext->SetStructuredBuffer(LIGHT_SLOT_INDEX, lightsBuffer.mBufferHandle);
						}						
					});

				frameGraphBlackboard.mLightsBufferHandle = output.mLightsBufferHandle;
			}
	};


	class CLightCullingPass : public CBaseRenderPass
	{
		public:
			explicit CLightCullingPass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
			{
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard)
			{
				TLightCullData& lightCullingData = frameGraphBlackboard.mLightCullingData;
				TFrameGraphResourceHandle depthBufferResourceHandle = frameGraphBlackboard.mDepthBufferHandle;

				const U32 workGroupsX = (mContext.mWindowWidth + (mContext.mWindowWidth % LIGHT_GRID_TILE_BLOCK_SIZE)) / LIGHT_GRID_TILE_BLOCK_SIZE;
				const U32 workGroupsY = (mContext.mWindowHeight + (mContext.mWindowHeight % LIGHT_GRID_TILE_BLOCK_SIZE)) / LIGHT_GRID_TILE_BLOCK_SIZE;

				auto&& output = pFrameGraph->AddPass<TLightCullData>("LightCullingPass", [&, this](CFrameGraphBuilder& builder, TLightCullData& data)
					{
						builder.Read(frameGraphBlackboard.mDepthBufferHandle);
						builder.Read(frameGraphBlackboard.mLightsBufferHandle);
						builder.Read(lightCullingData.mTileFrustumsBufferHandle);
						data.mLightIndexCountersInitializerBufferHandle = builder.Read(lightCullingData.mLightIndexCountersInitializerBufferHandle);

						if (auto opaqueLightCullingStructsResult = _initLightGridAndBuffer(mContext, builder, workGroupsX, workGroupsY, "OpaqueVisibleLightsBuffer", "OpaqueLightGridTexture"))
						{
							std::tie(data.mOpaqueVisibleLightsBufferHandle, data.mOpaqueLightGridTextureHandle) = opaqueLightCullingStructsResult.Get();
						}

						if (auto transparentLightCullingStructsResult = _initLightGridAndBuffer(mContext, builder, workGroupsX, workGroupsY, "TransparentVisibleLightsBuffer", "TransparentLightGridTexture"))
						{
							std::tie(data.mTransparentVisibleLightsBufferHandle, data.mTransparentLightGridTextureHandle) = transparentLightCullingStructsResult.Get();
						}

						// \note Create a light index counters buffer
						TFrameGraphBuffer::TDesc lightIndexCountersBufferParams{};

						lightIndexCountersBufferParams.mBufferType                = E_BUFFER_TYPE::STRUCTURED;
						lightIndexCountersBufferParams.mDataSize                  = 4 * sizeof(U32);
						lightIndexCountersBufferParams.mElementStrideSize         = sizeof(U32);
						lightIndexCountersBufferParams.mFlags                     = E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT;
						lightIndexCountersBufferParams.mIsUnorderedAccessResource = true;
						lightIndexCountersBufferParams.mpDataPtr                  = nullptr;
						lightIndexCountersBufferParams.mName                      = "LightIndexCountersBuffer";
						lightIndexCountersBufferParams.mStructuredBufferType      = E_STRUCTURED_BUFFER_TYPE::DEFAULT;
						lightIndexCountersBufferParams.mTotalBufferSize           = lightIndexCountersBufferParams.mDataSize;
						lightIndexCountersBufferParams.mUsageType                 = E_BUFFER_USAGE_TYPE::DEFAULT;

						data.mLightIndexCountersBufferHandle = builder.Create<TFrameGraphBuffer>(lightIndexCountersBufferParams.mName, lightIndexCountersBufferParams);
						data.mLightIndexCountersBufferHandle = builder.Write(data.mLightIndexCountersBufferHandle);
					}, [=](const TLightCullData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);

#if TDE2_DEBUG_MODE
						pGraphicsContext->EndSectionMarker();
#endif

						TDE2_PROFILER_SCOPE("LightCullingPass");
						TDE_RENDER_SECTION(pGraphicsContext, "LightCullingPass");

						E_RESULT_CODE result = RC_OK;

						defer([&result] { TDE2_ASSERT(RC_OK == result); });

						TFrameGraphBuffer& opaqueVisibleLightsBuffer = executionContext.mpOwnerGraph->GetResource<TFrameGraphBuffer>(data.mOpaqueVisibleLightsBufferHandle);
						TFrameGraphBuffer& transparentVisibleLightsBuffer = executionContext.mpOwnerGraph->GetResource<TFrameGraphBuffer>(data.mTransparentVisibleLightsBufferHandle);
						TFrameGraphBuffer& lightIndexCountersBufferHandle = executionContext.mpOwnerGraph->GetResource<TFrameGraphBuffer>(data.mLightIndexCountersBufferHandle);
						TFrameGraphBuffer& lightIndexInitCountersBufferHandle = executionContext.mpOwnerGraph->GetResource<TFrameGraphBuffer>(data.mLightIndexCountersInitializerBufferHandle);
						
						result = result | pGraphicsContext->CopyResource(lightIndexInitCountersBufferHandle.mBufferHandle, lightIndexCountersBufferHandle.mBufferHandle);

						//// \note Cull lighting shader
						result = result | pGraphicsContext->SetStructuredBuffer(OPAQUE_VISIBLE_LIGHTS_BUFFER_SLOT, opaqueVisibleLightsBuffer.mBufferHandle, true);
						result = result | pGraphicsContext->SetStructuredBuffer(TRANSPARENT_VISIBLE_LIGHTS_BUFFER_SLOT, transparentVisibleLightsBuffer.mBufferHandle, true);
						result = result | pGraphicsContext->SetStructuredBuffer(LIGHT_INDEX_COUNTERS_BUFFER_SLOT, lightIndexCountersBufferHandle.mBufferHandle, true);

						struct
						{
							U32 mWorkGroupsX = 0;
							U32 mWorkGroupsY = 0;
						} shaderParameters;

						shaderParameters.mWorkGroupsX = workGroupsX;
						shaderParameters.mWorkGroupsY = workGroupsY;

						auto&& pResourceManager = mContext.mpResourceManager;

						TFrameGraphTexture& opaqueLightGridTextureHandle = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mOpaqueLightGridTextureHandle);
						TFrameGraphTexture& transparentLightGridTextureHandle = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mTransparentLightGridTextureHandle);
						TFrameGraphTexture& depthBufferHandle = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(depthBufferResourceHandle);

						TFrameGraphBuffer& tileFrustumsBufferHandle = executionContext.mpOwnerGraph->GetResource<TFrameGraphBuffer>(lightCullingData.mTileFrustumsBufferHandle);

						auto pLightCullShader = pResourceManager->GetResource<IShader>(pResourceManager->Load<IShader>("Shaders/Default/ForwardLightCulling.cshader"));
						if (pLightCullShader)
						{
							const TTextureSamplerId linearSamplerHandle = pGraphicsContext->GetGraphicsObjectManager()->GetDefaultTextureSampler(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

							// \todo Refactor access to binding slots values
							pGraphicsContext->SetTexture(pLightCullShader->GetResourceBindingSlot("OpaqueLightGridTexture"), opaqueLightGridTextureHandle.mTextureHandle, true);
							pGraphicsContext->SetSampler(pLightCullShader->GetResourceBindingSlot("OpaqueLightGridTexture"), linearSamplerHandle);
							pGraphicsContext->SetTexture(pLightCullShader->GetResourceBindingSlot("TransparentLightGridTexture"), transparentLightGridTextureHandle.mTextureHandle, true);
							pGraphicsContext->SetSampler(pLightCullShader->GetResourceBindingSlot("TransparentLightGridTexture"), linearSamplerHandle);
							pGraphicsContext->SetTexture(pLightCullShader->GetResourceBindingSlot("DepthTexture"), depthBufferHandle.mTextureHandle);
							pGraphicsContext->SetSampler(pLightCullShader->GetResourceBindingSlot("DepthTexture"), linearSamplerHandle);

							pLightCullShader->SetStructuredBufferResource("TileFrustums", tileFrustumsBufferHandle.mBufferHandle);
							pLightCullShader->SetUserUniformsBuffer(0, reinterpret_cast<U8*>(&shaderParameters), sizeof(shaderParameters));

							pLightCullShader->Bind();
						}

						pGraphicsContext->DispatchCompute(workGroupsX, workGroupsY, 1);

						if (pLightCullShader)
						{
							pGraphicsContext->SetTexture(pLightCullShader->GetResourceBindingSlot("OpaqueLightGridTexture"), TTextureHandleId::Invalid, true);
							pGraphicsContext->SetTexture(pLightCullShader->GetResourceBindingSlot("TransparentLightGridTexture"), TTextureHandleId::Invalid, true);
							pGraphicsContext->SetTexture(pLightCullShader->GetResourceBindingSlot("DepthTexture"), TTextureHandleId::Invalid);
						}

						// unbind lights indices buffers
						result = result | pGraphicsContext->SetStructuredBuffer(OPAQUE_VISIBLE_LIGHTS_BUFFER_SLOT, TBufferHandleId::Invalid, true);
						result = result | pGraphicsContext->SetStructuredBuffer(TRANSPARENT_VISIBLE_LIGHTS_BUFFER_SLOT, TBufferHandleId::Invalid, true);
						result = result | pGraphicsContext->SetStructuredBuffer(LIGHT_INDEX_COUNTERS_BUFFER_SLOT, TBufferHandleId::Invalid, true);

					});

				lightCullingData.mLightIndexCountersBufferHandle       = output.mLightIndexCountersBufferHandle;
				lightCullingData.mOpaqueLightGridTextureHandle         = output.mOpaqueLightGridTextureHandle;
				lightCullingData.mOpaqueVisibleLightsBufferHandle      = output.mOpaqueVisibleLightsBufferHandle;
				lightCullingData.mTransparentLightGridTextureHandle    = output.mTransparentLightGridTextureHandle;
				lightCullingData.mTransparentVisibleLightsBufferHandle = output.mTransparentVisibleLightsBufferHandle;
			}
		private:
			static TResult<std::tuple<TFrameGraphResourceHandle, TFrameGraphResourceHandle>> _initLightGridAndBuffer(const TPassInvokeContext& context, CFrameGraphBuilder& builder, U32 workGroupsX, U32 workGroupsY, 
				const C8* visibleLightsBufferName, const C8* lightGridTextureName)
			{
				const U32 tilesCount = workGroupsX * workGroupsY;
				const USIZE bufferSize = sizeof(U32) * tilesCount * MAX_LIGHTS_PER_TILE_BLOCK;

				// \note Create lights indices buffer
				TFrameGraphBuffer::TDesc visibleLightsBufferParams{};

				visibleLightsBufferParams.mBufferType = E_BUFFER_TYPE::STRUCTURED;
				visibleLightsBufferParams.mDataSize = bufferSize;
				visibleLightsBufferParams.mElementStrideSize = sizeof(U32);
				visibleLightsBufferParams.mFlags = E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT;
				visibleLightsBufferParams.mIsUnorderedAccessResource = true;
				visibleLightsBufferParams.mpDataPtr = nullptr;
				visibleLightsBufferParams.mName = visibleLightsBufferName;
				visibleLightsBufferParams.mStructuredBufferType = E_STRUCTURED_BUFFER_TYPE::DEFAULT;
				visibleLightsBufferParams.mTotalBufferSize = visibleLightsBufferParams.mDataSize;
				visibleLightsBufferParams.mUsageType = E_BUFFER_USAGE_TYPE::DEFAULT;

				TFrameGraphResourceHandle visibleLightsBufferHandle = builder.Create<TFrameGraphBuffer>(visibleLightsBufferName, visibleLightsBufferParams);
				visibleLightsBufferHandle = builder.Write(visibleLightsBufferHandle);

				// \note Create the light grid's texture
				TFrameGraphTexture::TDesc lightGridTextureParams{};

				lightGridTextureParams.mWidth = workGroupsX;
				lightGridTextureParams.mHeight = workGroupsY;
				lightGridTextureParams.mFormat = FT_UINT2;
				lightGridTextureParams.mNumOfMipLevels = 1;
				lightGridTextureParams.mNumOfSamples = 1;
				lightGridTextureParams.mSamplingQuality = 0;
				lightGridTextureParams.mType = E_TEXTURE_IMPL_TYPE::TEXTURE_2D;
				lightGridTextureParams.mUsageType = E_TEXTURE_IMPL_USAGE_TYPE::STATIC;
				lightGridTextureParams.mBindFlags = E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE | E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS;
				lightGridTextureParams.mName = lightGridTextureName;
				lightGridTextureParams.mFlags = E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT;
				lightGridTextureParams.mIsWriteable = true;

				TFrameGraphResourceHandle lightGridTextureHandle = builder.Create<TFrameGraphTexture>(lightGridTextureName, lightGridTextureParams);
				lightGridTextureHandle = builder.Write(lightGridTextureHandle);

				return Wrench::TOkValue<std::tuple<TFrameGraphResourceHandle, TFrameGraphResourceHandle>>(std::make_tuple(visibleLightsBufferHandle, lightGridTextureHandle));
			}
	};


	class COpaqueRenderPass : public CBaseRenderPass
	{
		public:
			explicit COpaqueRenderPass(const TPassInvokeContext& context, TPtr<CRenderQueue> pCommandsBuffer) :
				CBaseRenderPass(context, pCommandsBuffer)
			{
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard, bool isHDRSupportEnabled)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mMainRenderTargetHandle = TFrameGraphResourceHandle::Invalid;
				};

				auto& lightCullData = frameGraphBlackboard.mLightCullingData;

				auto&& output = pFrameGraph->AddPass<TPassData>("OpaqueRenderPass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{
						builder.Read(lightCullData.mOpaqueVisibleLightsBufferHandle);
						builder.Read(lightCullData.mOpaqueLightGridTextureHandle);
						builder.Read(frameGraphBlackboard.mDepthBufferHandle);
						builder.Read(frameGraphBlackboard.mSunLightShadowMapHandle);

						for (auto&& currOmniLightTargetHandle : frameGraphBlackboard.mOmniLightShadowMapHandles)
						{
							builder.Read(currOmniLightTargetHandle);
						}

						data.mMainRenderTargetHandle = builder.Write(builder.Read(frameGraphBlackboard.mMainRenderTargetHandle));

						TDE2_ASSERT(data.mMainRenderTargetHandle != TFrameGraphResourceHandle::Invalid);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("OpaqueRenderPass");

#if TDE2_DEBUG_MODE
						pGraphicsContext->BeginSectionMarker("RenderGeometry");
#endif
						TDE_RENDER_SECTION(pGraphicsContext, "OpaqueRenderPass");

						TFrameGraphTexture& mainRenderTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mMainRenderTargetHandle);
						TFrameGraphTexture& depthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mDepthBufferHandle);

						pGraphicsContext->BindRenderTarget(0, mainRenderTarget.mTextureHandle);
						pGraphicsContext->BindDepthBufferTarget(depthBufferTarget.mTextureHandle);

						TFrameGraphBuffer& opaqueVisibleLightsBuffer = executionContext.mpOwnerGraph->GetResource<TFrameGraphBuffer>(lightCullData.mOpaqueVisibleLightsBufferHandle);
						TFrameGraphTexture& opaqueLightGridTexture = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(lightCullData.mOpaqueLightGridTextureHandle);

						TFrameGraphTexture& sunLightShadowMapTexture = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mSunLightShadowMapHandle);

						pGraphicsContext->SetStructuredBuffer(VISIBLE_LIGHTS_BUFFER_SLOT, opaqueVisibleLightsBuffer.mBufferHandle, false);
						pGraphicsContext->SetTexture(LIGHT_GRID_TEXTURE_SLOT, opaqueLightGridTexture.mTextureHandle, false);

						const TTextureSamplerId linearSamplerHandle = pGraphicsContext->GetGraphicsObjectManager()->GetDefaultTextureSampler(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

						pGraphicsContext->SetTexture(SUN_LIGHT_SHADOW_MAP_TEXTURE_SLOT, sunLightShadowMapTexture.mTextureHandle, false);
						pGraphicsContext->SetSampler(SUN_LIGHT_SHADOW_MAP_TEXTURE_SLOT, linearSamplerHandle);

						for (USIZE i = 0; i < frameGraphBlackboard.mOmniLightShadowMapHandles.size(); i++)
						{
							TFrameGraphTexture& omniLightShadowMapTexture = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mOmniLightShadowMapHandles[i]);

							pGraphicsContext->SetTexture(OMNI_LIGHT_SHADOW_MAP_START_SLOT + static_cast<U32>(i), omniLightShadowMapTexture.mTextureHandle, false);
							pGraphicsContext->SetSampler(OMNI_LIGHT_SHADOW_MAP_START_SLOT + static_cast<U32>(i), linearSamplerHandle);
						}

						ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mpCommandsBuffer, true, 1);

						pGraphicsContext->BindDepthBufferTarget(TTextureHandleId::Invalid);
						pGraphicsContext->BindRenderTarget(0, TTextureHandleId::Invalid);
					});

				frameGraphBlackboard.mMainRenderTargetHandle = output.mMainRenderTargetHandle;
			}
	};


	class CTransparentRenderPass : public CBaseRenderPass
	{
		public:
			explicit CTransparentRenderPass(const TPassInvokeContext& context, TPtr<CRenderQueue> pCommandsBuffer) :
				CBaseRenderPass(context, pCommandsBuffer)
			{
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mMainRenderTargetHandle = TFrameGraphResourceHandle::Invalid;
				};

				auto& lightCullData = frameGraphBlackboard.mLightCullingData;

				auto&& output = pFrameGraph->AddPass<TPassData>("TransparentRenderPass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{
						builder.Read(lightCullData.mTransparentVisibleLightsBufferHandle);
						builder.Read(lightCullData.mTransparentLightGridTextureHandle);
						builder.Read(frameGraphBlackboard.mDepthBufferHandle);
						builder.Read(frameGraphBlackboard.mReadonlyDepthBufferHandle);
						builder.Read(frameGraphBlackboard.mMainRenderTargetHandle);

						data.mMainRenderTargetHandle = builder.Write(frameGraphBlackboard.mMainRenderTargetHandle);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("TransparentRenderPass");
						TDE_RENDER_SECTION(pGraphicsContext, "TransparentRenderPass");

						TFrameGraphTexture& mainRenderTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mMainRenderTargetHandle);
						TFrameGraphTexture& depthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mDepthBufferHandle);
						TFrameGraphTexture& readonlyDepthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mReadonlyDepthBufferHandle); // contains same data as depth buffer but used for effects that needs depth data

						pGraphicsContext->BindRenderTarget(0, mainRenderTarget.mTextureHandle);
						pGraphicsContext->BindDepthBufferTarget(depthBufferTarget.mTextureHandle);

						TFrameGraphBuffer& transparentVisibleLightsBuffer = executionContext.mpOwnerGraph->GetResource<TFrameGraphBuffer>(lightCullData.mTransparentVisibleLightsBufferHandle);
						TFrameGraphTexture& transparentLightGridTexture = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(lightCullData.mTransparentLightGridTextureHandle);

						pGraphicsContext->SetStructuredBuffer(VISIBLE_LIGHTS_BUFFER_SLOT, transparentVisibleLightsBuffer.mBufferHandle, false);
						pGraphicsContext->SetTexture(LIGHT_GRID_TEXTURE_SLOT, transparentLightGridTexture.mTextureHandle, false);
						pGraphicsContext->SetTexture(READ_ONLY_DEPTH_TEXTURE_SLOT, readonlyDepthBufferTarget.mTextureHandle, false);

						ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mpCommandsBuffer, true);

						pGraphicsContext->BindDepthBufferTarget(TTextureHandleId::Invalid);
						pGraphicsContext->BindRenderTarget(0, TTextureHandleId::Invalid);
					});

				frameGraphBlackboard.mMainRenderTargetHandle = output.mMainRenderTargetHandle;
			}
	};


	class CSpritesRenderPass : public CBaseRenderPass
	{
		public:
			explicit CSpritesRenderPass(const TPassInvokeContext& context, TPtr<CRenderQueue> pCommandsBuffer) :
				CBaseRenderPass(context, pCommandsBuffer)
			{
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mMainRenderTargetHandle = TFrameGraphResourceHandle::Invalid;
				};

				auto&& output = pFrameGraph->AddPass<TPassData>("SpritesRenderPass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{
						builder.Read(frameGraphBlackboard.mDepthBufferHandle);
						builder.Read(frameGraphBlackboard.mMainRenderTargetHandle);

						data.mMainRenderTargetHandle = builder.Write(frameGraphBlackboard.mMainRenderTargetHandle);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("SpritesRenderPass");
						TDE_RENDER_SECTION(pGraphicsContext, "SpritesRenderPass");

						TFrameGraphTexture& mainRenderTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mMainRenderTargetHandle);
						TFrameGraphTexture& depthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mDepthBufferHandle);

						pGraphicsContext->BindRenderTarget(0, mainRenderTarget.mTextureHandle);
						pGraphicsContext->BindDepthBufferTarget(depthBufferTarget.mTextureHandle);

						ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mpCommandsBuffer, true);

						pGraphicsContext->BindDepthBufferTarget(TTextureHandleId::Invalid);
						pGraphicsContext->BindRenderTarget(0, TTextureHandleId::Invalid);
					});

				frameGraphBlackboard.mMainRenderTargetHandle = output.mMainRenderTargetHandle;
			}
	};


	class CDebugRenderPass : public CBaseRenderPass
	{
		public:
			explicit CDebugRenderPass(const TPassInvokeContext& context, TPtr<CRenderQueue> pCommandsBuffer) :
				CBaseRenderPass(context, pCommandsBuffer)
			{
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mMainRenderTargetHandle = TFrameGraphResourceHandle::Invalid;
				};

				auto&& output = pFrameGraph->AddPass<TPassData>("DebugRenderPass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{
						builder.Read(frameGraphBlackboard.mDepthBufferHandle);
						builder.Read(frameGraphBlackboard.mMainRenderTargetHandle);

						data.mMainRenderTargetHandle = builder.Write(frameGraphBlackboard.mMainRenderTargetHandle);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("DebugRenderPass");
						TDE_RENDER_SECTION(pGraphicsContext, "DebugRenderPass");

						TFrameGraphTexture& mainRenderTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mMainRenderTargetHandle);
						TFrameGraphTexture& depthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mDepthBufferHandle);

						pGraphicsContext->BindRenderTarget(0, mainRenderTarget.mTextureHandle);
						pGraphicsContext->BindDepthBufferTarget(depthBufferTarget.mTextureHandle);

						ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mpCommandsBuffer, true);

						pGraphicsContext->BindDepthBufferTarget(TTextureHandleId::Invalid);
						pGraphicsContext->BindRenderTarget(0, TTextureHandleId::Invalid);

#if TDE2_DEBUG_MODE
						pGraphicsContext->EndSectionMarker();
#endif
					});

				frameGraphBlackboard.mMainRenderTargetHandle = output.mMainRenderTargetHandle;
			}
	};


	class CSkyGeometryRenderPass : public CBaseRenderPass
	{
		public:
			explicit CSkyGeometryRenderPass(const TPassInvokeContext& context, TPtr<CRenderQueue> pCommandsBuffer) :
				CBaseRenderPass(context, pCommandsBuffer)
			{
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard, bool isHDRSupportEnabled)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mMainRenderTargetHandle = TFrameGraphResourceHandle::Invalid;
				};

				auto& lightCullData = frameGraphBlackboard.mLightCullingData;

				auto&& output = pFrameGraph->AddPass<TPassData>("RenderSkyGeometryRenderPass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{
						builder.Read(frameGraphBlackboard.mDepthBufferHandle);

						TFrameGraphTexture::TDesc mainRenderTargetParams{};

						mainRenderTargetParams.mWidth = mContext.mWindowWidth;
						mainRenderTargetParams.mHeight = mContext.mWindowHeight;
						mainRenderTargetParams.mFormat = isHDRSupportEnabled ? FT_FLOAT4 : FT_NORM_UBYTE4;
						mainRenderTargetParams.mNumOfMipLevels = 1;
						mainRenderTargetParams.mNumOfSamples = 1;
						mainRenderTargetParams.mSamplingQuality = 0;
						mainRenderTargetParams.mType = E_TEXTURE_IMPL_TYPE::TEXTURE_2D;
						mainRenderTargetParams.mUsageType = E_TEXTURE_IMPL_USAGE_TYPE::STATIC;
						mainRenderTargetParams.mBindFlags = E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE | E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET;
						mainRenderTargetParams.mName = "MainRenderTaget";
						mainRenderTargetParams.mFlags = E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT;

						data.mMainRenderTargetHandle = builder.Create<TFrameGraphTexture>(mainRenderTargetParams.mName, mainRenderTargetParams);
						data.mMainRenderTargetHandle = builder.Write(data.mMainRenderTargetHandle);

						TDE2_ASSERT(data.mMainRenderTargetHandle != TFrameGraphResourceHandle::Invalid);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("RenderSkyGeometryRenderPass");

#if TDE2_DEBUG_MODE
						pGraphicsContext->BeginSectionMarker("RenderSky");
#endif

						TDE_RENDER_SECTION(pGraphicsContext, "RenderSkyGeometryRenderPass");

						TFrameGraphTexture& mainRenderTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mMainRenderTargetHandle);
						TFrameGraphTexture& depthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mDepthBufferHandle);

						pGraphicsContext->BindRenderTarget(0, mainRenderTarget.mTextureHandle);
						pGraphicsContext->BindDepthBufferTarget(depthBufferTarget.mTextureHandle);

						pGraphicsContext->ClearRenderTarget(static_cast<U8>(0), TColorUtils::mBlack);

						ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mpCommandsBuffer, false, 0, 1);

						pGraphicsContext->BindDepthBufferTarget(TTextureHandleId::Invalid);
						pGraphicsContext->BindRenderTarget(0, TTextureHandleId::Invalid);
					});

				frameGraphBlackboard.mMainRenderTargetHandle = output.mMainRenderTargetHandle;
			}
	};


	class CVolumetricCloudsMainPass : public CBaseRenderPass
	{
		public:
			explicit CVolumetricCloudsMainPass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
			{
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mVolumetricCloudsMainBufferHandle = TFrameGraphResourceHandle::Invalid;
				};

				const U32 textureWidth = mContext.mWindowWidth / 4;
				const U32 textureHeight = mContext.mWindowHeight / 4;

				auto&& output = pFrameGraph->AddPass<TPassData>("VolumetricCloudsMainPass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{
						builder.Read(frameGraphBlackboard.mMainRenderTargetHandle);
						builder.Read(frameGraphBlackboard.mDepthBufferHandle);

						TFrameGraphTexture::TDesc volumetricCloudsMainBufferParams{};

						volumetricCloudsMainBufferParams.mWidth = textureWidth;
						volumetricCloudsMainBufferParams.mHeight = textureHeight;
						volumetricCloudsMainBufferParams.mFormat = FT_FLOAT4;
						volumetricCloudsMainBufferParams.mNumOfMipLevels = 1;
						volumetricCloudsMainBufferParams.mNumOfSamples = 1;
						volumetricCloudsMainBufferParams.mSamplingQuality = 0;
						volumetricCloudsMainBufferParams.mType = E_TEXTURE_IMPL_TYPE::TEXTURE_2D;
						volumetricCloudsMainBufferParams.mUsageType = E_TEXTURE_IMPL_USAGE_TYPE::STATIC;
						volumetricCloudsMainBufferParams.mBindFlags = E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE | E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS;
						volumetricCloudsMainBufferParams.mName = "VolumetricCloudsMainTarget";
						volumetricCloudsMainBufferParams.mFlags = E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT;
						volumetricCloudsMainBufferParams.mIsWriteable = true;

						data.mVolumetricCloudsMainBufferHandle = builder.Create<TFrameGraphTexture>(volumetricCloudsMainBufferParams.mName, volumetricCloudsMainBufferParams);
						data.mVolumetricCloudsMainBufferHandle = builder.Write(data.mVolumetricCloudsMainBufferHandle);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("VolumetricCloudsMainPass");

#if TDE2_DEBUG_MODE
						pGraphicsContext->BeginSectionMarker("RenderVolumetricsClouds");
#endif

						struct
						{
							TVector2 mInvTextureSizes;
							I32      mStepsCount;
						} uniformsData;

						uniformsData.mInvTextureSizes = TVector2{ 1 / static_cast<F32>(textureWidth), 1 / static_cast<F32>(textureHeight) };
						uniformsData.mStepsCount = 64;

						TFrameGraphTexture& mainRenderTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mMainRenderTargetHandle);
						TFrameGraphTexture& depthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mDepthBufferHandle);
						TFrameGraphTexture& cloudsMainTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mVolumetricCloudsMainBufferHandle);

						const TTextureSamplerId linearSamplerHandle = pGraphicsContext->GetGraphicsObjectManager()->GetDefaultTextureSampler(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

						auto pVolumetricCloudsRenderPassShader = pResourceManager->GetResource<IShader>(pResourceManager->Load<IShader>(CProjectSettings::Get()->mGraphicsSettings.mVolumetricCloudsMainShader)); // \todo add caching
						
						if (pVolumetricCloudsRenderPassShader)
						{
							pGraphicsContext->SetTexture(pVolumetricCloudsRenderPassShader->GetResourceBindingSlot("OutputTexture"), cloudsMainTarget.mTextureHandle, true);
							pGraphicsContext->SetSampler(pVolumetricCloudsRenderPassShader->GetResourceBindingSlot("OutputTexture"), linearSamplerHandle);
							pGraphicsContext->SetTexture(pVolumetricCloudsRenderPassShader->GetResourceBindingSlot("DepthTexture"), depthBufferTarget.mTextureHandle);
							pGraphicsContext->SetSampler(pVolumetricCloudsRenderPassShader->GetResourceBindingSlot("DepthTexture"), linearSamplerHandle);
							pGraphicsContext->SetTexture(pVolumetricCloudsRenderPassShader->GetResourceBindingSlot("MainTexture"), mainRenderTarget.mTextureHandle);
							pGraphicsContext->SetSampler(pVolumetricCloudsRenderPassShader->GetResourceBindingSlot("MainTexture"), linearSamplerHandle);

							pVolumetricCloudsRenderPassShader->SetUserUniformsBuffer(0, reinterpret_cast<const U8*>(&uniformsData), sizeof(uniformsData));
							pVolumetricCloudsRenderPassShader->Bind();

							pGraphicsContext->DispatchCompute(textureWidth / 16, textureHeight / 16, 1);

							// \note Unbind resources
							pGraphicsContext->SetTexture(pVolumetricCloudsRenderPassShader->GetResourceBindingSlot("OutputTexture"), TTextureHandleId::Invalid, true);
							pGraphicsContext->SetTexture(pVolumetricCloudsRenderPassShader->GetResourceBindingSlot("DepthTexture"), TTextureHandleId::Invalid);
							pGraphicsContext->SetTexture(pVolumetricCloudsRenderPassShader->GetResourceBindingSlot("MainTexture"), TTextureHandleId::Invalid);
						}
					});

				frameGraphBlackboard.mVolumetricCloudsMainTargetHandle = output.mVolumetricCloudsMainBufferHandle;
			}
	};


	class CVolumetricCloudsUpscalePass : public CBaseRenderPass
	{
		public:
			explicit CVolumetricCloudsUpscalePass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
			{
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mVolumetricCloudsFullSizeBufferHandle = TFrameGraphResourceHandle::Invalid;
				};

				auto&& output = pFrameGraph->AddPass<TPassData>("VolumetricCloudsBlurUpscalePass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{
						builder.Read(frameGraphBlackboard.mVolumetricCloudsMainTargetHandle);
						builder.Read(frameGraphBlackboard.mDepthBufferHandle);

						TFrameGraphTexture::TDesc volumetricCloudsFullSizeBufferParams = pFrameGraph->GetResourceDesc<TFrameGraphTexture>(frameGraphBlackboard.mVolumetricCloudsMainTargetHandle);

						volumetricCloudsFullSizeBufferParams.mWidth  = mContext.mWindowWidth;
						volumetricCloudsFullSizeBufferParams.mHeight = mContext.mWindowHeight;
						volumetricCloudsFullSizeBufferParams.mName   = "VolumetricCloudsFullSizeTarget";

						data.mVolumetricCloudsFullSizeBufferHandle = builder.Create<TFrameGraphTexture>(volumetricCloudsFullSizeBufferParams.mName, volumetricCloudsFullSizeBufferParams);
						data.mVolumetricCloudsFullSizeBufferHandle = builder.Write(data.mVolumetricCloudsFullSizeBufferHandle);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("VolumetricCloudsBlurUpscalePass");

						TFrameGraphTexture& depthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mDepthBufferHandle);
						TFrameGraphTexture& cloudsMainTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mVolumetricCloudsMainTargetHandle);
						TFrameGraphTexture& cloudsFullSizeTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mVolumetricCloudsFullSizeBufferHandle);

						const TTextureSamplerId linearSamplerHandle = pGraphicsContext->GetGraphicsObjectManager()->GetDefaultTextureSampler(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

						auto pVolumetricCloudsUpsampleBlurPassShader = pResourceManager->GetResource<IShader>(pResourceManager->Load<IShader>("Shaders/Default/Volumetrics/VolumetricCloudsBlur.cshader")); // \todo Add caching
						if (pVolumetricCloudsUpsampleBlurPassShader)
						{
							pGraphicsContext->SetTexture(pVolumetricCloudsUpsampleBlurPassShader->GetResourceBindingSlot("OutputTexture"), cloudsFullSizeTarget.mTextureHandle, true);
							pGraphicsContext->SetSampler(pVolumetricCloudsUpsampleBlurPassShader->GetResourceBindingSlot("OutputTexture"), linearSamplerHandle);

							pGraphicsContext->SetTexture(pVolumetricCloudsUpsampleBlurPassShader->GetResourceBindingSlot("DepthTexture"), depthBufferTarget.mTextureHandle);
							pGraphicsContext->SetSampler(pVolumetricCloudsUpsampleBlurPassShader->GetResourceBindingSlot("DepthTexture"), linearSamplerHandle);

							pGraphicsContext->SetTexture(pVolumetricCloudsUpsampleBlurPassShader->GetResourceBindingSlot("MainTexture"), cloudsMainTarget.mTextureHandle);
							pGraphicsContext->SetSampler(pVolumetricCloudsUpsampleBlurPassShader->GetResourceBindingSlot("MainTexture"), linearSamplerHandle);
							
							pVolumetricCloudsUpsampleBlurPassShader->Bind();

							pGraphicsContext->DispatchCompute(mContext.mWindowWidth / 16, mContext.mWindowHeight / 16, 1);

							// \note unbind
							pGraphicsContext->SetTexture(pVolumetricCloudsUpsampleBlurPassShader->GetResourceBindingSlot("OutputTexture"), TTextureHandleId::Invalid, true);
							pGraphicsContext->SetTexture(pVolumetricCloudsUpsampleBlurPassShader->GetResourceBindingSlot("DepthTexture"), TTextureHandleId::Invalid);
							pGraphicsContext->SetTexture(pVolumetricCloudsUpsampleBlurPassShader->GetResourceBindingSlot("MainTexture"), TTextureHandleId::Invalid);
						}
					});

				frameGraphBlackboard.mVolumetricCloudsFullSizeTargetHandle = output.mVolumetricCloudsFullSizeBufferHandle;
			}
	};


	class CVolumetricCloudsComposePass : public CBaseRenderPass
	{
		public:
			explicit CVolumetricCloudsComposePass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
			{
				mGraphicsPipelineHandle = mContext.mpGraphicsContext->GetGraphicsObjectManager()->CreateGraphicsPipelineState(
					{
						mShaderId,
						{},
						TDepthStencilStateDesc { false, false },
						{}
					}
				).GetOrDefault(TGraphicsPipelineStateId::Invalid);
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mMainRenderTargetHandle = TFrameGraphResourceHandle::Invalid;
				};

				auto&& output = pFrameGraph->AddPass<TPassData>("VolumetricCloudsBlurComposePass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{
						builder.Read(frameGraphBlackboard.mVolumetricCloudsFullSizeTargetHandle);
						builder.Read(frameGraphBlackboard.mDepthBufferHandle);

						builder.Read(frameGraphBlackboard.mMainRenderTargetHandle);

						const TFrameGraphTexture::TDesc& mainTargetDesc = pFrameGraph->GetResourceDesc<TFrameGraphTexture>(frameGraphBlackboard.mMainRenderTargetHandle);

						mContext.mWindowWidth = mainTargetDesc.mWidth;
						mContext.mWindowHeight = mainTargetDesc.mHeight;

						data.mMainRenderTargetHandle = builder.Write(frameGraphBlackboard.mMainRenderTargetHandle);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("VolumetricCloudsBlurComposePass");

						TFrameGraphTexture& depthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mDepthBufferHandle);
						TFrameGraphTexture& cloudsFullSizeTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mVolumetricCloudsFullSizeTargetHandle);
						TFrameGraphTexture& mainRenderTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mMainRenderTargetHandle);

						ExecuteFullScreenShader(
							{ 
								mContext.mpGraphicsContext, 
								mContext.mpResourceManager, 
								depthBufferTarget.mTextureHandle, 
								cloudsFullSizeTarget.mTextureHandle,
								mainRenderTarget.mTextureHandle,
								mGraphicsPipelineHandle,
								mShaderId,
								mContext.mWindowWidth,
								mContext.mWindowHeight 
							});

#if TDE2_DEBUG_MODE
						pGraphicsContext->EndSectionMarker();
#endif
					});

				frameGraphBlackboard.mMainRenderTargetHandle = output.mMainRenderTargetHandle;
			}
		private:
			static const std::string mShaderId;

			TGraphicsPipelineStateId mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	};


	const std::string CVolumetricCloudsComposePass::mShaderId = "Shaders/Default/Volumetrics/VolumetricCloudsCompose.shader";


	class CUIRenderPass : public CBaseRenderPass
	{
		public:
			explicit CUIRenderPass(const TPassInvokeContext& context, TPtr<CRenderQueue> pCommandsBuffer) :
				CBaseRenderPass(context, pCommandsBuffer)
			{
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mUIRenderTargetHandle = TFrameGraphResourceHandle::Invalid;
				};

				auto& lightCullData = frameGraphBlackboard.mLightCullingData;

				auto&& output = pFrameGraph->AddPass<TPassData>("UIRenderPass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{
						TFrameGraphTexture::TDesc uiRenderTargetParams = pFrameGraph->GetResourceDesc<TFrameGraphTexture>(frameGraphBlackboard.mMainRenderTargetHandle);
						uiRenderTargetParams.mName = "UIRenderTaget";

						data.mUIRenderTargetHandle = builder.Create<TFrameGraphTexture>(uiRenderTargetParams.mName, uiRenderTargetParams);
						data.mUIRenderTargetHandle = builder.Write(data.mUIRenderTargetHandle);

						TDE2_ASSERT(data.mUIRenderTargetHandle != TFrameGraphResourceHandle::Invalid);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("UIRenderPass");

#if TDE2_DEBUG_MODE
						//pGraphicsContext->EndSectionMarker(); // pop PostProcessing section
#endif
						TDE_RENDER_SECTION(pGraphicsContext, "UIRenderPass");

						TFrameGraphTexture& uiRenderTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mUIRenderTargetHandle);

						pGraphicsContext->SetDepthBufferEnabled(false);
						pGraphicsContext->BindRenderTarget(0, uiRenderTarget.mTextureHandle);
						pGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(mContext.mWindowWidth), static_cast<F32>(mContext.mWindowHeight), 0.0f, 1.0f);

						pGraphicsContext->ClearRenderTarget(static_cast<U8>(0), TColor32F(0.0f));

						ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mpCommandsBuffer, true);

						pGraphicsContext->BindDepthBufferTarget(TTextureHandleId::Invalid);
						pGraphicsContext->BindRenderTarget(0, TTextureHandleId::Invalid);
					});

				frameGraphBlackboard.mUIRenderTargetHandle = output.mUIRenderTargetHandle;
			}
	};


	class CDebugUIRenderPass : public CBaseRenderPass
	{
		public:
			explicit CDebugUIRenderPass(const TPassInvokeContext& context, TPtr<CRenderQueue> pCommandsBuffer) :
				CBaseRenderPass(context, pCommandsBuffer)
			{
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mTargetHandle = TFrameGraphResourceHandle::Invalid;
				};

				auto& lightCullData = frameGraphBlackboard.mLightCullingData;

				auto&& output = pFrameGraph->AddPass<TPassData>("DebugUIRenderPass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{
						data.mTargetHandle = builder.Write(builder.Read(frameGraphBlackboard.mLDRMainRenderTargetHandle));

						TDE2_ASSERT(data.mTargetHandle != TFrameGraphResourceHandle::Invalid);

					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);

						TDE2_PROFILER_SCOPE("DebugUIRenderPass");
						TDE_RENDER_SECTION(pGraphicsContext, "DebugUIRenderPass");

						ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mpCommandsBuffer, true);
					});
			}
	};


	class CLightsHeatmapDebugPostProcessPass : public CBaseRenderPass
	{
		public:
			explicit CLightsHeatmapDebugPostProcessPass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
			{
				mGraphicsPipelineHandle = mContext.mpGraphicsContext->GetGraphicsObjectManager()->CreateGraphicsPipelineState(
					{
						mShaderId,
						{},
						TDepthStencilStateDesc { false, false },
						{}
					}
				).GetOrDefault(TGraphicsPipelineStateId::Invalid);
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard, TFrameGraphResourceHandle target)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mSourceTargetHandle = TFrameGraphResourceHandle::Invalid;
					TFrameGraphResourceHandle mDestTargetHandle = TFrameGraphResourceHandle::Invalid;
				};

				auto&& output = pFrameGraph->AddPass<TPassData>("LightsHeatmapDebugPostProcessPass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{
						builder.Read(frameGraphBlackboard.mLightCullingData.mOpaqueLightGridTextureHandle);

						data.mSourceTargetHandle = builder.Read(frameGraphBlackboard.mMainRenderTargetHandle);						
						
						TFrameGraphTexture::TDesc outputTargetParams = pFrameGraph->GetResourceDesc<TFrameGraphTexture>(frameGraphBlackboard.mMainRenderTargetHandle);
						outputTargetParams.mName = "IntermediateRenderTaget";

						mContext.mWindowWidth  = outputTargetParams.mWidth;
						mContext.mWindowHeight = outputTargetParams.mHeight;

						data.mDestTargetHandle = builder.Create<TFrameGraphTexture>(outputTargetParams.mName, outputTargetParams);
						data.mDestTargetHandle = builder.Write(data.mDestTargetHandle);

					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("LightsHeatmapDebugPostProcessPass");
						TDE_RENDER_SECTION(pGraphicsContext, "LightsHeatmapDebugPostProcessPass");

						TFrameGraphTexture& opaqueLightsGridTexture = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mLightCullingData.mOpaqueLightGridTextureHandle);
						TFrameGraphTexture& sourceTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mSourceTargetHandle);
						TFrameGraphTexture& destTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mDestTargetHandle);

						pGraphicsContext->SetTexture(LIGHT_GRID_TEXTURE_SLOT, opaqueLightsGridTexture.mTextureHandle, false);

						ExecuteFullScreenShader(
							{
								mContext.mpGraphicsContext,
								mContext.mpResourceManager,
								sourceTarget.mTextureHandle,
								TTextureHandleId::Invalid,
								destTarget.mTextureHandle,
								mGraphicsPipelineHandle,
								mShaderId,
								mContext.mWindowWidth,
								mContext.mWindowHeight
							});
					});

				frameGraphBlackboard.mMainRenderTargetHandle = output.mDestTargetHandle;
			}
		private:
			static const std::string mShaderId;

			TGraphicsPipelineStateId mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	};


	const std::string CLightsHeatmapDebugPostProcessPass::mShaderId = "Shaders/PostEffects/LightsHeatmap.shader";


	class CExtractLuminancePostProcessPass : public CBaseRenderPass
	{
		public:
			explicit CExtractLuminancePostProcessPass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
			{
				mGraphicsPipelineHandle = mContext.mpGraphicsContext->GetGraphicsObjectManager()->CreateGraphicsPipelineState(
					{
						mShaderId,
						{},
						TDepthStencilStateDesc { false, false },
						{}
					}
				).GetOrDefault(TGraphicsPipelineStateId::Invalid);

				mContext.mWindowWidth  = mLuminanceTargetSizes;
				mContext.mWindowHeight = mLuminanceTargetSizes;
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mSourceTargetHandle = TFrameGraphResourceHandle::Invalid;
					TFrameGraphResourceHandle mDestTargetHandle = TFrameGraphResourceHandle::Invalid;
				};

				auto&& output = pFrameGraph->AddPass<TPassData>("ExtractLuminancePostProcessPass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{
						builder.Read(frameGraphBlackboard.mLightCullingData.mOpaqueLightGridTextureHandle);

						data.mSourceTargetHandle = builder.Read(frameGraphBlackboard.mMainRenderTargetHandle);

						TFrameGraphTexture::TDesc outputTargetParams{};

						outputTargetParams.mWidth           = mLuminanceTargetSizes;
						outputTargetParams.mHeight          = mLuminanceTargetSizes;
						outputTargetParams.mFormat          = FT_FLOAT1;
						outputTargetParams.mNumOfMipLevels  = static_cast<U32>(log2(mLuminanceTargetSizes)) + 1;
						outputTargetParams.mNumOfSamples    = 1;
						outputTargetParams.mSamplingQuality = 0;
						outputTargetParams.mType            = E_TEXTURE_IMPL_TYPE::TEXTURE_2D;
						outputTargetParams.mUsageType       = E_TEXTURE_IMPL_USAGE_TYPE::STATIC;
						outputTargetParams.mBindFlags       = E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE | E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET;
						outputTargetParams.mName            = "LuminanceTarget";
						outputTargetParams.mFlags           = E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT;

						data.mDestTargetHandle = builder.Create<TFrameGraphTexture>(outputTargetParams.mName, outputTargetParams);
						data.mDestTargetHandle = builder.Write(data.mDestTargetHandle);

					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("ExtractLuminancePostProcessPass");

#if TDE2_DEBUG_MODE
						pGraphicsContext->BeginSectionMarker("EyeAdaptation");
#endif

						TFrameGraphTexture& opaqueLightsGridTexture = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mLightCullingData.mOpaqueLightGridTextureHandle);
						TFrameGraphTexture& sourceTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mSourceTargetHandle);
						TFrameGraphTexture& destTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mDestTargetHandle);

						ExecuteFullScreenShader(
							{
								mContext.mpGraphicsContext,
								mContext.mpResourceManager,
								sourceTarget.mTextureHandle,
								TTextureHandleId::Invalid,
								destTarget.mTextureHandle,
								mGraphicsPipelineHandle,
								mShaderId,
								mContext.mWindowWidth,
								mContext.mWindowHeight
							});

						pGraphicsContext->GenerateMipMaps(destTarget.mTextureHandle);
					});

				frameGraphBlackboard.mLuminanceTargetHandle = output.mDestTargetHandle;
			}
		private:
			TDE2_STATIC_CONSTEXPR U32 mLuminanceTargetSizes = 1024;

			static const std::string mShaderId;

			TGraphicsPipelineStateId mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	};


	const std::string CExtractLuminancePostProcessPass::mShaderId = "Shaders/PostEffects/GenerateLuminance.shader";


	class CCalcAverageLuminancePostProcessPass : public CBaseRenderPass
	{
		public:
			explicit CCalcAverageLuminancePostProcessPass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
			{
				mGraphicsPipelineHandle = mContext.mpGraphicsContext->GetGraphicsObjectManager()->CreateGraphicsPipelineState(
					{
						mShaderId,
						{},
						TDepthStencilStateDesc { false, false },
						{}
					}
				).GetOrDefault(TGraphicsPipelineStateId::Invalid);

				mContext.mWindowWidth  = 1;
				mContext.mWindowHeight = 1;

				mAvgLuminanceTargets[0] = _createLuminanceTarget("PrevAvgLuminanceTarget");
				mAvgLuminanceTargets[1] = _createLuminanceTarget("CurrAvgLuminanceTarget");

				mTrilinearSamplerId = mContext.mpGraphicsContext->GetGraphicsObjectManager()->CreateTextureSampler(
					{
						E_TEXTURE_FILTER_TYPE::FT_TRILINEAR, E_ADDRESS_MODE_TYPE::AMT_CLAMP, E_ADDRESS_MODE_TYPE::AMT_CLAMP, E_ADDRESS_MODE_TYPE::AMT_CLAMP, true
					}).Get();
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard, const IPostProcessingProfile* pPostProcessProfile)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mOutAvgLuminanceTargetHandle = TFrameGraphResourceHandle::Invalid;
				};

				frameGraphBlackboard.mPrevAvgLuminanceTargetHandle = pFrameGraph->ImportResource("PrevAvgLuminanceTarget", TFrameGraphTexture::TDesc{}, TFrameGraphTexture{ mAvgLuminanceTargets[(mCurrActiveLuminanceTarget + 1) & 0x1] });
				frameGraphBlackboard.mCurrAvgLuminanceTargetHandle = pFrameGraph->ImportResource("CurrAvgLuminanceTarget", TFrameGraphTexture::TDesc{}, TFrameGraphTexture{ mAvgLuminanceTargets[mCurrActiveLuminanceTarget] });

				auto&& output = pFrameGraph->AddPass<TPassData>("CalcAverageLuminancePostProcessPass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{						
						builder.Read(frameGraphBlackboard.mLuminanceTargetHandle);
						builder.Read(frameGraphBlackboard.mPrevAvgLuminanceTargetHandle);

						data.mOutAvgLuminanceTargetHandle = builder.Write(frameGraphBlackboard.mCurrAvgLuminanceTargetHandle);

						mCurrActiveLuminanceTarget = (mCurrActiveLuminanceTarget + 1) & 0x1;

					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("CalcAverageLuminancePostProcessPass");

						TFrameGraphTexture& sceneLuminanceTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mLuminanceTargetHandle);
						TFrameGraphTexture& prevLuminanceTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mPrevAvgLuminanceTargetHandle);
						TFrameGraphTexture& currLuminanceTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mOutAvgLuminanceTargetHandle);
						
						struct
						{
							F32 mAdaptationRate = 0.5f;
						} uniformsData;

						if (pPostProcessProfile)
						{
							uniformsData.mAdaptationRate = pPostProcessProfile->GetToneMappingParameters().mEyeAdaptionCoeff;
						}

						if (auto pShader = pResourceManager->GetResource<IShader>(pResourceManager->Load<IShader>(mShaderId)))
						{
							pShader->SetUserUniformsBuffer(0, reinterpret_cast<const U8*>(&uniformsData), sizeof(uniformsData));
						}

						ExecuteFullScreenShader(
							{
								mContext.mpGraphicsContext,
								mContext.mpResourceManager,
								sceneLuminanceTarget.mTextureHandle,
								prevLuminanceTarget.mTextureHandle,
								currLuminanceTarget.mTextureHandle,
								mGraphicsPipelineHandle,
								mShaderId,
								mContext.mWindowWidth,
								mContext.mWindowHeight,
								&mTrilinearSamplerId
							});

#if TDE2_DEBUG_MODE
						pGraphicsContext->EndSectionMarker();
#endif
					});
			}
		private:
			TTextureHandleId _createLuminanceTarget(const std::string& name)
			{
				TInitTextureImplParams luminanceTargetParams{};

				luminanceTargetParams.mWidth           = 1;
				luminanceTargetParams.mHeight          = 1;
				luminanceTargetParams.mFormat          = FT_FLOAT1;
				luminanceTargetParams.mNumOfMipLevels  = 1;
				luminanceTargetParams.mNumOfSamples    = 1;
				luminanceTargetParams.mSamplingQuality = 0;
				luminanceTargetParams.mType            = E_TEXTURE_IMPL_TYPE::TEXTURE_2D;
				luminanceTargetParams.mUsageType       = E_TEXTURE_IMPL_USAGE_TYPE::STATIC;
				luminanceTargetParams.mBindFlags       = E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE | E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET;
				luminanceTargetParams.mName            = name.c_str();

				return mContext.mpGraphicsContext->GetGraphicsObjectManager()->CreateTexture(luminanceTargetParams).Get();
			}

		private:
			static const std::string        mShaderId;

			TGraphicsPipelineStateId        mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;

			std::array<TTextureHandleId, 2> mAvgLuminanceTargets;

			TTextureSamplerId               mTrilinearSamplerId = TTextureSamplerId::Invalid;

			U32                             mCurrActiveLuminanceTarget = 0;
	};


	const std::string CCalcAverageLuminancePostProcessPass::mShaderId = "Shaders/PostEffects/AdaptLuminance.shader";


	class CBloomThresholdPostProcessPass : public CBaseRenderPass
	{
		public:
			explicit CBloomThresholdPostProcessPass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
			{
				mGraphicsPipelineHandle = mContext.mpGraphicsContext->GetGraphicsObjectManager()->CreateGraphicsPipelineState(
					{
						mShaderId,
						{},
						TDepthStencilStateDesc { false, false },
						{}
					}
				).GetOrDefault(TGraphicsPipelineStateId::Invalid);
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard, U32 windowWidth, U32 windowHeight, const IPostProcessingProfile* pPostProcessProfile)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mSourceTargetHandle = TFrameGraphResourceHandle::Invalid;
					TFrameGraphResourceHandle mDestTargetHandle = TFrameGraphResourceHandle::Invalid;
				};

				mContext.mWindowWidth = windowWidth;
				mContext.mWindowHeight = windowHeight;

				auto&& output = pFrameGraph->AddPass<TPassData>("BloomPostProcessPass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{
						builder.Read(frameGraphBlackboard.mCurrAvgLuminanceTargetHandle);
						data.mSourceTargetHandle = builder.Read(frameGraphBlackboard.mMainRenderTargetHandle);

						TFrameGraphTexture::TDesc outputTargetParams = pFrameGraph->GetResourceDesc<TFrameGraphTexture>(frameGraphBlackboard.mMainRenderTargetHandle);
						outputTargetParams.mWidth  = mContext.mWindowWidth;
						outputTargetParams.mHeight = mContext.mWindowHeight;
						outputTargetParams.mName   = "BloomThresholdTarget";

						data.mDestTargetHandle = builder.Create<TFrameGraphTexture>(outputTargetParams.mName, outputTargetParams);
						data.mDestTargetHandle = builder.Write(data.mDestTargetHandle);

					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("BloomPostProcessPass");

#if TDE2_DEBUG_MODE
						pGraphicsContext->BeginSectionMarker("BloomPostProcessPass"); 
#endif

						TFrameGraphTexture& sourceTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mSourceTargetHandle);
						TFrameGraphTexture& destTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mDestTargetHandle);
						TFrameGraphTexture& avgLuminanceTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mCurrAvgLuminanceTargetHandle);

						struct
						{
							F32 mThreshold;
							F32 mKeyValue;
						} uniformsData;

						if (pPostProcessProfile)
						{
							uniformsData.mThreshold = pPostProcessProfile->GetBloomParameters().mThreshold;
							uniformsData.mKeyValue = pPostProcessProfile->GetToneMappingParameters().mKeyValue;
						}

						if (auto pShader = pResourceManager->GetResource<IShader>(pResourceManager->Load<IShader>(mShaderId)))
						{
							pShader->SetUserUniformsBuffer(0, reinterpret_cast<const U8*>(&uniformsData), sizeof(uniformsData));
						}

						ExecuteFullScreenShader(
							{
								mContext.mpGraphicsContext,
								mContext.mpResourceManager,
								sourceTarget.mTextureHandle,
								avgLuminanceTarget.mTextureHandle,
								destTarget.mTextureHandle,
								mGraphicsPipelineHandle,
								mShaderId,
								mContext.mWindowWidth,
								mContext.mWindowHeight
							});
					});

				frameGraphBlackboard.mBloomThresholdTargetHandle = output.mDestTargetHandle;
			}
		private:
			static const std::string mShaderId;

			TGraphicsPipelineStateId mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	};


	const std::string CBloomThresholdPostProcessPass::mShaderId = "Shaders/PostEffects/Bloom.shader";


	class CBloomComposePostProcessPass : public CBaseRenderPass
	{
		public:
			explicit CBloomComposePostProcessPass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
			{
				mGraphicsPipelineHandle = mContext.mpGraphicsContext->GetGraphicsObjectManager()->CreateGraphicsPipelineState(
					{
						mShaderId,
						{},
						TDepthStencilStateDesc { false, false },
						{}
					}
				).GetOrDefault(TGraphicsPipelineStateId::Invalid);
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard, TFrameGraphResourceHandle blurredTargetHandle)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mSourceTargetHandle = TFrameGraphResourceHandle::Invalid;
					TFrameGraphResourceHandle mDestTargetHandle = TFrameGraphResourceHandle::Invalid;
				};

				auto&& output = pFrameGraph->AddPass<TPassData>("BloomComposePostProcessPass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{
						builder.Read(blurredTargetHandle);
						data.mSourceTargetHandle = builder.Read(frameGraphBlackboard.mMainRenderTargetHandle);

						TFrameGraphTexture::TDesc outputTargetParams = pFrameGraph->GetResourceDesc<TFrameGraphTexture>(frameGraphBlackboard.mMainRenderTargetHandle);
						outputTargetParams.mName = "MainTargetBloomApplied";

						mContext.mWindowWidth  = outputTargetParams.mWidth;
						mContext.mWindowHeight = outputTargetParams.mHeight;

						data.mDestTargetHandle = builder.Create<TFrameGraphTexture>(outputTargetParams.mName, outputTargetParams);
						data.mDestTargetHandle = builder.Write(data.mDestTargetHandle);

					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("BloomComposePostProcessPass");
						TDE_RENDER_SECTION(pGraphicsContext, "BloomComposePostProcessPass");

						TFrameGraphTexture& sourceTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mSourceTargetHandle);
						TFrameGraphTexture& destTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mDestTargetHandle);
						TFrameGraphTexture& blurredTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(blurredTargetHandle);

						ExecuteFullScreenShader(
							{
								mContext.mpGraphicsContext,
								mContext.mpResourceManager,
								sourceTarget.mTextureHandle,
								blurredTarget.mTextureHandle,
								destTarget.mTextureHandle,
								mGraphicsPipelineHandle,
								mShaderId,
								mContext.mWindowWidth,
								mContext.mWindowHeight
							});

#if TDE2_DEBUG_MODE
						pGraphicsContext->EndSectionMarker(); 
#endif
					});

				frameGraphBlackboard.mMainRenderTargetHandle = output.mDestTargetHandle;
			}
		private:
			static const std::string mShaderId;

			TGraphicsPipelineStateId mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	};


	const std::string CBloomComposePostProcessPass::mShaderId = "Shaders/PostEffects/BloomFinal.shader";



	class CBlurPostProcessPass : public CBaseRenderPass
	{
		public:
			explicit CBlurPostProcessPass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
			{
				mGraphicsPipelineHandle = mContext.mpGraphicsContext->GetGraphicsObjectManager()->CreateGraphicsPipelineState(
					{
						mShaderId,
						{},
						TDepthStencilStateDesc { false, false },
						{}
					}
				).GetOrDefault(TGraphicsPipelineStateId::Invalid);
			}

			TFrameGraphResourceHandle AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard, 
				TFrameGraphResourceHandle source, U32 windowWidth, U32 windowHeight, const TVector4& blurParams, U32 samplesCount)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mSourceTargetHandle = TFrameGraphResourceHandle::Invalid;
					TFrameGraphResourceHandle mDestTargetHandle = TFrameGraphResourceHandle::Invalid;
				};

				mContext.mWindowWidth = windowWidth;
				mContext.mWindowHeight = windowHeight;

				std::string blurPassId = blurParams.y > 0.0f ? "VerticalBlurPostProcessPass" : "HorizontalBlurPostProcessPass";

				auto&& output = pFrameGraph->AddPass<TPassData>(blurPassId, [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{
						data.mSourceTargetHandle = builder.Read(source);

						TFrameGraphTexture::TDesc outputTargetParams = pFrameGraph->GetResourceDesc<TFrameGraphTexture>(frameGraphBlackboard.mMainRenderTargetHandle);

						outputTargetParams.mWidth  = mContext.mWindowWidth;
						outputTargetParams.mHeight = mContext.mWindowHeight;
						outputTargetParams.mName   = blurParams.y > 0.0f ? "VerticalBlurOutputTarget" : "HorizontalBlurOutputTarget";

						data.mDestTargetHandle = builder.Create<TFrameGraphTexture>(outputTargetParams.mName, outputTargetParams);
						data.mDestTargetHandle = builder.Write(data.mDestTargetHandle);

					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE(blurPassId);
						TDE_RENDER_SECTION(pGraphicsContext, blurPassId);

						TFrameGraphTexture& sourceTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mSourceTargetHandle);
						TFrameGraphTexture& destTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mDestTargetHandle);
						
						struct
						{
							TVector4 mBlurParams;
							U32      mSamplesCount = 1;
						} uniformsData;

						uniformsData.mBlurParams = blurParams;
						uniformsData.mSamplesCount = samplesCount;

						if (auto pShader = pResourceManager->GetResource<IShader>(pResourceManager->Load<IShader>(mShaderId)))
						{
							pShader->SetUserUniformsBuffer(0, reinterpret_cast<const U8*>(&uniformsData), sizeof(uniformsData));
						}

						ExecuteFullScreenShader(
							{
								mContext.mpGraphicsContext,
								mContext.mpResourceManager,
								sourceTarget.mTextureHandle,
								TTextureHandleId::Invalid,
								destTarget.mTextureHandle,
								mGraphicsPipelineHandle,
								mShaderId,
								mContext.mWindowWidth,
								mContext.mWindowHeight
							});
					});

				return output.mDestTargetHandle;
			}
		private:
			static const std::string mShaderId;

			TGraphicsPipelineStateId mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	};


	const std::string CBlurPostProcessPass::mShaderId = "Shaders/PostEffects/GaussianBlur.shader";


	class CToneMapAndComposePostProcessPass : public CBaseRenderPass
	{
		public:
			explicit CToneMapAndComposePostProcessPass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
			{
				mGraphicsPipelineHandle = mContext.mpGraphicsContext->GetGraphicsObjectManager()->CreateGraphicsPipelineState(
					{
						mShaderId,
						{},
						TDepthStencilStateDesc { false, false },
						{}
					}
				).GetOrDefault(TGraphicsPipelineStateId::Invalid);
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard, U32 windowWidth, U32 windowHeight, bool isHDRSupportEnabled, const IPostProcessingProfile* pCurrPostProcessProfile)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mSourceTargetHandle = TFrameGraphResourceHandle::Invalid;
					TFrameGraphResourceHandle mDestTargetHandle = TFrameGraphResourceHandle::Invalid;
				};

				mContext.mWindowWidth = windowWidth;
				mContext.mWindowHeight = windowHeight;

				auto&& output = pFrameGraph->AddPass<TPassData>("ToneMapAndComposePass", [&, this](CFrameGraphBuilder& builder, TPassData& data)
					{
						builder.Read(frameGraphBlackboard.mUIRenderTargetHandle);
						builder.Read(frameGraphBlackboard.mPrevAvgLuminanceTargetHandle);

						if (pCurrPostProcessProfile && pCurrPostProcessProfile->GetColorGradingParameters().mIsEnabled)
						{
							builder.Read(frameGraphBlackboard.mColorGradingLUTHandle);
						}

						data.mSourceTargetHandle = builder.Read(frameGraphBlackboard.mMainRenderTargetHandle);
						data.mDestTargetHandle = builder.Write(frameGraphBlackboard.mBackBufferHandle);

					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("ToneMapAndComposePass");
						TDE_RENDER_SECTION(pGraphicsContext, "ToneMapAndComposePass");

						TFrameGraphTexture& uiTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mUIRenderTargetHandle);
						TFrameGraphTexture& avgLuminanceTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mPrevAvgLuminanceTargetHandle);
						TFrameGraphTexture& sourceTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mSourceTargetHandle);
						TFrameGraphTexture& destTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mDestTargetHandle);
						TFrameGraphTexture& colorGradingLUTTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mColorGradingLUTHandle);

						const TTextureSamplerId linearSamplerHandle = pGraphicsContext->GetGraphicsObjectManager()->GetDefaultTextureSampler(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

						if (auto pShader = pResourceManager->GetResource<IShader>(pResourceManager->Load<IShader>(mShaderId)))
						{
							pGraphicsContext->SetTexture(pShader->GetResourceBindingSlot("LuminanceBuffer"), avgLuminanceTarget.mTextureHandle);
							pGraphicsContext->SetSampler(pShader->GetResourceBindingSlot("LuminanceBuffer"), linearSamplerHandle);

							pGraphicsContext->SetTexture(pShader->GetResourceBindingSlot("UIBuffer"), uiTarget.mTextureHandle);
							pGraphicsContext->SetSampler(pShader->GetResourceBindingSlot("UIBuffer"), linearSamplerHandle);

							// \todo Pass color LUT texture
							pGraphicsContext->SetTexture(pShader->GetResourceBindingSlot("ColorGradingLUT"), colorGradingLUTTarget.mTextureHandle);
							pGraphicsContext->SetSampler(pShader->GetResourceBindingSlot("ColorGradingLUT"), linearSamplerHandle);

							struct
							{
								TVector4 mToneMappingParams; // \todo Add default values
								TVector4 mColorGradingParams;
							} uniformsData;

							if (pCurrPostProcessProfile)
							{
								const auto& toneMappingParameters = pCurrPostProcessProfile->GetToneMappingParameters();
								const auto& colorGradingParameters = pCurrPostProcessProfile->GetColorGradingParameters();

								uniformsData.mToneMappingParams = TVector4(isHDRSupportEnabled ? 1.0f : 0.0f, toneMappingParameters.mExposure, toneMappingParameters.mKeyValue, 0.0f);
								uniformsData.mColorGradingParams = TVector4(colorGradingParameters.mIsEnabled ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f);
							}

							pShader->SetUserUniformsBuffer(0, reinterpret_cast<const U8*>(&uniformsData), sizeof(uniformsData));
						}

						ExecuteFullScreenShader(
							{
								mContext.mpGraphicsContext,
								mContext.mpResourceManager,
								sourceTarget.mTextureHandle,
								TTextureHandleId::Invalid,
								destTarget.mTextureHandle, // draw onto the screen, destTarget.mTextureHandle is always Invalid
								mGraphicsPipelineHandle,
								mShaderId,
								mContext.mWindowWidth,
								mContext.mWindowHeight
							});
					});

				frameGraphBlackboard.mLDRMainRenderTargetHandle = output.mDestTargetHandle;
			}
		private:
			static const std::string mShaderId;

			TGraphicsPipelineStateId mGraphicsPipelineHandle = TGraphicsPipelineStateId::Invalid;
	};


	const std::string CToneMapAndComposePostProcessPass::mShaderId = "Shaders/PostEffects/ToneMapping.shader";


	static std::unique_ptr<CVolumetricCloudsComposePass> pVolumetricCloudsComposePass = nullptr;
	static std::unique_ptr<CLightsHeatmapDebugPostProcessPass> pLightsHeatmapDebugPostProcessPass = nullptr;
	static std::unique_ptr<CExtractLuminancePostProcessPass> pExtractLuminancePostProcessPass = nullptr;
	static std::unique_ptr<CCalcAverageLuminancePostProcessPass> pCalcAverageLuminancePostProcessPass = nullptr;
	static std::unique_ptr<CBloomThresholdPostProcessPass> pBloomThresholdPostProcessPass = nullptr;
	static std::unique_ptr<CBloomComposePostProcessPass> pBloomComposePostProcessPass = nullptr;
	static std::unique_ptr<CBlurPostProcessPass> pBlurPostProcessPass = nullptr;
	static std::unique_ptr<CToneMapAndComposePostProcessPass> pToneMappingComposePostProcessPass = nullptr;


	E_RESULT_CODE InitStaticRenderPasses(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TPtr<IGlobalShaderProperties> pGlobalShaderProperties)
	{
		TDE2_PROFILER_SCOPE("InitStaticRenderPasses");

		E_RESULT_CODE result = RC_OK;

		TPassInvokeContext passConfig
		{
			pGraphicsContext,
			pResourceManager,
			pGlobalShaderProperties,
			0, 0
		};

		pVolumetricCloudsComposePass = std::make_unique<CVolumetricCloudsComposePass>(passConfig);
		pLightsHeatmapDebugPostProcessPass = std::make_unique<CLightsHeatmapDebugPostProcessPass>(passConfig);
		pExtractLuminancePostProcessPass = std::make_unique<CExtractLuminancePostProcessPass>(passConfig);
		pCalcAverageLuminancePostProcessPass = std::make_unique<CCalcAverageLuminancePostProcessPass>(passConfig);
		pBloomThresholdPostProcessPass = std::make_unique<CBloomThresholdPostProcessPass>(passConfig);
		pBloomComposePostProcessPass = std::make_unique<CBloomComposePostProcessPass>(passConfig);
		pBlurPostProcessPass = std::make_unique<CBlurPostProcessPass>(passConfig);
		pToneMappingComposePostProcessPass = std::make_unique<CToneMapAndComposePostProcessPass>(passConfig);

		return result;
	}

	void DestroyStaticRenderPasses()
	{
		pVolumetricCloudsComposePass = nullptr;
		pLightsHeatmapDebugPostProcessPass = nullptr;
		pExtractLuminancePostProcessPass = nullptr;
		pCalcAverageLuminancePostProcessPass = nullptr;
		pBloomThresholdPostProcessPass = nullptr;
		pBloomComposePostProcessPass = nullptr;
		pBlurPostProcessPass = nullptr;
		pToneMappingComposePostProcessPass = nullptr;
	}


	/*!
		\brief CForwardRenderer's definition
	*/

	CForwardRenderer::CForwardRenderer():
		CBaseObject(), mpMainCamera(nullptr), mpResourceManager(nullptr), mpGlobalShaderProperties(nullptr)
	{
	}


	static E_RESULT_CODE PrepareTileFrustums(const TLightCullingData& data, TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager)
	{
		E_RESULT_CODE result = RC_OK;

		struct
		{
			U32 mWorkGroupsX = 0;
			U32 mWorkGroupsY = 0;
		} shaderParameters;

		shaderParameters.mWorkGroupsX = data.mWorkGroupsX;
		shaderParameters.mWorkGroupsY = data.mWorkGroupsY;

		result = pGraphicsContext->SetStructuredBuffer(TILE_FRUSTUMS_BUFFER_SLOT, data.mTileFrustumsBufferHandle, true);

		auto pTileFrustumInitializationShader = pResourceManager->GetResource<IShader>(pResourceManager->Load<IShader>("Shaders/Default/TileFrustumsConstruction.cshader"));
		
		pTileFrustumInitializationShader->SetUserUniformsBuffer(0, reinterpret_cast<const U8*>(&shaderParameters), sizeof(shaderParameters));
		pTileFrustumInitializationShader->Bind();

		pGraphicsContext->DispatchCompute(
			(data.mWorkGroupsX + FRUSTUM_TILES_PER_GROUP - 1) / FRUSTUM_TILES_PER_GROUP, 
			(data.mWorkGroupsY + FRUSTUM_TILES_PER_GROUP - 1) / FRUSTUM_TILES_PER_GROUP, 
			1);

		result = result | pGraphicsContext->SetStructuredBuffer(TILE_FRUSTUMS_BUFFER_SLOT, TBufferHandleId::Invalid, true);

		return result;
	}


	static TResult<TBufferHandleId> CreateLightIndexCountersBuffer(IGraphicsObjectManager* pGraphicsObjectManager, bool isInitializerBuffer = false, 
		const C8* debugName = "LightIndexCounters")
	{
		static const std::array<U32, 4> initialValues{ 0 };

		auto lightIndexCountersCreateResult = pGraphicsObjectManager->CreateBuffer(
			{
				isInitializerBuffer ? E_BUFFER_USAGE_TYPE::STATIC : E_BUFFER_USAGE_TYPE::DEFAULT,
				E_BUFFER_TYPE::STRUCTURED,
				sizeof(U32) * initialValues.size(),
				initialValues.data(),
				sizeof(U32) * initialValues.size(),
				!isInitializerBuffer,
				sizeof(U32),
				E_STRUCTURED_BUFFER_TYPE::DEFAULT,
				E_INDEX_FORMAT_TYPE::INDEX16, debugName
			});

		if (lightIndexCountersCreateResult.HasError())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(lightIndexCountersCreateResult.GetError());
		}

		return Wrench::TOkValue<TBufferHandleId>(lightIndexCountersCreateResult.Get());
	}


	static TResult<TLightCullingData> InitLightGrid(const TRendererInitParams& params)
	{
		TDE2_PROFILER_SCOPE("InitLightGrid");

		TLightCullingData data;

		auto&& graphicsObjectManager = params.mpGraphicsContext->GetGraphicsObjectManager();

		const U32 width = params.mpWindowSystem->GetWidth();
		const U32 height = params.mpWindowSystem->GetHeight();

		data.mWorkGroupsX = (width + (width % LIGHT_GRID_TILE_BLOCK_SIZE)) / LIGHT_GRID_TILE_BLOCK_SIZE;
		data.mWorkGroupsY = (height + (height % LIGHT_GRID_TILE_BLOCK_SIZE)) / LIGHT_GRID_TILE_BLOCK_SIZE;

		// \note Create buffer for initialization of mLightIndexCountersBuffer buffer on per frame basis
		auto lightIndexCountersInitializerCreateResult = CreateLightIndexCountersBuffer(graphicsObjectManager, true, "LightIndexCountersInitial");
		if (lightIndexCountersInitializerCreateResult.HasError())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(lightIndexCountersInitializerCreateResult.GetError());
		}

		data.mLightIndexCountersInitializerBufferHandle = lightIndexCountersInitializerCreateResult.Get();

		// \note Create frustums buffer
		const USIZE frustumsBufferSize = data.mWorkGroupsX * data.mWorkGroupsY * sizeof(TVector4) * 4;

		auto createFrustumsBufferResult = graphicsObjectManager->CreateBuffer(
			{
				E_BUFFER_USAGE_TYPE::DEFAULT,
				E_BUFFER_TYPE::STRUCTURED,
				frustumsBufferSize,
				nullptr,
				frustumsBufferSize,
				true,
				sizeof(TVector4) * 4,
				E_STRUCTURED_BUFFER_TYPE::DEFAULT
			});

		if (createFrustumsBufferResult.HasError())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(createFrustumsBufferResult.GetError());
		}

		data.mTileFrustumsBufferHandle = createFrustumsBufferResult.Get();
		data.mIsTileFrustumsInitialized = false;

		return Wrench::TOkValue<TLightCullingData>(data);
	}


	static E_RESULT_CODE CreateRandomTexture(TPtr<IResourceManager> pResourceManager)
	{
		TDE2_PROFILER_SCOPE("CreateRandomTexture");

		constexpr U32 RND_TEXTURE_SIZES = 1024;
		const TTexture2DParameters randTextureParams(RND_TEXTURE_SIZES, RND_TEXTURE_SIZES, FT_FLOAT4);
		
		TPtr<ITexture2D> pTexture = pResourceManager->GetResource<ITexture2D>(pResourceManager->Create<ITexture2D>(CProjectSettings::Get()->mGraphicsSettings.mRandomTextureId, randTextureParams));
		if (!pTexture)
		{
			return RC_FAIL;
		}

		TPtr<IJobManager> pJobManager = pResourceManager->GetJobManager();

		pJobManager->SubmitJob(nullptr, [pJobManager, pTexture](auto)
			{
				std::vector<F32> values(RND_TEXTURE_SIZES * RND_TEXTURE_SIZES * sizeof(F32) * 4);
				
				TJobCounter counter;

				pJobManager->SubmitMultipleJobs(&counter, static_cast<U32>(values.size()), 1024, [&values](const TJobArgs& arg)
					{
						values[arg.mJobIndex] = CRandomUtils::GetRandF32Value({0.0f, 1.0f});
					});

				pJobManager->WaitForJobCounter(counter);

				E_RESULT_CODE result = pTexture->WriteData({ 0, 0, static_cast<I32>(RND_TEXTURE_SIZES), static_cast<I32>(RND_TEXTURE_SIZES) }, reinterpret_cast<const U8*>(values.data()));
				TDE2_ASSERT(RC_OK == result);
			});

		return RC_OK;
	}


	E_RESULT_CODE CForwardRenderer::Init(const TRendererInitParams& params)
	{
		TDE2_PROFILER_SCOPE("CForwardRenderer::Init");

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!params.mAllocatorFactoryFunctor || !params.mpGraphicsContext || !params.mpResourceManager || !params.mpWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = params.mpGraphicsContext;
		mpResourceManager = params.mpResourceManager;
		mpWindowSystem    = params.mpWindowSystem;

		auto pEventManager = mpWindowSystem->GetEventManager();
		if (pEventManager)
		{
			pEventManager->Subscribe(TOnWindowResized::GetTypeId(), this);
		}

		E_RESULT_CODE result = RC_OK;

		mpFrameGraph = CreateFrameGraph(mpGraphicsContext.Get(), result);

		auto allocatorFactory = params.mAllocatorFactoryFunctor;
				
		IAllocator* pCurrAllocator = nullptr;

		void* pCurrMemoryBlock = nullptr;

		for (U8 i = 0; i < NumOfRenderQueuesGroup; ++i)
		{
			pCurrAllocator = allocatorFactory(PerRenderQueueMemoryBlockSize, result);

			if (result != RC_OK)
			{
				return result;
			}

			/// \note this CRenderQueue's instance now owns this allocator
			mpRenderQueues[i] = TPtr<CRenderQueue>(CreateRenderQueue(pCurrAllocator, result));

			if (result != RC_OK)
			{
				return result;
			}
			
			LOG_MESSAGE(std::string("[Forward Renderer] A new render queue buffer was created ( mem-size : ").append(std::to_string(PerRenderQueueMemoryBlockSize / 1024)).
																											  append(" KiB; group-type : ").
																											  append(std::to_string(i)).
																											  append(")"));
		}

		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();

		mpGlobalShaderProperties = TPtr<IGlobalShaderProperties>(CreateGlobalShaderProperties(pGraphicsObjectManager, result));

		/// \todo fill in data into TConstantsShaderData buffer
		TConstantShaderData constShaderData { static_cast<U32>(CProjectSettings::Get()->mGraphicsSettings.mIsGPUParticlesSimulationEnabled) };
		mpGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_CONSTANTS, reinterpret_cast<const U8*>(&constShaderData), sizeof(constShaderData));
		if (result != RC_OK)
		{
			return result;
		}
		
		TRareUpdateShaderData rareUpdatedData{ mpWindowSystem->GetWidth(), mpWindowSystem->GetHeight() };
		mpGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_RARE_UDATED, reinterpret_cast<const U8*>(&rareUpdatedData), sizeof(rareUpdatedData));

		auto debugUtilityResult = pGraphicsObjectManager->CreateDebugUtility(mpResourceManager.Get(), this);
		if (debugUtilityResult.HasError())
		{
			return debugUtilityResult.GetError();
		}

		mpDebugUtility = debugUtilityResult.Get();

		auto lightGridDataResult = InitLightGrid(params);
		if (lightGridDataResult.HasError())
		{
			return lightGridDataResult.GetError();
		}

		mLightGridData = lightGridDataResult.Get();

		result = InitStaticRenderPasses(mpGraphicsContext, mpResourceManager, mpGlobalShaderProperties);
		if (RC_OK != result)
		{
			return result;
		}

		result = CreateRandomTexture(mpResourceManager);
		if (RC_OK != result)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}


	static inline void SubmitCommandsToDraw(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TPtr<IGlobalShaderProperties> pGlobalShaderProperties,
									TPtr<CRenderQueue> pRenderQueue, U32 startRange, U32 endRange)
	{
		CRenderQueue::CRenderQueueIterator iter = pRenderQueue->GetIterator();

		TRenderCommand* pCurrDrawCommand = nullptr;

		const TRenderCommandSubmitParams drawContext { pGraphicsContext.Get(), pResourceManager.Get(), pGlobalShaderProperties.Get() };

		while (iter.HasNext())
		{
			if (iter.GetIndex() < startRange)
			{
				++iter;
				continue;
			}

			if (iter.GetIndex() >= endRange)
			{
				break;
			}

			pCurrDrawCommand = *(iter++);

			if (!pCurrDrawCommand)
			{
				continue;
			}

			pCurrDrawCommand->Submit(drawContext);
		}
	}


	static inline void ExecuteDrawCommands(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TPtr<IGlobalShaderProperties> pGlobalShaderProperties,
									TPtr<CRenderQueue> pCommandsBuffer, bool shouldClearBuffers, U32 startRange, U32 endRange)
	{
		if (pCommandsBuffer->IsEmpty())
		{
			return;
		}

		pCommandsBuffer->Sort();
		SubmitCommandsToDraw(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pCommandsBuffer, startRange, endRange);

		if (shouldClearBuffers)
		{
			pCommandsBuffer->Clear();
		}
	}


#if TDE2_EDITORS_ENABLED
	static inline void DumpFrameGraph(TPtr<CFrameGraph> pFrameGraph)
	{
		E_RESULT_CODE result = RC_OK;

		auto pFileStream = TPtr<TDEngine2::IStream>(CreateFileOutputStream("ForwardRenderer_FrameGraphDump.dot", result));
		ITextFileWriter* pDumpWriter = dynamic_cast<ITextFileWriter*>(CreateTextFileWriter(nullptr, pFileStream, result));

		pFrameGraph->Dump(pDumpWriter);
		pDumpWriter->Close();

		TriggetDumpFrameGraph.Set(0);
	}
	
#endif


	E_RESULT_CODE CForwardRenderer::Draw(F32 currTime, F32 deltaTime)
	{
		TDE2_PROFILER_SCOPE("Renderer::Draw"); 

		if (!mIsInitialized)
		{
			return RC_FAIL;
		}
				
		{
			TDE2_BUILTIN_SPEC_PROFILER_EVENT(E_SPECIAL_PROFILE_EVENT::RENDER);
			TDE2_STATS_COUNTER_SET(mDrawCallsCount, 0);

			_prepareFrame(currTime, deltaTime);

			mpFrameGraph->Reset();

			TFrameGraphBlackboard frameGraphBlackboard;
			frameGraphBlackboard.mBackBufferHandle = mpFrameGraph->ImportResource("BackBuffer", TFrameGraphTexture::TDesc { }, TFrameGraphTexture{ TTextureHandleId::Invalid });
			
			if (auto pColorLUT = mpResourceManager->GetResource<ITexture2D>(mpResourceManager->Load<ITexture2D>(mpCurrPostProcessingProfile->GetColorGradingParameters().mLookUpTextureId)))
			{
				frameGraphBlackboard.mColorGradingLUTHandle = mpFrameGraph->ImportResource("ColorGradingLUT", TFrameGraphTexture::TDesc{ }, TFrameGraphTexture{ pColorLUT->GetHandle() });
			}
			
			frameGraphBlackboard.mLightCullingData.mTileFrustumsBufferHandle = mpFrameGraph->ImportResource("TileFrustums", TFrameGraphBuffer::TDesc { }, TFrameGraphBuffer{ mLightGridData.mTileFrustumsBufferHandle });
			frameGraphBlackboard.mLightCullingData.mLightIndexCountersInitializerBufferHandle = mpFrameGraph->ImportResource("InitialLightIndexCounters", TFrameGraphBuffer::TDesc { }, TFrameGraphBuffer{ mLightGridData.mLightIndexCountersInitializerBufferHandle });

			TPassInvokeContext passInvokeContext
			{
				mpGraphicsContext,
				mpResourceManager,
				mpGlobalShaderProperties,
				mpWindowSystem->GetWidth(),
				mpWindowSystem->GetHeight()
			};

			// \note editor mode (draw into selection buffer)
#if TDE2_EDITORS_ENABLED
			CRenderSelectionBufferPass{ passInvokeContext, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_EDITOR_ONLY)] }.AddPass(mpFrameGraph, frameGraphBlackboard, mpSelectionManager);
#endif

			// \note directional shadow pass
			if (CGameUserSettings::Get()->mpIsShadowMappingEnabledCVar->Get())
			{
				CSunLightShadowPass{ passInvokeContext, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_SHADOW_PASS)] }.AddPass(mpFrameGraph, frameGraphBlackboard);

				U32 currPointLightIndex = 0;

				// \note omni shadow passes
				const USIZE pointLightsCount = std::min<USIZE>(
					std::count_if(mActiveLightSources.cbegin(), mActiveLightSources.cend(), [](auto&& currLightSource) { return static_cast<I32>(E_LIGHT_SOURCE_TYPE::POINT) == currLightSource.mLightType; }),
					static_cast<USIZE>(CGameUserSettings::Get()->mpMaxOmniLightShadowMapsCVar->Get()));

				for (auto&& currLightSource : mActiveLightSources)
				{
					if (static_cast<I32>(E_LIGHT_SOURCE_TYPE::POINT) != currLightSource.mLightType)
					{
						continue;
					}

					if (currPointLightIndex >= static_cast<U32>(CGameUserSettings::Get()->mpMaxOmniLightShadowMapsCVar->Get()))
					{
						break;
					}

					COmniLightShadowPass{ passInvokeContext, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_SHADOW_PASS)], pointLightsCount }.AddPass(mpFrameGraph, frameGraphBlackboard, currPointLightIndex);

					++currPointLightIndex;
				}
			}

			// \note depth pre-pass
			CDepthPrePass{ passInvokeContext, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_DEPTH_PREPASS)] }.AddPass(mpFrameGraph, frameGraphBlackboard);

			CSkyGeometryRenderPass{ passInvokeContext, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_OPAQUE_GEOMETRY)] }.AddPass(mpFrameGraph, frameGraphBlackboard, true); // \todo replace with configuration of hdr

			if (CGameUserSettings::Get()->mpIsVolumetricCloudsEnabledCVar->Get())
			{
				CVolumetricCloudsMainPass{ passInvokeContext }.AddPass(mpFrameGraph, frameGraphBlackboard);
				CVolumetricCloudsUpscalePass{ passInvokeContext }.AddPass(mpFrameGraph, frameGraphBlackboard);

				pVolumetricCloudsComposePass->AddPass(mpFrameGraph, frameGraphBlackboard);
			}

			CUploadLightsPass{ passInvokeContext }.AddPass(mpFrameGraph, frameGraphBlackboard, mActiveLightSources);
			CLightCullingPass{ passInvokeContext }.AddPass(mpFrameGraph, frameGraphBlackboard);

			// \note main pass
			COpaqueRenderPass{ passInvokeContext, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_OPAQUE_GEOMETRY)] }.AddPass(mpFrameGraph, frameGraphBlackboard, true); // \todo replace with configuration of hdr
			CTransparentRenderPass{ passInvokeContext, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_TRANSPARENT_GEOMETRY)] }.AddPass(mpFrameGraph, frameGraphBlackboard); 
			CSpritesRenderPass{ passInvokeContext, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_SPRITES)] }.AddPass(mpFrameGraph, frameGraphBlackboard);
			CDebugRenderPass{ passInvokeContext, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_DEBUG)] }.AddPass(mpFrameGraph, frameGraphBlackboard);

			// \note lights heatmap debug pass
			if (EnableLightsHeatMapCfgVar.Get())
			{
				pLightsHeatmapDebugPostProcessPass->AddPass(mpFrameGraph, frameGraphBlackboard, frameGraphBlackboard.mMainRenderTargetHandle);
			}

			// \note eye-adaptation pass
			pExtractLuminancePostProcessPass->AddPass(mpFrameGraph, frameGraphBlackboard);
			pCalcAverageLuminancePostProcessPass->AddPass(mpFrameGraph, frameGraphBlackboard, mpCurrPostProcessingProfile); // \todo Replace coeff with correct value from post-processing profile

			const auto& bloomParameters = mpCurrPostProcessingProfile->GetBloomParameters();
			const auto& toneMappingParameters = mpCurrPostProcessingProfile->GetToneMappingParameters();

			if (mpCurrPostProcessingProfile->IsPostProcessingEnabled() && bloomParameters.mIsEnabled)
			{
				const U16 downsampleCoeff = 1 << (TPostProcessingProfileParameters::TBloomParameters::mMaxQuality - bloomParameters.mQuality + 1);

				// \note bloom threshold
				pBloomThresholdPostProcessPass->AddPass(mpFrameGraph, frameGraphBlackboard, mpWindowSystem->GetWidth() / downsampleCoeff, mpWindowSystem->GetHeight() / downsampleCoeff, mpCurrPostProcessingProfile);

				const U32 bloomTargetWidth = mpWindowSystem->GetWidth() / downsampleCoeff;
				const U32 bloomTargetHeight = mpWindowSystem->GetHeight() / downsampleCoeff;

				TFrameGraphResourceHandle horizontalBlurTargetHandle = pBlurPostProcessPass->AddPass(
					mpFrameGraph, 
					frameGraphBlackboard, 
					frameGraphBlackboard.mBloomThresholdTargetHandle, 
					bloomTargetWidth,
					bloomTargetHeight,
					TVector4(
						bloomParameters.mSmoothness,
						0.0f, 
						1.0f / static_cast<F32>(bloomTargetWidth),
						1.0f / static_cast<F32>(bloomTargetHeight)),
					bloomParameters.mSamplesCount); // \todo replace with configuration of hdr support

				TFrameGraphResourceHandle verticalBlurTargetHandle = pBlurPostProcessPass->AddPass(
					mpFrameGraph, 
					frameGraphBlackboard, 
					horizontalBlurTargetHandle,
					bloomTargetWidth,
					bloomTargetHeight,
					TVector4(
						bloomParameters.mSmoothness,
						CMathConstants::Pi * 0.5f,
						1.0f / static_cast<F32>(bloomTargetWidth),
						1.0f / static_cast<F32>(bloomTargetHeight)),
					bloomParameters.mSamplesCount); // \todo replace with configuration of hdr support

				pBloomComposePostProcessPass->AddPass(mpFrameGraph, frameGraphBlackboard, verticalBlurTargetHandle);
			}
			
			// \note ui pass
			CUIRenderPass{ passInvokeContext, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_OVERLAY)] }.AddPass(mpFrameGraph, frameGraphBlackboard);

			// \note compose pass + tone mapping
			pToneMappingComposePostProcessPass->AddPass(mpFrameGraph, frameGraphBlackboard, mpWindowSystem->GetWidth(), mpWindowSystem->GetHeight(), true, mpCurrPostProcessingProfile); // \todo replace with configuration of hdr support

			// \note imgui pass
			CDebugUIRenderPass{ passInvokeContext, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_DEBUG_UI)] }.AddPass(mpFrameGraph, frameGraphBlackboard);

			mpFrameGraph->Compile();
			mpFrameGraph->Execute();
		}

		mpRenderQueues[static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_SHADOW_PASS)]->Clear();

		{
			TDE2_BUILTIN_SPEC_PROFILER_EVENT(E_SPECIAL_PROFILE_EVENT::PRESENT);
			mpGraphicsContext->Present();
		}

		mpDebugUtility->PostRender();

#if TDE2_EDITORS_ENABLED
		if (TriggetDumpFrameGraph.Get())
		{
			DumpFrameGraph(mpFrameGraph);
		}
#endif

		return RC_OK;
	}

	void CForwardRenderer::SetCamera(const ICamera* pCamera)
	{
		mpMainCamera = pCamera;
	}

	E_RESULT_CODE CForwardRenderer::SetPostProcessProfile(const IPostProcessingProfile* pProfileResource)
	{
		if (!pProfileResource)
		{
			return RC_INVALID_ARGS;
		}

		mpCurrPostProcessingProfile = pProfileResource;
		
		return RC_OK;
	}

	E_RESULT_CODE CForwardRenderer::SetSelectionManager(ISelectionManager* pSelectionManager)
	{
		if (!pSelectionManager)
		{
			return RC_INVALID_ARGS;
		}

		mpSelectionManager = pSelectionManager;

		return RC_OK;
	}

	E_RESULT_CODE CForwardRenderer::SetLightingData(const TLightingShaderData& commonLightData, const TLightsDataArray& activeLightSources)
	{
		if (Length(commonLightData.mSunLightDirection) < 1e-3f)
		{
			LOG_ERROR("[ForwardRenderer] Sun light's direction could not be a zero vector");
			TDE2_ASSERT(false);

			return RC_INVALID_ARGS;
		}

		mCommonLightingData = commonLightData;
		mActiveLightSources = activeLightSources;

		return RC_OK;
	}

	E_RESULT_CODE CForwardRenderer::OnEvent(const TBaseEvent* pEvent)
	{
		if (pEvent->GetEventType() != TOnWindowResized::GetTypeId())
		{
			return RC_OK;
		}

		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();
		
		E_RESULT_CODE result = RC_OK;
		
		// \todo Later move into Draw method and set only the flag here to invoke the update
		TRareUpdateShaderData rareUpdatedData{ mpWindowSystem->GetWidth(), mpWindowSystem->GetHeight() };
		mpGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_RARE_UDATED, reinterpret_cast<const U8*>(&rareUpdatedData), sizeof(rareUpdatedData));

		auto lightGridInitResult = InitLightGrid({ mpGraphicsContext, mpResourceManager, mpWindowSystem, nullptr });
		if (lightGridInitResult.IsOk())
		{
			mLightGridData = lightGridInitResult.Get();
		}

		TDE2_ASSERT(RC_OK == result);

		return RC_OK;
	}

	TEventListenerId CForwardRenderer::GetListenerId() const
	{
		return static_cast<TEventListenerId>(EST_RENDERER);
	}
	
	E_ENGINE_SUBSYSTEM_TYPE CForwardRenderer::GetType() const
	{
		return EST_RENDERER;
	}

	CRenderQueue* CForwardRenderer::GetRenderQueue(E_RENDER_QUEUE_GROUP queueType)
	{
		return mpRenderQueues[static_cast<U8>(queueType)].Get();
	}

	TPtr<IResourceManager> CForwardRenderer::GetResourceManager() const
	{
		return mpResourceManager;
	}

	TPtr<IGlobalShaderProperties> CForwardRenderer::GetGlobalShaderProperties() const
	{
		return mpGlobalShaderProperties;
	}

	void CForwardRenderer::_prepareFrame(F32 currTime, F32 deltaTime)
	{
		TDE2_PROFILER_SCOPE("Renderer::PreRender");
		TDE_RENDER_SECTION(mpGraphicsContext, "PreRender");

		///set up global shader properties for TPerFrameShaderData buffer
		TPerFrameShaderData perFrameShaderData;

		perFrameShaderData.mLightingData = mCommonLightingData;
		
		if (mpMainCamera)
		{
			perFrameShaderData.mProjMatrix             = Transpose(mpMainCamera->GetProjMatrix());
			perFrameShaderData.mViewMatrix             = Transpose(mpMainCamera->GetViewMatrix());
			perFrameShaderData.mInvProjMatrix          = Transpose(Inverse(mpMainCamera->GetProjMatrix()));
			perFrameShaderData.mInvViewMatrix          = Transpose(Inverse(mpMainCamera->GetViewMatrix()));
			perFrameShaderData.mInvViewProjMatrix      = Transpose(mpMainCamera->GetInverseViewProjMatrix());
			perFrameShaderData.mCameraPosition         = TVector4(mpMainCamera->GetPosition(), 1.0f);
			perFrameShaderData.mCameraProjectionParams = TVector4(mpMainCamera->GetNearPlane(), mpMainCamera->GetFarPlane(), 0.0f, 0.0f);
		}

		perFrameShaderData.mTime = TVector4(currTime, deltaTime, 0.0f, 0.0f);

		mpGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_PER_FRAME, reinterpret_cast<const U8*>(&perFrameShaderData), sizeof(perFrameShaderData));		
		mpGlobalShaderProperties->Bind();

		mpGraphicsContext->ClearBackBuffer(TColor32F(0.0f, 0.0f, 0.5f, 1.0f));
		mpGraphicsContext->ClearDepthBuffer(1.0f);
		mpGraphicsContext->ClearStencilBuffer(0x0);

		mpDebugUtility->PreRender();

		if (!mLightGridData.mIsTileFrustumsInitialized)
		{
			PrepareTileFrustums(mLightGridData, mpGraphicsContext, mpResourceManager);
			mLightGridData.mIsTileFrustumsInitialized = true;
		}
	}

	E_RESULT_CODE CForwardRenderer::_onFreeInternal()
	{
		DestroyStaticRenderPasses();
		return RC_OK;
	}


	TDE2_API IRenderer* CreateForwardRenderer(const TRendererInitParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IRenderer, CForwardRenderer, result, params);
	}
}