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
#include "../../include/core/memory/IAllocator.h"
#include "../../include/core/memory/CLinearAllocator.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/core/IEventManager.h"
#include "../../include/core/CGameUserSettings.h"
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


	typedef struct TFramePostProcessorParameters
	{
		IRenderer* mpRenderer;
		IGraphicsObjectManager* mpGraphicsObjectManager;
		IWindowSystem* mpWindowSystem;
	} TFramePostProcessorParameters, * TFramePostProcessorParametersPtr;


	enum class E_FRAME_RENDER_PARAMS_FLAGS : U32
	{
		CLEAR_RENDER_TARGET = 1 << 0,
		BIND_DEPTH_BUFFER = 1 << 1,
		RENDER_MAIN = 1 << 2,
		RENDER_UI = 1 << 3,
		NONE = 0
	};


	TDE2_DECLARE_BITMASK_OPERATORS_INTERNAL(E_FRAME_RENDER_PARAMS_FLAGS);


	constexpr U32 TILE_FRUSTUMS_BUFFER_SLOT = 0;
	constexpr U32 FRUSTUM_TILES_PER_GROUP = 16;

	constexpr U32 OPAQUE_VISIBLE_LIGHTS_BUFFER_SLOT = 11;
	constexpr U32 TRANSPARENT_VISIBLE_LIGHTS_BUFFER_SLOT = 12;
	constexpr U32 LIGHT_INDEX_COUNTERS_BUFFER_SLOT = 13;

	constexpr U32 LIGHT_GRID_TEXTURE_SLOT = 10;
	constexpr U32 VISIBLE_LIGHTS_BUFFER_SLOT = 11;

	constexpr U32 SUN_SHADOWS_MAP_SLOT = 0;
	constexpr U32 POINT_LIGHT_0_SHADOWS_MAP_SLOT = 1;


	static inline void ExecuteDrawCommands(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, 
		TPtr<IGlobalShaderProperties> pGlobalShaderProperties, TPtr<CRenderQueue> pCommandsBuffer, bool shouldClearBuffers, 
		U32 upperRenderIndexLimit = (std::numeric_limits<U32>::max)());


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
		TFrameGraphResourceHandle              mMainRenderTargetHandle = TFrameGraphResourceHandle::Invalid;
		TFrameGraphResourceHandle              mDepthBufferHandle = TFrameGraphResourceHandle::Invalid;

		TFrameGraphResourceHandle              mSunLightShadowMapHandle = TFrameGraphResourceHandle::Invalid;
		std::vector<TFrameGraphResourceHandle> mOmniLightShadowMapHandles;

		TFrameGraphResourceHandle              mLightsBufferHandle = TFrameGraphResourceHandle::Invalid;
		TLightCullData                         mLightCullingData;

		TFrameGraphResourceHandle mVolumetricCloudsMainTargetHandle = TFrameGraphResourceHandle::Invalid;
		TFrameGraphResourceHandle mVolumetricCloudsFullSizeTargetHandle = TFrameGraphResourceHandle::Invalid;
	};


	struct TPassInvokeContext
	{
		TPtr<IGraphicsContext>        mpGraphicsContext = nullptr;
		TPtr<IResourceManager>        mpResourceManager = nullptr;
		TPtr<IGlobalShaderProperties> mpGlobalShaderProperties = nullptr;
		TPtr<CRenderQueue>            mpCommandsBuffer = nullptr;

		U32                           mWindowWidth = 0;
		U32                           mWindowHeight = 0;
	};


	class CBaseRenderPass
	{
		public:
			explicit CBaseRenderPass(const TPassInvokeContext& context) :
				mContext(context)
			{
			}

		protected:
			const TPassInvokeContext& mContext;
	};


#if TDE2_EDITORS_ENABLED

	class CRenderSelectionBufferPass
	{
		public:
			struct TAddPassInvokeParams
			{
				TPtr<IGraphicsContext>        mpGraphicsContext = nullptr;
				TPtr<IResourceManager>        mpResourceManager = nullptr;
				TPtr<IGlobalShaderProperties> mpGlobalShaderProperties = nullptr;
				TPtr<CRenderQueue>            mpCommandsBuffer = nullptr;

				U32                           mWindowWidth = 0;
				U32                           mWindowHeight = 0;
			};

		public:
			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard, const TAddPassInvokeParams& context)
			{
				constexpr const C8* SELECTION_BUFFER_PASS_ID = "SelectionBuffer";

				struct TPassData
				{
					TFrameGraphResourceHandle mDepthBufferHandle = TFrameGraphResourceHandle::Invalid;
				};

				auto&& output = pFrameGraph->AddPass<TPassData>("RenderSelectionBuffer", [&](CFrameGraphBuilder& builder, TPassData& data)
					{
						/*TFrameGraphTexture::TDesc depthBufferParams{};

						depthBufferParams.mWidth = context.mWindowWidth;
						depthBufferParams.mHeight = context.mWindowHeight;
						depthBufferParams.mFormat = FT_D32;
						depthBufferParams.mNumOfMipLevels = 1;
						depthBufferParams.mNumOfSamples = 1;
						depthBufferParams.mSamplingQuality = 0;
						depthBufferParams.mType = E_TEXTURE_IMPL_TYPE::TEXTURE_2D;
						depthBufferParams.mUsageType = E_TEXTURE_IMPL_USAGE_TYPE::STATIC;
						depthBufferParams.mBindFlags = E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE | E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER;
						depthBufferParams.mName = SELECTION_BUFFER_PASS_ID;
						depthBufferParams.mFlags = E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT;

						data.mDepthBufferHandle = builder.Create<TFrameGraphTexture>(SELECTION_BUFFER_PASS_ID, depthBufferParams);
						data.mDepthBufferHandle = builder.Write(data.mDepthBufferHandle);

						TDE2_ASSERT(data.mDepthBufferHandle != TFrameGraphResourceHandle::Invalid);*/

						builder.MarkAsPersistent(); // TODO remove when the whole graph will be provided
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);

						TDE2_PROFILER_SCOPE("RenderSelectionBuffer");
						TDE_RENDER_SECTION(pGraphicsContext, "RenderSelectionBuffer");

						//ProcessEditorSelectionBuffer(mpGraphicsContext, mpResourceManager, mpGlobalShaderProperties, mpSelectionManager, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_EDITOR_ONLY)]);

						//TFrameGraphTexture& depthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mDepthBufferHandle);

						//pGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(context.mWindowWidth), static_cast<F32>(context.mWindowHeight), 0.0f, 1.0f);
						//pGraphicsContext->BindDepthBufferTarget(depthBufferTarget.mTextureHandle, true);
						//pGraphicsContext->ClearDepthBuffer(1.0f);

						//ExecuteDrawCommands(pGraphicsContext, context.mpResourceManager, context.mpGlobalShaderProperties, context.mpCommandsBuffer, false); // \todo Replace false with true when the graph will be completed

						//pGraphicsContext->BindDepthBufferTarget(TTextureHandleId::Invalid);
					});

				frameGraphBlackboard.mDepthBufferHandle = output.mDepthBufferHandle;
			}
	};

#endif


	class CDepthPrePass: public CBaseRenderPass
	{
		public:
			explicit CDepthPrePass(const TPassInvokeContext& context):
				CBaseRenderPass(context)
			{
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard)
			{
				constexpr const C8* DEPTH_PRE_PASS_BUFFER_ID = "PrePassDepthBuffer";

				struct TPassData
				{
					TFrameGraphResourceHandle mDepthBufferHandle = TFrameGraphResourceHandle::Invalid;
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

					TDE2_ASSERT(data.mDepthBufferHandle != TFrameGraphResourceHandle::Invalid);
				}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext)
				{
					auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);

					TDE2_PROFILER_SCOPE("DepthPrePass");
					TDE_RENDER_SECTION(pGraphicsContext, "DepthPrePass");
					
					TFrameGraphTexture& depthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mDepthBufferHandle);

					pGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(mContext.mWindowWidth), static_cast<F32>(mContext.mWindowHeight), 0.0f, 1.0f);
					pGraphicsContext->BindDepthBufferTarget(depthBufferTarget.mTextureHandle, true);
					pGraphicsContext->ClearDepthBuffer(1.0f);

					ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mContext.mpCommandsBuffer, false); // \todo Replace false with true when the graph will be completed

					pGraphicsContext->BindDepthBufferTarget(TTextureHandleId::Invalid);
				});

				frameGraphBlackboard.mDepthBufferHandle = output.mDepthBufferHandle;
			}
	};


	class CSunLightShadowPass: public CBaseRenderPass
	{
		public:
			explicit CSunLightShadowPass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
			{
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard)
			{
				constexpr const C8* SUN_LIGHT_SHADOW_MAP_ID = "SunLightShadowMap";

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
					shadowMapParams.mName            = SUN_LIGHT_SHADOW_MAP_ID;
					shadowMapParams.mFlags           = E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT;
					shadowMapParams.mArraySize       = CGameUserSettings::Get()->mpShadowCascadesCountCVar->Get();

					data.mShadowMapHandle = builder.Create<TFrameGraphTexture>(SUN_LIGHT_SHADOW_MAP_ID, shadowMapParams);
					data.mShadowMapHandle = builder.Write(data.mShadowMapHandle);

					TDE2_ASSERT(data.mShadowMapHandle != TFrameGraphResourceHandle::Invalid);
				}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext)
				{
					auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);

					TDE2_PROFILER_SCOPE("RenderSunLightShadows");
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

					ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mContext.mpCommandsBuffer, false);

					pGraphicsContext->BindDepthBufferTarget(TTextureHandleId::Invalid);
				});

				frameGraphBlackboard.mSunLightShadowMapHandle = output.mShadowMapHandle;
			}
	};


	class COmniLightShadowPass: public CBaseRenderPass
	{
		public:
			explicit COmniLightShadowPass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
			{
			}

			void AddPass(TPtr<CFrameGraph> pFrameGraph, TFrameGraphBlackboard& frameGraphBlackboard, U32 lightIndex = 0)
			{
				struct TPassData
				{
					TFrameGraphResourceHandle mShadowMapHandle = TFrameGraphResourceHandle::Invalid;
				};

				const U32 shadowMapSizes = static_cast<U32>(CGameUserSettings::Get()->mpShadowMapSizesCVar->Get());
				TDE2_ASSERT(shadowMapSizes > 0 && shadowMapSizes < 65536);

				auto&& output = pFrameGraph->AddPass<TPassData>(Wrench::StringUtils::Format("OmniLight{0}ShadowPass", lightIndex), [&](CFrameGraphBuilder& builder, TPassData& data)
					{
						TFrameGraphTexture::TDesc shadowMapParams{};

						const std::string OMNI_LIGHT_SHADOW_MAP_ID = Wrench::StringUtils::Format("OmniLightShadowMap_{0}", lightIndex);

						shadowMapParams.mWidth           = shadowMapSizes;
						shadowMapParams.mHeight          = shadowMapSizes;
						shadowMapParams.mFormat          = FT_D32;
						shadowMapParams.mNumOfMipLevels  = 1;
						shadowMapParams.mNumOfSamples    = 1;
						shadowMapParams.mSamplingQuality = 0;
						shadowMapParams.mType            = E_TEXTURE_IMPL_TYPE::CUBEMAP;
						shadowMapParams.mUsageType       = E_TEXTURE_IMPL_USAGE_TYPE::STATIC;
						shadowMapParams.mBindFlags       = E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE | E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER;
						shadowMapParams.mName            = OMNI_LIGHT_SHADOW_MAP_ID.c_str();
						shadowMapParams.mFlags           = E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT;

						data.mShadowMapHandle = builder.Create<TFrameGraphTexture>(OMNI_LIGHT_SHADOW_MAP_ID, shadowMapParams);
						data.mShadowMapHandle = builder.Write(data.mShadowMapHandle);

						TDE2_ASSERT(data.mShadowMapHandle != TFrameGraphResourceHandle::Invalid);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);

						TDE2_PROFILER_SCOPE("RenderOmniLightShadows" + std::to_string(lightIndex));
						TDE_RENDER_SECTION(pGraphicsContext, "RenderOmniLightShadows" + std::to_string(lightIndex));

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

						ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mContext.mpCommandsBuffer, false);

						pGraphicsContext->BindDepthBufferTarget(TTextureHandleId::Invalid);
					});

				frameGraphBlackboard.mOmniLightShadowMapHandles.emplace_back(output.mShadowMapHandle);
			}
	};


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
						constexpr const C8* LIGHTS_BUFFER_ID = "ActiveLightsBuffer";

						TFrameGraphBuffer::TDesc lightsBufferParams{};
						lightsBufferParams.mBufferType                = E_BUFFER_TYPE::STRUCTURED;
						lightsBufferParams.mDataSize                  = MaxLightsCount * sizeof(TLightData);
						lightsBufferParams.mElementStrideSize         = sizeof(TLightData);
						lightsBufferParams.mFlags                     = E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT;
						lightsBufferParams.mIsUnorderedAccessResource = false;
						lightsBufferParams.mName                      = LIGHTS_BUFFER_ID;
						lightsBufferParams.mpDataPtr                  = nullptr;
						lightsBufferParams.mStructuredBufferType      = E_STRUCTURED_BUFFER_TYPE::DEFAULT;
						lightsBufferParams.mTotalBufferSize           = lightsBufferParams.mDataSize;
						lightsBufferParams.mUsageType                 = E_BUFFER_USAGE_TYPE::DYNAMIC;

						data.mLightsBufferHandle = builder.Create<TFrameGraphBuffer>(LIGHTS_BUFFER_ID, lightsBufferParams);
						data.mLightsBufferHandle = builder.Write(data.mLightsBufferHandle);

						TDE2_ASSERT(data.mLightsBufferHandle != TFrameGraphResourceHandle::Invalid);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext)
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
						builder.Read(lightCullingData.mLightIndexCountersInitializerBufferHandle);

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
					}, [=](const TLightCullData& data, const TFramePassExecutionContext& executionContext)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						
						TDE2_PROFILER_SCOPE("LightCullingPass");
						TDE_RENDER_SECTION(pGraphicsContext, "LightCullingPass");

						E_RESULT_CODE result = RC_OK;
						defer([&result] { TDE2_ASSERT(RC_OK == result); });

						TFrameGraphBuffer& opaqueVisibleLightsBuffer = executionContext.mpOwnerGraph->GetResource<TFrameGraphBuffer>(data.mOpaqueVisibleLightsBufferHandle);
						TFrameGraphBuffer& transparentVisibleLightsBuffer = executionContext.mpOwnerGraph->GetResource<TFrameGraphBuffer>(data.mTransparentVisibleLightsBufferHandle);
						TFrameGraphBuffer& lightIndexCountersBufferHandle = executionContext.mpOwnerGraph->GetResource<TFrameGraphBuffer>(data.mLightIndexCountersBufferHandle);

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
							pGraphicsContext->SetTexture(pLightCullShader->GetResourceBindingSlot("OpaqueLightGridTexture"), opaqueLightGridTextureHandle.mTextureHandle);
							pGraphicsContext->SetSampler(pLightCullShader->GetResourceBindingSlot("OpaqueLightGridTexture"), linearSamplerHandle);
							pGraphicsContext->SetTexture(pLightCullShader->GetResourceBindingSlot("TransparentLightGridTexture"), transparentLightGridTextureHandle.mTextureHandle);
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
							pLightCullShader->Unbind();
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

				TFrameGraphResourceHandle lightGridTextureHandle = builder.Create<TFrameGraphTexture>(lightGridTextureName, lightGridTextureParams);
				lightGridTextureHandle = builder.Write(lightGridTextureHandle);

				return Wrench::TOkValue<std::tuple<TFrameGraphResourceHandle, TFrameGraphResourceHandle>>(std::make_tuple(visibleLightsBufferHandle, lightGridTextureHandle));
			}
	};


	class COpaqueRenderPass : public CBaseRenderPass
	{
		public:
			explicit COpaqueRenderPass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
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

						TFrameGraphTexture::TDesc mainRenderTargetParams{};

						mainRenderTargetParams.mWidth           = mContext.mWindowWidth;
						mainRenderTargetParams.mHeight          = mContext.mWindowHeight;
						mainRenderTargetParams.mFormat          = isHDRSupportEnabled ? FT_FLOAT4 : FT_NORM_UBYTE4;
						mainRenderTargetParams.mNumOfMipLevels  = 1;
						mainRenderTargetParams.mNumOfSamples    = 1;
						mainRenderTargetParams.mSamplingQuality = 0;
						mainRenderTargetParams.mType            = E_TEXTURE_IMPL_TYPE::TEXTURE_2D;
						mainRenderTargetParams.mUsageType       = E_TEXTURE_IMPL_USAGE_TYPE::STATIC;
						mainRenderTargetParams.mBindFlags       = E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE | E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET;
						mainRenderTargetParams.mName            = "MainRenderTaget";
						mainRenderTargetParams.mFlags           = E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT;

						data.mMainRenderTargetHandle = builder.Create<TFrameGraphTexture>(mainRenderTargetParams.mName, mainRenderTargetParams);
						data.mMainRenderTargetHandle = builder.Write(data.mMainRenderTargetHandle);

						TDE2_ASSERT(data.mMainRenderTargetHandle != TFrameGraphResourceHandle::Invalid);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("OpaqueRenderPass");
						TDE_RENDER_SECTION(pGraphicsContext, "OpaqueRenderPass");

						TFrameGraphTexture& mainRenderTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mMainRenderTargetHandle);
						TFrameGraphTexture& depthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mDepthBufferHandle);

						pGraphicsContext->BindRenderTarget(0, mainRenderTarget.mTextureHandle);
						pGraphicsContext->BindDepthBufferTarget(depthBufferTarget.mTextureHandle);

						pGraphicsContext->ClearRenderTarget(static_cast<U8>(0), TColorUtils::mBlack);

						TFrameGraphBuffer& opaqueVisibleLightsBuffer = executionContext.mpOwnerGraph->GetResource<TFrameGraphBuffer>(lightCullData.mOpaqueVisibleLightsBufferHandle);
						TFrameGraphTexture& opaqueLightGridTexture = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(lightCullData.mOpaqueLightGridTextureHandle);

						pGraphicsContext->SetStructuredBuffer(VISIBLE_LIGHTS_BUFFER_SLOT, opaqueVisibleLightsBuffer.mBufferHandle, false);
						pGraphicsContext->SetTexture(LIGHT_GRID_TEXTURE_SLOT, opaqueLightGridTexture.mTextureHandle, false);

						ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mContext.mpCommandsBuffer, false); // \todo Replace false with true when the graph will be completed

						pGraphicsContext->BindDepthBufferTarget(TTextureHandleId::Invalid);
						pGraphicsContext->BindRenderTarget(0, TTextureHandleId::Invalid);
					});

				frameGraphBlackboard.mMainRenderTargetHandle = output.mMainRenderTargetHandle;
			}
	};


	class CTransparentRenderPass : public CBaseRenderPass
	{
		public:
			explicit CTransparentRenderPass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
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
						builder.Read(frameGraphBlackboard.mMainRenderTargetHandle);

						data.mMainRenderTargetHandle = builder.Write(frameGraphBlackboard.mMainRenderTargetHandle);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("TransparentRenderPass");
						TDE_RENDER_SECTION(pGraphicsContext, "TransparentRenderPass");

						TFrameGraphTexture& mainRenderTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mMainRenderTargetHandle);
						TFrameGraphTexture& depthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mDepthBufferHandle);

						pGraphicsContext->BindRenderTarget(0, mainRenderTarget.mTextureHandle);
						pGraphicsContext->BindDepthBufferTarget(depthBufferTarget.mTextureHandle);

						TFrameGraphBuffer& transparentVisibleLightsBuffer = executionContext.mpOwnerGraph->GetResource<TFrameGraphBuffer>(lightCullData.mTransparentVisibleLightsBufferHandle);
						TFrameGraphTexture& transparentLightGridTexture = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(lightCullData.mTransparentLightGridTextureHandle);

						pGraphicsContext->SetStructuredBuffer(VISIBLE_LIGHTS_BUFFER_SLOT, transparentVisibleLightsBuffer.mBufferHandle, false);
						pGraphicsContext->SetTexture(LIGHT_GRID_TEXTURE_SLOT, transparentLightGridTexture.mTextureHandle, false);

						ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mContext.mpCommandsBuffer, false); // \todo Replace false with true when the graph will be completed

						pGraphicsContext->BindDepthBufferTarget(TTextureHandleId::Invalid);
						pGraphicsContext->BindRenderTarget(0, TTextureHandleId::Invalid);
					});

				frameGraphBlackboard.mMainRenderTargetHandle = output.mMainRenderTargetHandle;
			}
	};


	class CSpritesRenderPass : public CBaseRenderPass
	{
		public:
			explicit CSpritesRenderPass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
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
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("SpritesRenderPass");
						TDE_RENDER_SECTION(pGraphicsContext, "SpritesRenderPass");

						TFrameGraphTexture& mainRenderTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mMainRenderTargetHandle);
						TFrameGraphTexture& depthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mDepthBufferHandle);

						pGraphicsContext->BindRenderTarget(0, mainRenderTarget.mTextureHandle);
						pGraphicsContext->BindDepthBufferTarget(depthBufferTarget.mTextureHandle);

						ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mContext.mpCommandsBuffer, false); // \todo Replace false with true when the graph will be completed

						pGraphicsContext->BindDepthBufferTarget(TTextureHandleId::Invalid);
						pGraphicsContext->BindRenderTarget(0, TTextureHandleId::Invalid);
					});

				frameGraphBlackboard.mMainRenderTargetHandle = output.mMainRenderTargetHandle;
			}
	};


	class CDebugRenderPass : public CBaseRenderPass
	{
		public:
			explicit CDebugRenderPass(const TPassInvokeContext& context) :
				CBaseRenderPass(context)
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
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("DebugRenderPass");
						TDE_RENDER_SECTION(pGraphicsContext, "DebugRenderPass");

						TFrameGraphTexture& mainRenderTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mMainRenderTargetHandle);
						TFrameGraphTexture& depthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mDepthBufferHandle);

						pGraphicsContext->BindRenderTarget(0, mainRenderTarget.mTextureHandle);
						pGraphicsContext->BindDepthBufferTarget(depthBufferTarget.mTextureHandle);

						ExecuteDrawCommands(pGraphicsContext, mContext.mpResourceManager, mContext.mpGlobalShaderProperties, mContext.mpCommandsBuffer, false); // \todo Replace false with true when the graph will be completed

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

						data.mVolumetricCloudsMainBufferHandle = builder.Create<TFrameGraphTexture>(volumetricCloudsMainBufferParams.mName, volumetricCloudsMainBufferParams);
						data.mVolumetricCloudsMainBufferHandle = builder.Write(data.mVolumetricCloudsMainBufferHandle);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("VolumetricCloudsMainPass");
						TDE_RENDER_SECTION(pGraphicsContext, "VolumetricCloudsMainPass");

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

						auto pVolumetricCloudsRenderPassShader = pResourceManager->GetResource<IShader>(pResourceManager->Load<IShader>("Shaders/Default/Volumetrics/VolumetricClouds.cshader")); // \todo add caching
						
						if (pVolumetricCloudsRenderPassShader)
						{
							pGraphicsContext->SetTexture(pVolumetricCloudsRenderPassShader->GetResourceBindingSlot("OutputTexture"), cloudsMainTarget.mTextureHandle);
							pGraphicsContext->SetSampler(pVolumetricCloudsRenderPassShader->GetResourceBindingSlot("OutputTexture"), linearSamplerHandle);
							pGraphicsContext->SetTexture(pVolumetricCloudsRenderPassShader->GetResourceBindingSlot("DepthTexture"), depthBufferTarget.mTextureHandle);
							pGraphicsContext->SetSampler(pVolumetricCloudsRenderPassShader->GetResourceBindingSlot("DepthTexture"), linearSamplerHandle);
							pGraphicsContext->SetTexture(pVolumetricCloudsRenderPassShader->GetResourceBindingSlot("MainTexture"), mainRenderTarget.mTextureHandle);
							pGraphicsContext->SetSampler(pVolumetricCloudsRenderPassShader->GetResourceBindingSlot("MainTexture"), linearSamplerHandle);

							pVolumetricCloudsRenderPassShader->SetUserUniformsBuffer(0, reinterpret_cast<const U8*>(&uniformsData), sizeof(uniformsData));
							pVolumetricCloudsRenderPassShader->Bind();

							pGraphicsContext->DispatchCompute(textureWidth / 16, textureHeight / 16, 1);

							pVolumetricCloudsRenderPassShader->Unbind();
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

						TFrameGraphTexture::TDesc volumetricCloudsFullSizeBufferParams{};

						volumetricCloudsFullSizeBufferParams.mWidth = mContext.mWindowWidth;
						volumetricCloudsFullSizeBufferParams.mHeight = mContext.mWindowHeight;
						volumetricCloudsFullSizeBufferParams.mFormat = FT_FLOAT4;
						volumetricCloudsFullSizeBufferParams.mNumOfMipLevels = 1;
						volumetricCloudsFullSizeBufferParams.mNumOfSamples = 1;
						volumetricCloudsFullSizeBufferParams.mSamplingQuality = 0;
						volumetricCloudsFullSizeBufferParams.mType = E_TEXTURE_IMPL_TYPE::TEXTURE_2D;
						volumetricCloudsFullSizeBufferParams.mUsageType = E_TEXTURE_IMPL_USAGE_TYPE::STATIC;
						volumetricCloudsFullSizeBufferParams.mBindFlags = E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE | E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS;
						volumetricCloudsFullSizeBufferParams.mName = "VolumetricCloudsFulLSizeTarget";
						volumetricCloudsFullSizeBufferParams.mFlags = E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT;

						data.mVolumetricCloudsFullSizeBufferHandle = builder.Create<TFrameGraphTexture>(volumetricCloudsFullSizeBufferParams.mName, volumetricCloudsFullSizeBufferParams);
						data.mVolumetricCloudsFullSizeBufferHandle = builder.Write(data.mVolumetricCloudsFullSizeBufferHandle);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("VolumetricCloudsBlurUpscalePass");
						TDE_RENDER_SECTION(pGraphicsContext, "VolumetricCloudsBlurUpscalePass");

						TFrameGraphTexture& depthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mDepthBufferHandle);
						TFrameGraphTexture& cloudsMainTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mVolumetricCloudsMainTargetHandle);
						TFrameGraphTexture& cloudsFullSizeTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mVolumetricCloudsFullSizeBufferHandle);

						const TTextureSamplerId linearSamplerHandle = pGraphicsContext->GetGraphicsObjectManager()->GetDefaultTextureSampler(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

						auto pVolumetricCloudsUpsampleBlurPassShader = pResourceManager->GetResource<IShader>(pResourceManager->Load<IShader>("Shaders/Default/Volumetrics/VolumetricCloudsBlur.cshader")); // \todo Add caching
						if (pVolumetricCloudsUpsampleBlurPassShader)
						{
							pGraphicsContext->SetTexture(pVolumetricCloudsUpsampleBlurPassShader->GetResourceBindingSlot("OutputTexture"), cloudsFullSizeTarget.mTextureHandle);
							pGraphicsContext->SetSampler(pVolumetricCloudsUpsampleBlurPassShader->GetResourceBindingSlot("OutputTexture"), linearSamplerHandle);

							pGraphicsContext->SetTexture(pVolumetricCloudsUpsampleBlurPassShader->GetResourceBindingSlot("DepthTexture"), depthBufferTarget.mTextureHandle);
							pGraphicsContext->SetSampler(pVolumetricCloudsUpsampleBlurPassShader->GetResourceBindingSlot("DepthTexture"), linearSamplerHandle);

							pGraphicsContext->SetTexture(pVolumetricCloudsUpsampleBlurPassShader->GetResourceBindingSlot("MainTexture"), cloudsMainTarget.mTextureHandle);
							pGraphicsContext->SetSampler(pVolumetricCloudsUpsampleBlurPassShader->GetResourceBindingSlot("MainTexture"), linearSamplerHandle);
							
							pVolumetricCloudsUpsampleBlurPassShader->Bind();

							pGraphicsContext->DispatchCompute(mContext.mWindowWidth / 16, mContext.mWindowHeight / 16, 1);

							pVolumetricCloudsUpsampleBlurPassShader->Unbind();
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
				mVolumetricCloudsComposeMaterialHandle = mContext.mpResourceManager->Create<IMaterial>("VolumetricCloudsCompose.material", TMaterialParameters{ "Shaders/Default/Volumetrics/VolumetricCloudsCompose.shader", false, TDepthStencilStateDesc { false, false } });
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
						data.mMainRenderTargetHandle = builder.Write(frameGraphBlackboard.mMainRenderTargetHandle);
					}, [=](const TPassData& data, const TFramePassExecutionContext& executionContext)
					{
						auto&& pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(executionContext.mpGraphicsContext);
						auto&& pResourceManager = mContext.mpResourceManager;

						TDE2_PROFILER_SCOPE("VolumetricCloudsBlurComposePass");
						TDE_RENDER_SECTION(pGraphicsContext, "VolumetricCloudsBlurComposePass");

						TFrameGraphTexture& depthBufferTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mDepthBufferHandle);
						TFrameGraphTexture& cloudsFullSizeTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(frameGraphBlackboard.mVolumetricCloudsFullSizeTargetHandle);
						TFrameGraphTexture& mainRenderTarget = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mMainRenderTargetHandle);

						pGraphicsContext->SetDepthBufferEnabled(false);
						pGraphicsContext->BindRenderTarget(0, mainRenderTarget.mTextureHandle);
						pGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(mContext.mWindowWidth), static_cast<F32>(mContext.mWindowHeight), 0.0f, 1.0f);

						/*if (auto pMaterial = pResourceManager->GetResource<IMaterial>(mVolumetricCloudsComposeMaterialHandle))
						{
							pMaterial->SetTextureResource(FrontFrameTextureUniformId, pSource.Get());
							pMaterial->SetTextureResource(BackFrameTextureUniformId, pExtraSource);
						}

						_submitFullScreenTriangle(mpPreUIRenderQueue, materialHandle, true);*/

						pGraphicsContext->SetDepthBufferEnabled(true);
						pGraphicsContext->BindRenderTarget(0, nullptr);
					});

				frameGraphBlackboard.mMainRenderTargetHandle = output.mMainRenderTargetHandle;
			}
		private:
			TResourceId mVolumetricCloudsComposeMaterialHandle = TResourceId::Invalid;
	};


	/*!
		interface IFramePostProcessor

		\brief The interface describes a functionality of frames post-processor
	*/

	class IFramePostProcessor : public virtual IBaseObject
	{
		public:
			typedef std::function<void()> TRenderFrameCallback;
		public:
			/*!
				\brief The method initializes an internal state of the processor

				/param[in] desc A set of parameters that're needed to initialize the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const TFramePostProcessorParameters& desc) = 0;

			/*!
				\brief The method assigns a processing profile into the processor

				\param[in] pProfileResource A pointer to IPostProcessingProfile implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetProcessingProfile(const IPostProcessingProfile* pProfileResource) = 0;

			/*!
				\brief The method renders all the geometry into an off-screen target that later will be used as post-processing
				source. It could be used multiple times during single frame to accumulate new draw calls on top previous ones

				\param[in] onRenderFrameCallback A callback in which all the geometry of the scene should be rendered

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Render(const TRenderFrameCallback& onRenderFrameCallback, E_FRAME_RENDER_PARAMS_FLAGS flags) = 0;

			/*!
				\brief The method prepares render targets and materials that're involved into post-processings

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE PreRender() = 0;

			/*!
				\brief The actual post-processing stages (bloom, AA) are run through this call

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE RunPostProcess() = 0;

			/*!
				\brief The method prepares last draw call with full-screen quad with tone mapping material
				to output result onto the screen

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE PostRender() = 0;

			TDE2_API virtual E_RESULT_CODE RunVolumetricCloudsPass() = 0;

			TDE2_API virtual TResourceId GetMainDepthBufferHandle() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IFramePostProcessor)
	};


	TDE2_DECLARE_SCOPED_PTR(IFramePostProcessor)
	TDE2_DEFINE_SCOPED_PTR(IFramePostProcessor)


	/*!
		\brief CFramePostProcessor's definition
	*/

	/*!
		\brief A factory function for creation objects of CFramePostProcessor's type

		/param[in] desc A set of parameters that're needed to initialize the object
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFramePostProcessor's implementation
	*/

	static IFramePostProcessor* CreateFramePostProcessor(const TFramePostProcessorParameters& desc, E_RESULT_CODE& result);


	TDE2_STATIC_CONSTEXPR U32 LuminanceTargetSizes = 1024;


	/*!
		class CFramePostProcessor

		\brief The class is a basic implementation of default in-engine frame post-processor
	*/

	class CFramePostProcessor : public CBaseObject, public IFramePostProcessor
	{
		public:
			friend IFramePostProcessor* CreateFramePostProcessor(const TFramePostProcessorParameters&, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of the processor

				/param[in] desc A set of parameters that're needed to initialize the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(const TFramePostProcessorParameters& desc) override;

			/*!
				\brief The method assigns a processing profile into the processor

				\param[in] pProfileResource A pointer to IPostProcessingProfile implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetProcessingProfile(const IPostProcessingProfile* pProfileResource) override;

			/*!
				\brief he method renders all the geometry into an off-screen target that later will be used as post-processing
				source

				\param[in] onRenderFrameCallback A callback in which all the geometry of the scene should be rendered

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Render(const TRenderFrameCallback& onRenderFrameCallback, E_FRAME_RENDER_PARAMS_FLAGS flags) override;

			TDE2_API E_RESULT_CODE PreRender() override;
			TDE2_API E_RESULT_CODE RunPostProcess() override;
			TDE2_API E_RESULT_CODE PostRender() override;

			TDE2_API E_RESULT_CODE RunVolumetricCloudsPass() override;

			TDE2_API TResourceId GetMainDepthBufferHandle() const override { return mMainDepthBufferHandle; }
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFramePostProcessor)

			TDE2_API void _submitFullScreenTriangle(CRenderQueue* pRenderQueue, TResourceId materialHandle, bool drawImmediately = false);
			TDE2_API void _renderTargetToTarget(TPtr<IRenderTarget> pSource, ITexture* pExtraSource, TPtr<IRenderTarget> pDest, TResourceId materialHandle);

			TDE2_API void _prepareRenderTargetsChain(U32 width, U32 height, bool isHDRSupport = false);
			TDE2_API void _resizeRenderTargetsChain(U32 width, U32 height);

			TDE2_API void _processBloomPass(TPtr<IRenderTarget> pFrontTarget, TPtr<IRenderTarget> pBackTarget, TPtr<IRenderTarget> pBloomTarget);
		protected:
			CRenderQueue*                 mpPreUIRenderQueue;
			CRenderQueue*                 mpOverlayRenderQueue;

			const IPostProcessingProfile* mpCurrPostProcessingProfile;

			TResourceId                   mDefaultScreenSpaceMaterialHandle;
			TResourceId                   mBloomFilterMaterialHandle;
			TResourceId                   mBloomFinalPassMaterialHandle;
			TResourceId                   mGaussianBlurMaterialHandle;
			TResourceId                   mToneMappingPassMaterialHandle;
			TResourceId                   mGenerateLuminanceMaterialHandle;
			TResourceId                   mLuminanceAdaptationMaterialHandle;
			TResourceId                   mBuildLightsHeatMapMaterialHandle;

			TResourceId                   mVolumetricCloudsComputeShaderHandle;
			TResourceId                   mVolumetricCloudsUpsampleBlurShaderHandle;
			TResourceId                   mVolumetricCloudsComposeMaterialHandle;

			TResourceId                   mRenderTargetHandle;
			TResourceId                   mUITargetHandle;
			TResourceId                   mLightsHeatmapTargetHandle;
			TResourceId                   mBloomRenderTargetHandle;
			TResourceId                   mTemporaryRenderTargetHandle;
			TResourceId                   mMainDepthBufferHandle;
			TResourceId                   mLuminanceTargetHandle;
			TResourceId                   mVolumetricCloudsScreenBufferHandle;
			TResourceId                   mVolumetricCloudsFullResScreenBufferHandle;

			std::array<TResourceId, 2>    mFramesLuminanceHistoryTargets;
			USIZE                         mCurrActiveLuminanceFrameTargetIndex = 0;

			IVertexDeclaration*           mpVertexFormatDeclaration;

			TPtr<IResourceManager>        mpResourceManager;

			IWindowSystem*                mpWindowSystem;

			IGraphicsContext*             mpGraphicsContext;

			TBufferHandleId               mFullScreenTriangleVertexBufferHandle; // \note This buffer is only needed when we work with GL GAPI, D3D11 uses bufferless rendering

			IGlobalShaderProperties*      mpGlobalShaderProperties;
	};


	static TResult<TResourceId> GetOrCreateLuminanceTarget(TPtr<IResourceManager> pResourceManager, const std::string& name, U32 sizes)
	{
		TRenderTargetParameters luminanceTargetParams;
		luminanceTargetParams.mWidth = sizes;
		luminanceTargetParams.mHeight = sizes;
		luminanceTargetParams.mFormat = FT_FLOAT1;
		luminanceTargetParams.mNumOfMipLevels = static_cast<U32>(log2(sizes)) + 1;
		luminanceTargetParams.mNumOfSamples = 1;
		luminanceTargetParams.mSamplingQuality = 0;
		luminanceTargetParams.mType = TRenderTargetParameters::E_TARGET_TYPE::TEXTURE2D;

		const TResourceId targetHandle = pResourceManager->Create<IRenderTarget>(name, luminanceTargetParams);
		if (TResourceId::Invalid == targetHandle)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (auto pTarget = pResourceManager->GetResource<IRenderTarget>(targetHandle))
		{
			pTarget->SetUWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pTarget->SetVWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);

			pTarget->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);
		}

		return Wrench::TOkValue<TResourceId>(targetHandle);
	}


	static TResourceId GetRenderTarget(TPtr<IResourceManager> pResourceManager, U32 width, U32 height, bool isHDRSupport, E_FRAME_RENDER_PARAMS_FLAGS flags)
	{
		static U32 counter = 0;

		std::string renderTargetName;

		if (E_FRAME_RENDER_PARAMS_FLAGS::RENDER_MAIN == (E_FRAME_RENDER_PARAMS_FLAGS::RENDER_MAIN & flags))
		{
			renderTargetName = "MainRenderTarget";
		}
		else if (E_FRAME_RENDER_PARAMS_FLAGS::RENDER_UI == (E_FRAME_RENDER_PARAMS_FLAGS::RENDER_UI & flags))
		{
			renderTargetName = "UITarget";
		}
		else
		{
			renderTargetName = "SecondaryRenderTarget" + std::to_string(counter++);
		}

		return pResourceManager->Create<IRenderTarget>(renderTargetName, TTexture2DParameters{ width, height, isHDRSupport ? FT_FLOAT4 : FT_NORM_UBYTE4, 1, 1, 0 });
	}


	CFramePostProcessor::CFramePostProcessor() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CFramePostProcessor::Init(const TFramePostProcessorParameters& desc)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!desc.mpRenderer || !desc.mpGraphicsObjectManager || !desc.mpWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = desc.mpRenderer->GetResourceManager();
		mpGlobalShaderProperties = desc.mpRenderer->GetGlobalShaderProperties().Get();

		mpPreUIRenderQueue = desc.mpRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_SPRITES);
		mpOverlayRenderQueue = desc.mpRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_OVERLAY);
		mpWindowSystem = desc.mpWindowSystem;
		mpGraphicsContext = desc.mpGraphicsObjectManager->GetGraphicsContext();
		mRenderTargetHandle = TResourceId::Invalid;
		mUITargetHandle = TResourceId::Invalid;
		mLightsHeatmapTargetHandle = TResourceId::Invalid;

		auto luminanceTargetResult = GetOrCreateLuminanceTarget(mpResourceManager, "LuminanceTarget", LuminanceTargetSizes);
		if (luminanceTargetResult.HasError())
		{
			return luminanceTargetResult.GetError();
		}

		mLuminanceTargetHandle = luminanceTargetResult.Get();

		std::array<TResult<TResourceId>, 2> frameLuminanceBuffers
		{
			GetOrCreateLuminanceTarget(mpResourceManager, "LuminancePrevFrame", 1),
			GetOrCreateLuminanceTarget(mpResourceManager, "LuminanceCurrFrame", 1)
		};

		std::transform(frameLuminanceBuffers.begin(), frameLuminanceBuffers.end(), mFramesLuminanceHistoryTargets.begin(), [](const TResult<TResourceId>& result)
		{
			TDE2_ASSERT(result.IsOk());
			return result.Get();
		});

		// Used materials 
		// \todo Refactor this later
		mDefaultScreenSpaceMaterialHandle = mpResourceManager->Create<IMaterial>("DefaultScreenSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/BlitTexture.shader", false, TDepthStencilStateDesc { false, false } });
		mBloomFilterMaterialHandle = mpResourceManager->Create<IMaterial>("BloomScreenSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/Bloom.shader", false, TDepthStencilStateDesc { false, false } });
		mBloomFinalPassMaterialHandle = mpResourceManager->Create<IMaterial>("BloomFinalPassSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/BloomFinal.shader", false, TDepthStencilStateDesc { false, false } });
		mGaussianBlurMaterialHandle = mpResourceManager->Create<IMaterial>("GaussianBlurSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/GaussianBlur.shader", false, TDepthStencilStateDesc { false, false } });
		mToneMappingPassMaterialHandle = mpResourceManager->Create<IMaterial>("ToneMappingPass.material", TMaterialParameters{ "Shaders/PostEffects/ToneMapping.shader", false, TDepthStencilStateDesc { false, false } });
		mGenerateLuminanceMaterialHandle = mpResourceManager->Create<IMaterial>("GenerateLuminance.material", TMaterialParameters{ "Shaders/PostEffects/GenerateLuminance.shader", false, TDepthStencilStateDesc { false, false } });
		mLuminanceAdaptationMaterialHandle = mpResourceManager->Create<IMaterial>("AdaptLuminance.material", TMaterialParameters{ "Shaders/PostEffects/AdaptLuminance.shader", false, TDepthStencilStateDesc { false, false } });
		mBuildLightsHeatMapMaterialHandle = mpResourceManager->Create<IMaterial>("BuildLightsHeatmap.material", TMaterialParameters{ "Shaders/PostEffects/LightsHeatmap.shader", false, TDepthStencilStateDesc { false, false } });

		mVolumetricCloudsComputeShaderHandle = mpResourceManager->Load<IShader>("Shaders/Default/Volumetrics/VolumetricClouds.cshader");
		mVolumetricCloudsUpsampleBlurShaderHandle = mpResourceManager->Load<IShader>("Shaders/Default/Volumetrics/VolumetricCloudsBlur.cshader");
		mVolumetricCloudsComposeMaterialHandle = mpResourceManager->Create<IMaterial>("VolumetricCloudsCompose.material", TMaterialParameters{ "Shaders/Default/Volumetrics/VolumetricCloudsCompose.shader", false, TDepthStencilStateDesc { false, false } });

		if (auto vertexFormatResult = desc.mpGraphicsObjectManager->CreateVertexDeclaration())
		{
			mpVertexFormatDeclaration = vertexFormatResult.Get();
		}

		TDE2_ASSERT(mpVertexFormatDeclaration);
		mpVertexFormatDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });

		const void* pScreenTriangleVerts = static_cast<const void*>(&desc.mpGraphicsObjectManager->GetScreenTriangleVertices()[0]);

		if (auto vertexBufferResult = desc.mpGraphicsObjectManager->CreateBuffer(
			{
				E_BUFFER_USAGE_TYPE::STATIC,
				E_BUFFER_TYPE::VERTEX,
				3 * sizeof(TVector4),
				pScreenTriangleVerts
			}))
		{
			mFullScreenTriangleVertexBufferHandle = vertexBufferResult.Get();
		}

		TDE2_ASSERT(TBufferHandleId::Invalid != mFullScreenTriangleVertexBufferHandle);

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CFramePostProcessor::SetProcessingProfile(const IPostProcessingProfile* pProfileResource)
	{
		if (!pProfileResource)
		{
			return RC_INVALID_ARGS;
		}

		mpCurrPostProcessingProfile = pProfileResource;

		return RC_OK;
	}


	static const std::string FrontFrameTextureUniformId = "FrameTexture";
	static const std::string BackFrameTextureUniformId = "FrameTexture1";


	E_RESULT_CODE CFramePostProcessor::Render(const TRenderFrameCallback& onRenderFrameCallback, E_FRAME_RENDER_PARAMS_FLAGS flags)
	{
		if (!onRenderFrameCallback)
		{
			LOG_WARNING("[FramePostProcessor] Render method was got empty \"onRenderFrameCallback\" argument");
			return RC_INVALID_ARGS;
		}

		const bool bindDepthBuffer = E_FRAME_RENDER_PARAMS_FLAGS::BIND_DEPTH_BUFFER == (flags & E_FRAME_RENDER_PARAMS_FLAGS::BIND_DEPTH_BUFFER);
		const bool clearRT = E_FRAME_RENDER_PARAMS_FLAGS::CLEAR_RENDER_TARGET == (flags & E_FRAME_RENDER_PARAMS_FLAGS::CLEAR_RENDER_TARGET);
		const bool isMainTarget = E_FRAME_RENDER_PARAMS_FLAGS::RENDER_MAIN == (flags & E_FRAME_RENDER_PARAMS_FLAGS::RENDER_MAIN);


		TPtr<IRenderTarget> pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(isMainTarget ? mRenderTargetHandle : mUITargetHandle);
		TPtr<IDepthBufferTarget> pMainDepthBuffer = mpResourceManager->GetResource<IDepthBufferTarget>(mMainDepthBufferHandle);

		if (bindDepthBuffer && !isMainTarget) // depth only pass
		{
			mpGraphicsContext->BindDepthBufferTarget(pMainDepthBuffer.Get(), true);
			mpGraphicsContext->ClearDepthBuffer(1.0f);

			onRenderFrameCallback();

			mpGraphicsContext->BindDepthBufferTarget(nullptr);

			return RC_OK;
		}

		{
			mpGraphicsContext->BindRenderTarget(0, pCurrRenderTarget.Get());
			if (bindDepthBuffer)
			{
				mpGraphicsContext->BindDepthBufferTarget(pMainDepthBuffer.Get());
			}

			if (clearRT)
			{
				mpGraphicsContext->ClearRenderTarget(pCurrRenderTarget.Get(), TColor32F{});
			}

			onRenderFrameCallback();

			if (bindDepthBuffer)
			{
				mpGraphicsContext->BindDepthBufferTarget(nullptr);
			}

			mpGraphicsContext->BindRenderTarget(0, nullptr);
		}

		return RC_OK;
	}

	E_RESULT_CODE CFramePostProcessor::PreRender()
	{
		const U32 width = mpWindowSystem->GetWidth();
		const U32 height = mpWindowSystem->GetHeight();

		const auto& toneMappingParameters = mpCurrPostProcessingProfile->GetToneMappingParameters();
		const auto& colorGradingParameters = mpCurrPostProcessingProfile->GetColorGradingParameters();

		const bool isHDREnabled = toneMappingParameters.mIsEnabled;

		if (auto pToneMappingMaterial = mpResourceManager->GetResource<IMaterial>(mToneMappingPassMaterialHandle))
		{
			pToneMappingMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "toneMappingParams", TVector4(isHDREnabled ? 1.0f : 0.0f, toneMappingParameters.mExposure, toneMappingParameters.mKeyValue, 0.0f));
			pToneMappingMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "colorGradingParams", TVector4(colorGradingParameters.mIsEnabled ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f));
		}

		_prepareRenderTargetsChain(width, height, isHDREnabled);

		return RC_OK;
	}

	E_RESULT_CODE CFramePostProcessor::RunPostProcess()
	{
		TPtr<IRenderTarget> pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle);
		TPtr<IRenderTarget> pBloomRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mBloomRenderTargetHandle);
		TPtr<IRenderTarget> pTempRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mTemporaryRenderTargetHandle);

		if (EnableLightsHeatMapCfgVar.Get())
		{ // Light heatmap rendering
#if TDE2_EDITORS_ENABLED
			auto pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(mpGraphicsContext);
			TDE_RENDER_SECTION(pGraphicsContext, "LightHeatmapRendering");
#endif
			TPtr<IRenderTarget> pLightsHeatmapTarget = mpResourceManager->GetResource<IRenderTarget>(mLightsHeatmapTargetHandle);
			_renderTargetToTarget(pCurrRenderTarget, nullptr, pLightsHeatmapTarget, mBuildLightsHeatMapMaterialHandle);
			_renderTargetToTarget(pLightsHeatmapTarget, nullptr, pCurrRenderTarget, mDefaultScreenSpaceMaterialHandle);
		}

		{ // Luminance calculations
#if TDE2_EDITORS_ENABLED
			auto pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(mpGraphicsContext);
			TDE_RENDER_SECTION(pGraphicsContext, "EyeAdaptation");
#endif

			TPtr<IRenderTarget> pLuminanceTarget = mpResourceManager->GetResource<IRenderTarget>(mLuminanceTargetHandle);

			_renderTargetToTarget(pCurrRenderTarget, nullptr, pLuminanceTarget, mGenerateLuminanceMaterialHandle); // Extract luminance
			pLuminanceTarget->GenerateMipMaps();

			// Eye adaptation 
			TPtr<IRenderTarget> pLuminanceAdaptPrevTarget = mpResourceManager->GetResource<IRenderTarget>(mFramesLuminanceHistoryTargets[(mCurrActiveLuminanceFrameTargetIndex + 1) & 0x1]);
			TPtr<IRenderTarget> pLuminanceAdaptCurrTarget = mpResourceManager->GetResource<IRenderTarget>(mFramesLuminanceHistoryTargets[mCurrActiveLuminanceFrameTargetIndex]);

			if (auto pMaterial = mpResourceManager->GetResource<IMaterial>(mLuminanceAdaptationMaterialHandle))
			{
				const auto& toneMappingParameters = mpCurrPostProcessingProfile->GetToneMappingParameters();

				pMaterial->SetVariableForInstance<F32>(DefaultMaterialInstanceId, "mAdaptationRate", toneMappingParameters.mEyeAdaptionCoeff);
			}

			_renderTargetToTarget(pLuminanceTarget, pLuminanceAdaptPrevTarget.Get(), pLuminanceAdaptCurrTarget, mLuminanceAdaptationMaterialHandle);
		}

		_processBloomPass(pCurrRenderTarget, pTempRenderTarget, pBloomRenderTarget);

		mpPreUIRenderQueue->Clear(); // commands above are executed immediately, so we don't need to store them anymore

		return RC_OK;
	}

	E_RESULT_CODE CFramePostProcessor::PostRender()
	{
		_submitFullScreenTriangle(mpOverlayRenderQueue, mToneMappingPassMaterialHandle);

		mCurrActiveLuminanceFrameTargetIndex = (mCurrActiveLuminanceFrameTargetIndex + 1) & 0x1;

		return RC_OK;
	}


	E_RESULT_CODE CFramePostProcessor::RunVolumetricCloudsPass()
	{
		TDE2_PROFILER_SCOPE("CFramePostProcessor::RunVolumetricCloudsPass");

		if (!CGameUserSettings::Get()->mpIsVolumetricCloudsEnabledCVar->Get())
		{
			return RC_OK;
		}

		auto pVolumetricCloudsScreenBufferTexture = mpResourceManager->GetResource<ITexture2D>(mVolumetricCloudsScreenBufferHandle);
		if (!pVolumetricCloudsScreenBufferTexture)
		{
			return RC_FAIL;
		}

		auto pDepthBufferResource = mpResourceManager->GetResource<IDepthBufferTarget>(mMainDepthBufferHandle);
		auto pMainRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle);

		// Main pass
		{
			struct
			{
				TVector2 mInvTextureSizes;
				I32      mStepsCount;
			} uniformsData;

			uniformsData.mInvTextureSizes = TVector2
			{
				1 / static_cast<F32>(pVolumetricCloudsScreenBufferTexture->GetWidth()), 1 / static_cast<F32>(pVolumetricCloudsScreenBufferTexture->GetHeight())
			};
			uniformsData.mStepsCount = 64;

			auto pVolumetricCloudsRenderPassShader = mpResourceManager->GetResource<IShader>(mVolumetricCloudsComputeShaderHandle);
			pVolumetricCloudsRenderPassShader->SetTextureResource("OutputTexture", pVolumetricCloudsScreenBufferTexture.Get());
			pVolumetricCloudsRenderPassShader->SetTextureResource("DepthTexture", pDepthBufferResource.Get());
			pVolumetricCloudsRenderPassShader->SetTextureResource("MainTexture", pMainRenderTarget.Get());

			pVolumetricCloudsRenderPassShader->SetUserUniformsBuffer(0, reinterpret_cast<const U8*>(&uniformsData), sizeof(uniformsData));
			pVolumetricCloudsRenderPassShader->Bind();

			mpGraphicsContext->DispatchCompute(pVolumetricCloudsScreenBufferTexture->GetWidth() / 16, pVolumetricCloudsScreenBufferTexture->GetHeight() / 16, 1);

			pVolumetricCloudsRenderPassShader->Unbind();
		}

		auto pVolumetricCloudsFullSizeBufferTexture = mpResourceManager->GetResource<ITexture2D>(mVolumetricCloudsFullResScreenBufferHandle);

		// Blur + Upsample
		{
			auto pVolumetricCloudsUpsampleBlurPassShader = mpResourceManager->GetResource<IShader>(mVolumetricCloudsUpsampleBlurShaderHandle);
			pVolumetricCloudsUpsampleBlurPassShader->SetTextureResource("OutputTexture", pVolumetricCloudsFullSizeBufferTexture.Get());
			pVolumetricCloudsUpsampleBlurPassShader->SetTextureResource("DepthTexture", pDepthBufferResource.Get());
			pVolumetricCloudsUpsampleBlurPassShader->SetTextureResource("MainTexture", pVolumetricCloudsScreenBufferTexture.Get());

			pVolumetricCloudsUpsampleBlurPassShader->Bind();

			mpGraphicsContext->DispatchCompute(pVolumetricCloudsFullSizeBufferTexture->GetWidth() / 16, pVolumetricCloudsFullSizeBufferTexture->GetHeight() / 16, 1);

			pVolumetricCloudsUpsampleBlurPassShader->Unbind();
		}

		// Compose pass
		_renderTargetToTarget(
			DynamicPtrCast<IRenderTarget>(pDepthBufferResource),
			pVolumetricCloudsFullSizeBufferTexture.Get(),
			pMainRenderTarget,
			mVolumetricCloudsComposeMaterialHandle); // Compose

		return RC_OK;
	}


	static TResult<TResourceId> GetOrCreateVolumetricCloudsBuffer(TPtr<IResourceManager> pResourceManager, U32 width, U32 height, const std::string& suffix = "Main")
	{
		TTexture2DParameters textureParams
		{
			width,
			height,
			FT_FLOAT4, 1, 1, 0
		};
		textureParams.mIsWriteable = true;

		const TResourceId textureHandle = pResourceManager->Create<ITexture2D>("VolumetricCloudsScreenBuffer_" + suffix, textureParams);
		if (auto pScreenBufferTexture = pResourceManager->GetResource<ITexture2D>(textureHandle))
		{
			pScreenBufferTexture->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

			if (pScreenBufferTexture->GetWidth() != width || pScreenBufferTexture->GetHeight() != height)
			{
				LOG_MESSAGE(Wrench::StringUtils::Format("[CFramePostProcessor] The sizes has of volumetric clouds buffer been changed", width, height));
				pScreenBufferTexture->Resize(width, height);
			}
		}

		if (TResourceId::Invalid == textureHandle)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<TResourceId>(textureHandle);
	}


	static TResult<TResourceId> GetOrCreateDepthBuffer(TPtr<IResourceManager> pResourceManager, U32 width, U32 height)
	{
		TRenderTargetParameters depthBufferParams;
		depthBufferParams.mWidth = width;
		depthBufferParams.mHeight = height;
		depthBufferParams.mFormat = FT_D32;
		depthBufferParams.mNumOfMipLevels = 1;
		depthBufferParams.mNumOfSamples = 1;
		depthBufferParams.mSamplingQuality = 0;
		depthBufferParams.mType = TRenderTargetParameters::E_TARGET_TYPE::TEXTURE2D;

		const TResourceId depthBufferHandle = pResourceManager->Create<IDepthBufferTarget>("MainDepthBuffer", depthBufferParams);
		if (TResourceId::Invalid == depthBufferHandle)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (auto pDepthBufferTexture = pResourceManager->GetResource<IDepthBufferTarget>(depthBufferHandle))
		{
			pDepthBufferTexture->SetUWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pDepthBufferTexture->SetVWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pDepthBufferTexture->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

			if (pDepthBufferTexture->GetWidth() != width || pDepthBufferTexture->GetHeight() != height)
			{
				LOG_MESSAGE("[CFramePostProcessor] The sizes has been changed");
				pDepthBufferTexture->Resize(width, height);
			}
		}

		return Wrench::TOkValue<TResourceId>(depthBufferHandle);
	}


	void CFramePostProcessor::_prepareRenderTargetsChain(U32 width, U32 height, bool isHDRSupport)
	{
		auto createVolumetricCloudsBufferResult = GetOrCreateVolumetricCloudsBuffer(mpResourceManager, width / 4, height / 4);
		if (createVolumetricCloudsBufferResult.IsOk())
		{
			mVolumetricCloudsScreenBufferHandle = createVolumetricCloudsBufferResult.Get();
		}

		auto createVolumetricCloudsFullResBufferResult = GetOrCreateVolumetricCloudsBuffer(mpResourceManager, width, height, "FullRes");
		if (createVolumetricCloudsFullResBufferResult.IsOk())
		{
			mVolumetricCloudsFullResScreenBufferHandle = createVolumetricCloudsFullResBufferResult.Get();
		}

		auto mainDepthBufferRetrieveResult = GetOrCreateDepthBuffer(mpResourceManager, width, height);
		if (mainDepthBufferRetrieveResult.IsOk())
		{
			mMainDepthBufferHandle = mainDepthBufferRetrieveResult.Get();
		}

		if (TResourceId::Invalid != mRenderTargetHandle)
		{
			if (auto pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle))
			{
				if (pCurrRenderTarget && (pCurrRenderTarget->GetWidth() != width || pCurrRenderTarget->GetHeight() != height))
				{
					_resizeRenderTargetsChain(width, height);
				}
			}

			return;
		}

		mRenderTargetHandle = GetRenderTarget(mpResourceManager, width, height, isHDRSupport, E_FRAME_RENDER_PARAMS_FLAGS::RENDER_MAIN);
		mTemporaryRenderTargetHandle = GetRenderTarget(mpResourceManager, width, height, isHDRSupport, E_FRAME_RENDER_PARAMS_FLAGS::NONE);
		mUITargetHandle = GetRenderTarget(mpResourceManager, width, height, false, E_FRAME_RENDER_PARAMS_FLAGS::RENDER_UI);
		mLightsHeatmapTargetHandle = GetRenderTarget(mpResourceManager, width, height, false, E_FRAME_RENDER_PARAMS_FLAGS::NONE);

		auto pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle);
		auto pUIRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mUITargetHandle);
		auto pTempRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mTemporaryRenderTargetHandle);
		auto pLuminanceTarget = mpResourceManager->GetResource<IRenderTarget>(mFramesLuminanceHistoryTargets[mCurrActiveLuminanceFrameTargetIndex]);

		pCurrRenderTarget->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);
		pTempRenderTarget->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

		const bool isPostProcessingEnabled = mpCurrPostProcessingProfile->IsPostProcessingEnabled();

		if (auto pToneMappingMaterial = mpResourceManager->GetResource<IMaterial>(mToneMappingPassMaterialHandle))
		{
			pToneMappingMaterial->SetTextureResource(FrontFrameTextureUniformId, pCurrRenderTarget.Get());

			/// \note If the color grading's pass was enabled assign its look up texture (assume that it has linear sampling)
			{
				const auto& colorGradingParameters = mpCurrPostProcessingProfile->GetColorGradingParameters();

				if (isPostProcessingEnabled/* && colorGradingParameters.mIsEnabled*/)
				{
					if (auto pColorLUT = mpResourceManager->GetResource<ITexture>(mpResourceManager->Load<ITexture2D>(colorGradingParameters.mLookUpTextureId)))
					{
						pToneMappingMaterial->SetTextureResource("ColorGradingLUT", pColorLUT.Get());
					}
				}

				pToneMappingMaterial->SetTextureResource("LuminanceBuffer", pLuminanceTarget.Get());
				pToneMappingMaterial->SetTextureResource("UIBuffer", pUIRenderTarget.Get());
			}
		}

		if (isPostProcessingEnabled && mpCurrPostProcessingProfile->GetBloomParameters().mIsEnabled)
		{
			const U16 downsampleCoeff = 1 << (TPostProcessingProfileParameters::TBloomParameters::mMaxQuality - mpCurrPostProcessingProfile->GetBloomParameters().mQuality + 1);

			mBloomRenderTargetHandle = GetRenderTarget(mpResourceManager, width / downsampleCoeff, height / downsampleCoeff, isHDRSupport, E_FRAME_RENDER_PARAMS_FLAGS::NONE);

			if (auto pBloomRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mBloomRenderTargetHandle))
			{
				pBloomRenderTarget->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);
			}
		}
	}

	void CFramePostProcessor::_resizeRenderTargetsChain(U32 width, U32 height)
	{
		const std::array<TResourceId, 5> renderTargetHandles
		{
			mRenderTargetHandle,
			mUITargetHandle,
			mBloomRenderTargetHandle,
			mTemporaryRenderTargetHandle,
			mLightsHeatmapTargetHandle
		};

		for (const TResourceId& currTargetHandle : renderTargetHandles)
		{
			if (auto pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(currTargetHandle))
			{
				pCurrRenderTarget->Resize(width, height);
			}
		}
	}

	void CFramePostProcessor::_processBloomPass(TPtr<IRenderTarget> pFrontTarget, TPtr<IRenderTarget> pBackTarget, TPtr<IRenderTarget> pBloomTarget)
	{
		const auto& bloomParameters = mpCurrPostProcessingProfile->GetBloomParameters();
		const auto& toneMappingParameters = mpCurrPostProcessingProfile->GetToneMappingParameters();

		if (!mpCurrPostProcessingProfile->IsPostProcessingEnabled() || !bloomParameters.mIsEnabled)
		{
			return;
		}

#if TDE2_EDITORS_ENABLED
		auto pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(mpGraphicsContext);
		TDE_RENDER_SECTION(pGraphicsContext, "Bloom");
#endif

		if (auto pBloomMaterial = mpResourceManager->GetResource<IMaterial>(mBloomFilterMaterialHandle))
		{
			pBloomMaterial->SetVariableForInstance<F32>(DefaultMaterialInstanceId, "threshold", bloomParameters.mThreshold);
			pBloomMaterial->SetVariableForInstance<F32>(DefaultMaterialInstanceId, "keyValue", toneMappingParameters.mKeyValue);
		}

		const TVector4 blurParams{ bloomParameters.mSmoothness, 0.0, 1.0f / static_cast<F32>(pFrontTarget->GetWidth()), 1.0f / static_cast<F32>(pFrontTarget->GetHeight()) };
		const TVector4 vertBlurRotation{ 0.0f, CMathConstants::Pi * 0.5f, 0.0f, 0.0f };

		if (auto pBlurMaterial = mpResourceManager->GetResource<IMaterial>(mGaussianBlurMaterialHandle))
		{
			pBlurMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "blurParams", blurParams);
			pBlurMaterial->SetVariableForInstance<U32>(DefaultMaterialInstanceId, "samplesCount", bloomParameters.mSamplesCount);
		}

		auto pLuminanceTarget = mpResourceManager->GetResource<IRenderTarget>(mFramesLuminanceHistoryTargets[mCurrActiveLuminanceFrameTargetIndex]);

		_renderTargetToTarget(pFrontTarget, pLuminanceTarget.Get(), pBloomTarget, mBloomFilterMaterialHandle); // Bloom pass
		_renderTargetToTarget(pBloomTarget, nullptr, pBackTarget, mGaussianBlurMaterialHandle); // Horizontal Blur pass

		if (auto pBlurMaterial = mpResourceManager->GetResource<IMaterial>(mGaussianBlurMaterialHandle))
		{
			pBlurMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "blurParams", blurParams + vertBlurRotation);
		}

		_renderTargetToTarget(pBackTarget, nullptr, pBloomTarget, mGaussianBlurMaterialHandle); // Vertical Blur pass
		_renderTargetToTarget(pFrontTarget, pBloomTarget.Get(), pBackTarget, mBloomFinalPassMaterialHandle); // Compose
		_renderTargetToTarget(pBackTarget, nullptr, pFrontTarget, mDefaultScreenSpaceMaterialHandle); // Blit Temp -> Main render target
	}

	void CFramePostProcessor::_submitFullScreenTriangle(CRenderQueue* pRenderQueue, TResourceId materialHandle, bool drawImmediately)
	{
		// \note The fullscreen triangle is drawn without vertex buffer, all the geometry is generated inside the vertex shader
		TDrawCommandPtr pDrawCommand = pRenderQueue->SubmitDrawCommand<TDrawCommand>(static_cast<U32>(E_GEOMETRY_SUBGROUP_TAGS::IMAGE_EFFECTS));
		pDrawCommand->mNumOfVertices = 3;
		pDrawCommand->mPrimitiveType = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
		pDrawCommand->mMaterialHandle = materialHandle;
		pDrawCommand->mVertexBufferHandle = mFullScreenTriangleVertexBufferHandle;
		pDrawCommand->mpVertexDeclaration = mpVertexFormatDeclaration;

		if (drawImmediately)
		{
			pDrawCommand->Submit({ mpGraphicsContext, mpResourceManager.Get(), mpGlobalShaderProperties });
		}
	}

	void CFramePostProcessor::_renderTargetToTarget(TPtr<IRenderTarget> pSource, ITexture* pExtraSource, TPtr<IRenderTarget> pDest, TResourceId materialHandle)
	{
		mpGraphicsContext->SetDepthBufferEnabled(false);
		mpGraphicsContext->BindRenderTarget(0, pDest.Get());
		mpGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(pDest->GetWidth()), static_cast<F32>(pDest->GetHeight()), 0.0f, 1.0f);

		if (auto pMaterial = mpResourceManager->GetResource<IMaterial>(materialHandle))
		{
			pMaterial->SetTextureResource(FrontFrameTextureUniformId, pSource.Get());

			if (pExtraSource)
			{
				pMaterial->SetTextureResource(BackFrameTextureUniformId, pExtraSource);
			}
		}

		_submitFullScreenTriangle(mpPreUIRenderQueue, materialHandle, true);

		mpGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(mpWindowSystem->GetWidth()), static_cast<F32>(mpWindowSystem->GetHeight()), 0.0f, 1.0f);
		mpGraphicsContext->SetDepthBufferEnabled(true);
		mpGraphicsContext->BindRenderTarget(0, nullptr);
	}


	static IFramePostProcessor* CreateFramePostProcessor(const TFramePostProcessorParameters& desc, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IFramePostProcessor, CFramePostProcessor, result, desc);
	}


	/*!
		\brief CForwardRenderer's definition
	*/

	CForwardRenderer::CForwardRenderer():
		CBaseObject(), mpMainCamera(nullptr), mpResourceManager(nullptr), mpGlobalShaderProperties(nullptr), mpFramePostProcessor(nullptr)
	{
	}


	static TResult<TResourceId> GetOrCreateDirectionalShadowMap(TPtr<IResourceManager> pResourceManager)
	{
		const U32 shadowMapSizes = static_cast<U32>(CGameUserSettings::Get()->mpShadowMapSizesCVar->Get());
		TDE2_ASSERT(shadowMapSizes > 0 && shadowMapSizes < 65536);

		TRenderTargetParameters shadowMapParams;
		shadowMapParams.mWidth = shadowMapSizes;
		shadowMapParams.mHeight = shadowMapSizes;
		shadowMapParams.mFormat = FT_D32;
		shadowMapParams.mNumOfMipLevels = 1;
		shadowMapParams.mNumOfSamples = 1;
		shadowMapParams.mSamplingQuality = 0;
		shadowMapParams.mType = TRenderTargetParameters::E_TARGET_TYPE::TEXTURE2D_ARRAY;
		shadowMapParams.mArraySize = CGameUserSettings::Get()->mpShadowCascadesCountCVar->Get();

		const TResourceId shadowMapHandle = pResourceManager->Create<IDepthBufferTarget>("ShadowMap", shadowMapParams);
		if (shadowMapHandle == TResourceId::Invalid)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (auto pShadowMapBuffer = pResourceManager->GetResource<IDepthBufferTarget>(shadowMapHandle))
		{
			pShadowMapBuffer->SetUWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pShadowMapBuffer->SetVWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pShadowMapBuffer->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

			if (pShadowMapBuffer->GetWidth() != shadowMapSizes)
			{
				LOG_MESSAGE(Wrench::StringUtils::Format("[CForwardRenderer] The shadow map sizes has been changed, from {0}x{0} to {1}x{1}", pShadowMapBuffer->GetWidth(), shadowMapSizes));
				pShadowMapBuffer->Resize(shadowMapSizes, shadowMapSizes);
			}
		}

		return Wrench::TOkValue<TResourceId>(shadowMapHandle);
	}


	static TResult<TResourceId> GetOrCreatePointShadowMap(TPtr<IResourceManager> pResourceManager, USIZE pointLightIndex)
	{
		const U32 shadowMapSizes = static_cast<U32>(CGameUserSettings::Get()->mpShadowMapSizesCVar->Get());
		TDE2_ASSERT(shadowMapSizes > 0 && shadowMapSizes < 65536);

		TRenderTargetParameters shadowMapParams;
		shadowMapParams.mWidth = shadowMapSizes;
		shadowMapParams.mHeight = shadowMapSizes;
		shadowMapParams.mFormat = FT_D32;
		shadowMapParams.mNumOfMipLevels = 1;
		shadowMapParams.mNumOfSamples = 1;
		shadowMapParams.mSamplingQuality = 0;
		shadowMapParams.mType = TRenderTargetParameters::E_TARGET_TYPE::CUBEMAP;

		const TResourceId shadowMapHandle = pResourceManager->Create<IDepthBufferTarget>("PointShadowMap" + std::to_string(pointLightIndex), shadowMapParams);
		if (shadowMapHandle == TResourceId::Invalid)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (auto pShadowMapBuffer = pResourceManager->GetResource<IDepthBufferTarget>(shadowMapHandle))
		{
			pShadowMapBuffer->SetUWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pShadowMapBuffer->SetVWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pShadowMapBuffer->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

			if (pShadowMapBuffer->GetWidth() != shadowMapSizes)
			{
				LOG_MESSAGE(Wrench::StringUtils::Format("[CForwardRenderer] The shadow map sizes has been changed, from {0}x{0} to {1}x{1}", pShadowMapBuffer->GetWidth(), shadowMapSizes));
				pShadowMapBuffer->Resize(shadowMapSizes, shadowMapSizes);
			}
		}

		return Wrench::TOkValue<TResourceId>(shadowMapHandle);
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


	static TResult<std::tuple<TResourceId, TBufferHandleId>> InitLightGridAndBuffer(const TRendererInitParams& params, U32 workGroupsX, U32 workGroupsY, const std::string& prefix)
	{
		auto&& graphicsObjectManager = params.mpGraphicsContext->GetGraphicsObjectManager();

		const U32 tilesCount = workGroupsX * workGroupsY;
		const USIZE bufferSize = sizeof(U32) * tilesCount * MAX_LIGHTS_PER_TILE_BLOCK;

		// \note Create lights indices buffer
		auto createBufferResult = graphicsObjectManager->CreateBuffer(
			{
				E_BUFFER_USAGE_TYPE::DEFAULT,
				E_BUFFER_TYPE::STRUCTURED,
				bufferSize,
				nullptr,
				bufferSize,
				true,
				sizeof(U32),
				E_STRUCTURED_BUFFER_TYPE::DEFAULT
			});

		if (createBufferResult.HasError())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(createBufferResult.GetError());
		}

		// \note Create the light grid's texture
		TTexture2DParameters lightGridTextureParams
		{
			workGroupsX,
			workGroupsY,
			FT_UINT2, 1, 1, 0
		};
		lightGridTextureParams.mIsWriteable = true;

		TResourceId lightGridHandle = params.mpResourceManager->Create<ITexture2D>("LightGridTexture" + prefix, lightGridTextureParams);
		if (auto pTexture = params.mpResourceManager->GetResource<ITexture2D>(lightGridHandle))
		{
			pTexture->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);
			pTexture->SetUWrapMode(E_ADDRESS_MODE_TYPE::AMT_WRAP);
			pTexture->SetVWrapMode(E_ADDRESS_MODE_TYPE::AMT_WRAP);
			pTexture->SetWWrapMode(E_ADDRESS_MODE_TYPE::AMT_WRAP);

			if (pTexture->GetWidth() != workGroupsX || pTexture->GetHeight() != workGroupsY)
			{
				pTexture->Resize(workGroupsX, workGroupsY);
			}
		}

		return Wrench::TOkValue<std::tuple<TResourceId, TBufferHandleId>>(std::make_tuple(lightGridHandle, createBufferResult.Get()));
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
		TLightCullingData data;

		auto&& graphicsObjectManager = params.mpGraphicsContext->GetGraphicsObjectManager();

		const U32 width = params.mpWindowSystem->GetWidth();
		const U32 height = params.mpWindowSystem->GetHeight();

		data.mWorkGroupsX = (width + (width % LIGHT_GRID_TILE_BLOCK_SIZE)) / LIGHT_GRID_TILE_BLOCK_SIZE;
		data.mWorkGroupsY = (height + (height % LIGHT_GRID_TILE_BLOCK_SIZE)) / LIGHT_GRID_TILE_BLOCK_SIZE;

		auto opaqueLightCullingStructsResult = InitLightGridAndBuffer(params, data.mWorkGroupsX, data.mWorkGroupsY, "_OpaqueObjects");
		if (opaqueLightCullingStructsResult.HasError())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(opaqueLightCullingStructsResult.GetError());
		}
		
		std::tie(data.mOpaqueLightGridTextureHandle, data.mOpaqueVisibleLightsBufferHandle) = opaqueLightCullingStructsResult.Get();

		auto transparentLightCullingStructsResult = InitLightGridAndBuffer(params, data.mWorkGroupsX, data.mWorkGroupsY, "_TransparentObjects");
		if (transparentLightCullingStructsResult.HasError())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(transparentLightCullingStructsResult.GetError());
		}

		std::tie(data.mTransparentLightGridTextureHandle, data.mTransparentVisibleLightsBufferHandle) = transparentLightCullingStructsResult.Get();

		// \note Create buffer for light index counters
		auto lightIndexCountersCreateResult = CreateLightIndexCountersBuffer(graphicsObjectManager);
		if (lightIndexCountersCreateResult.HasError())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(lightIndexCountersCreateResult.GetError());
		}

		data.mLightIndexCountersBufferHandle = lightIndexCountersCreateResult.Get();

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


	E_RESULT_CODE CForwardRenderer::Init(const TRendererInitParams& params)
	{
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
		mpGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_CONSTANTS, nullptr, 0);
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

		if (CGameUserSettings::Get()->mpIsShadowMappingEnabledCVar->Get())
		{
			GetOrCreateDirectionalShadowMap(mpResourceManager).Get(); /// \note Create a shadow map's texture before any Update will be executed
			GetOrCreatePointShadowMap(mpResourceManager, 0).Get();
		}

		mpFramePostProcessor = TPtr<IFramePostProcessor>(CreateFramePostProcessor({ this, mpGraphicsContext->GetGraphicsObjectManager(), mpWindowSystem.Get() }, result));
		if (result != RC_OK)
		{
			return result;
		}

		auto lightGridDataResult = InitLightGrid(params);
		if (lightGridDataResult.HasError())
		{
			return lightGridDataResult.GetError();
		}

		mLightGridData = lightGridDataResult.Get();

		mIsInitialized = true;

		return RC_OK;
	}


	static inline void SubmitCommandsToDraw(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TPtr<IGlobalShaderProperties> pGlobalShaderProperties,
									TPtr<CRenderQueue> pRenderQueue, U32 upperRenderIndexLimit)
	{
		CRenderQueue::CRenderQueueIterator iter = pRenderQueue->GetIterator();

		TRenderCommand* pCurrDrawCommand = nullptr;

		const TRenderCommandSubmitParams drawContext { pGraphicsContext.Get(), pResourceManager.Get(), pGlobalShaderProperties.Get() };

		while (iter.HasNext())
		{
			pCurrDrawCommand = *(iter++);

			if (!pCurrDrawCommand)
			{
				continue;
			}

			if (iter.GetIndex() >= upperRenderIndexLimit)
			{
				break;
			}

			pCurrDrawCommand->Submit(drawContext);
		}
	}


	static inline void ExecuteDrawCommands(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TPtr<IGlobalShaderProperties> pGlobalShaderProperties,
									TPtr<CRenderQueue> pCommandsBuffer, bool shouldClearBuffers, U32 upperRenderIndexLimit)
	{
		if (pCommandsBuffer->IsEmpty())
		{
			return;
		}

		pCommandsBuffer->Sort();
		SubmitCommandsToDraw(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pCommandsBuffer, upperRenderIndexLimit);

		if (shouldClearBuffers)
		{
			pCommandsBuffer->Clear();
		}
	};


	static void RenderShadowCasters(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TResourceId shadowMapHandle, const std::function<void()> action)
	{
		const F32 shadowMapSizes = static_cast<F32>(CGameUserSettings::Get()->mpShadowMapSizesCVar->Get());

		pGraphicsContext->SetViewport(0.0f, 0.0f, shadowMapSizes, shadowMapSizes, 0.0f, 1.0f);
		{
			pGraphicsContext->BindDepthBufferTarget(pResourceManager->GetResource<IDepthBufferTarget>(shadowMapHandle).Get(), true);

			pGraphicsContext->ClearDepthBuffer(1.0f);

			if (action)
			{
				action();
			}

			pGraphicsContext->BindDepthBufferTarget(nullptr);
		}

		if (auto pWindowSystem = pGraphicsContext->GetWindowSystem())
		{
			pGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(pWindowSystem->GetWidth()), static_cast<F32>(pWindowSystem->GetHeight()), 0.0f, 1.0f);
		}
	}


	static E_RESULT_CODE ProcessShadowPass(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TPtr<IGlobalShaderProperties> pGlobalShaderProperties,
										TPtr<CRenderQueue> pShadowCastersRenderGroup)
	{
		if (!pShadowCastersRenderGroup)
		{
			LOG_ERROR("[ForwardRenderer] Invalid \"Shadow Pass\" commands buffer was found");
			return RC_FAIL;
		}

		TDE2_PROFILER_SCOPE("Renderer::RenderShadows");
		TDE_RENDER_SECTION(pGraphicsContext, "RenderShadows");

		static std::vector<TResourceId> shadowMapHandles;
		shadowMapHandles.clear();

		shadowMapHandles.push_back(GetOrCreateDirectionalShadowMap(pResourceManager).Get());
		shadowMapHandles.push_back(GetOrCreatePointShadowMap(pResourceManager, 0).Get());

		/// \todo Replace hardcoded identifiers
		const TResourceId shadowPassMaterialHandle = pResourceManager->Load<IMaterial>("ShadowPassMaterial.material");

		for (const TResourceId currShadowMapHandle : shadowMapHandles)
		{
			/// \todo Assign "ShadowPass" material's variables that defines which type of a light casts shadows
			if (auto pMaterial = pResourceManager->GetResource<IMaterial>(shadowPassMaterialHandle))
			{
				pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, "mIsSunLight", shadowMapHandles.front() == currShadowMapHandle);
				pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, "mPointLightIndex", 0);
			}

			if (!pShadowCastersRenderGroup->IsEmpty())
			{
				RenderShadowCasters(pGraphicsContext, pResourceManager, currShadowMapHandle, [&]
				{
					ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pShadowCastersRenderGroup, shadowMapHandles.back() == currShadowMapHandle);
				});
			}
			else
			{
				RenderShadowCasters(pGraphicsContext, pResourceManager, currShadowMapHandle, nullptr);
			}
		}

		return RC_OK;
	}


#if TDE2_EDITORS_ENABLED

	static E_RESULT_CODE ProcessEditorSelectionBuffer(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TPtr<IGlobalShaderProperties> pGlobalShaderProperties,
													ISelectionManager* pSelectionManager, TPtr<CRenderQueue> pRenderGroup)
	{
		if (!pRenderGroup)
		{
			LOG_ERROR("[ForwardRenderer] Invalid \"Editor Only\" commands buffer was found");
			return RC_FAIL;
		}

		TDE2_PROFILER_SCOPE("Renderer::RenderSelectionBuffer");
		TDE_RENDER_SECTION(pGraphicsContext, "RenderSelectionBuffer");

		if (!pRenderGroup->IsEmpty() && pSelectionManager)
		{
			if (pSelectionManager->BuildSelectionMap([&, pRenderGroup]
			{
				ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pRenderGroup, true);
				return RC_OK;
			}) != RC_OK)
			{
				TDE2_ASSERT(false);
			}
		}

		return RC_OK;
	}

#endif


	static E_RESULT_CODE CullLights(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, const TLightCullingData& lightGridData)
	{
		TDE2_PROFILER_SCOPE("LightCulling");
		TDE_RENDER_SECTION(pGraphicsContext, "LightCulling");

		E_RESULT_CODE result = pGraphicsContext->CopyResource(lightGridData.mLightIndexCountersInitializerBufferHandle, lightGridData.mLightIndexCountersBufferHandle);

		// \note Cull lighting shader
		result = result | pGraphicsContext->SetStructuredBuffer(OPAQUE_VISIBLE_LIGHTS_BUFFER_SLOT, lightGridData.mOpaqueVisibleLightsBufferHandle, true);
		result = result | pGraphicsContext->SetStructuredBuffer(TRANSPARENT_VISIBLE_LIGHTS_BUFFER_SLOT, lightGridData.mTransparentVisibleLightsBufferHandle, true);
		result = result | pGraphicsContext->SetStructuredBuffer(LIGHT_INDEX_COUNTERS_BUFFER_SLOT, lightGridData.mLightIndexCountersBufferHandle, true);

		struct
		{
			U32 mWorkGroupsX = 0;
			U32 mWorkGroupsY = 0;
		} shaderParameters;

		shaderParameters.mWorkGroupsX = lightGridData.mWorkGroupsX;
		shaderParameters.mWorkGroupsY = lightGridData.mWorkGroupsY;

		auto pLightCullShader = pResourceManager->GetResource<IShader>(pResourceManager->Load<IShader>("Shaders/Default/ForwardLightCulling.cshader"));
		if (pLightCullShader)
		{
			pLightCullShader->SetTextureResource("OpaqueLightGridTexture", pResourceManager->GetResource<ITexture>(lightGridData.mOpaqueLightGridTextureHandle).Get());
			pLightCullShader->SetTextureResource("TransparentLightGridTexture", pResourceManager->GetResource<ITexture>(lightGridData.mTransparentLightGridTextureHandle).Get());
			pLightCullShader->SetTextureResource("DepthTexture", pResourceManager->GetResource<ITexture>(lightGridData.mMainDepthBufferHandle).Get());
			pLightCullShader->SetStructuredBufferResource("TileFrustums", lightGridData.mTileFrustumsBufferHandle);
			pLightCullShader->SetUserUniformsBuffer(0, reinterpret_cast<U8*>(&shaderParameters), sizeof(shaderParameters));
			
			pLightCullShader->Bind();
		}

		pGraphicsContext->DispatchCompute(lightGridData.mWorkGroupsX, lightGridData.mWorkGroupsY, 1);

		if (pLightCullShader)
		{
			pLightCullShader->Unbind();
		}

		// unbind lights indices buffers
		result = result | pGraphicsContext->SetStructuredBuffer(OPAQUE_VISIBLE_LIGHTS_BUFFER_SLOT, TBufferHandleId::Invalid, true);
		result = result | pGraphicsContext->SetStructuredBuffer(TRANSPARENT_VISIBLE_LIGHTS_BUFFER_SLOT, TBufferHandleId::Invalid, true);
		result = result | pGraphicsContext->SetStructuredBuffer(LIGHT_INDEX_COUNTERS_BUFFER_SLOT, TBufferHandleId::Invalid, true);

		return result;
	}


	static inline E_RESULT_CODE RenderMainPasses(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TPtr<IGlobalShaderProperties> pGlobalShaderProperties,
										TPtr<IFramePostProcessor> pFramePostProcessor, TPtr<CRenderQueue> pRenderQueues[], TLightCullingData& lightGridData)
	{
		TDE2_PROFILER_SCOPE("Renderer::RenderAll");

		pFramePostProcessor->PreRender();

		{
			TDE_RENDER_SECTION(pGraphicsContext, "DepthPrePass");

			pFramePostProcessor->Render([&]
			{
				ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_DEPTH_PREPASS)], true);
			}, E_FRAME_RENDER_PARAMS_FLAGS::BIND_DEPTH_BUFFER);
		}

		lightGridData.mMainDepthBufferHandle = pFramePostProcessor->GetMainDepthBufferHandle();
		CullLights(pGraphicsContext, pResourceManager, lightGridData);

		{
			TDE_RENDER_SECTION(pGraphicsContext, "RenderMainPass");

			pFramePostProcessor->Render([&]
			{
				pGraphicsContext->SetStructuredBuffer(VISIBLE_LIGHTS_BUFFER_SLOT, lightGridData.mOpaqueVisibleLightsBufferHandle, false);
				
				if (auto pLightGridTexture = pResourceManager->GetResource<ITexture>(lightGridData.mOpaqueLightGridTextureHandle))
				{
					pLightGridTexture->SetWriteable(false);
					pLightGridTexture->Bind(LIGHT_GRID_TEXTURE_SLOT);
				}

				ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties, 
					pRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_OPAQUE_GEOMETRY)], true); // opaque

				pGraphicsContext->SetStructuredBuffer(VISIBLE_LIGHTS_BUFFER_SLOT, lightGridData.mTransparentVisibleLightsBufferHandle, false);

				if (auto pLightGridTexture = pResourceManager->GetResource<ITexture>(lightGridData.mTransparentLightGridTextureHandle))
				{
					pLightGridTexture->SetWriteable(false);
					pLightGridTexture->Bind(LIGHT_GRID_TEXTURE_SLOT);
				}

				ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties, 
					pRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_TRANSPARENT_GEOMETRY)], true); // transparent

				ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties,
					pRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_SPRITES)], true); // sprites

				ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties,
					pRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_DEBUG)], true); // debug
			}, E_FRAME_RENDER_PARAMS_FLAGS::BIND_DEPTH_BUFFER | E_FRAME_RENDER_PARAMS_FLAGS::CLEAR_RENDER_TARGET | E_FRAME_RENDER_PARAMS_FLAGS::RENDER_MAIN);
		}

		if (auto pLightGridTexture = pResourceManager->GetResource<ITexture>(lightGridData.mOpaqueLightGridTextureHandle))
		{
			pLightGridTexture->SetWriteable(false);
			pLightGridTexture->Bind(LIGHT_GRID_TEXTURE_SLOT);
		}

		return RC_OK;
	}


	static E_RESULT_CODE RenderOverlayAndPostEffects(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TPtr<IGlobalShaderProperties> pGlobalShaderProperties,
		TPtr<IFramePostProcessor> pFramePostProcessor, TPtr<CRenderQueue> pUIRenderGroup, TPtr<CRenderQueue> pDebugUIRenderGroup)
	{
		if (!pUIRenderGroup)
		{
			LOG_ERROR("[ForwardRenderer] Invalid \"Overlays\" commands buffer was found");
			return RC_INVALID_ARGS;
		}

		{
			TDE2_PROFILER_SCOPE("CFramePostProcessor::RunPostProcess");
			TDE_RENDER_SECTION(pGraphicsContext, "PostProcessing");

			pFramePostProcessor->RunPostProcess();
		}

		{
			TDE2_PROFILER_SCOPE("Renderer::UI");
			TDE_RENDER_SECTION(pGraphicsContext, "RenderUI");

			pFramePostProcessor->Render([&] /// Render UI elements
			{
				ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pUIRenderGroup, true);
			}, E_FRAME_RENDER_PARAMS_FLAGS::CLEAR_RENDER_TARGET | E_FRAME_RENDER_PARAMS_FLAGS::RENDER_UI);
		}

		{
			TDE_RENDER_SECTION(pGraphicsContext, "FinalOutput");

			pFramePostProcessor->PostRender();

			if (pUIRenderGroup->IsEmpty())
			{
				return RC_FAIL;
			}

			pGraphicsContext->ClearDepthBuffer(1.0f);
			ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pUIRenderGroup, true);
		}

		{
			TDE_RENDER_SECTION(pGraphicsContext, "DebugUI");
			ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pDebugUIRenderGroup, true);
		}

		return RC_OK;
	}


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
			frameGraphBlackboard.mLightCullingData.mTileFrustumsBufferHandle = mpFrameGraph->ImportResource("TileFrustums", TFrameGraphBuffer::TDesc { }, TFrameGraphBuffer{ mLightGridData.mTileFrustumsBufferHandle });
			frameGraphBlackboard.mLightCullingData.mLightIndexCountersInitializerBufferHandle = mpFrameGraph->ImportResource("InitialLightIndexCounters", TFrameGraphBuffer::TDesc { }, TFrameGraphBuffer{ mLightGridData.mLightIndexCountersInitializerBufferHandle });

			// \note editor mode (draw into selection buffer)
#if TDE2_EDITORS_ENABLED
			CRenderSelectionBufferPass{}.AddPass(mpFrameGraph, frameGraphBlackboard, CRenderSelectionBufferPass::TAddPassInvokeParams
				{
					mpGraphicsContext,
					mpResourceManager,
					mpGlobalShaderProperties,
					mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_EDITOR_ONLY)],
					mpWindowSystem->GetWidth(),
					mpWindowSystem->GetHeight()
				});
#endif

			// \note directional shadow pass
			if (CGameUserSettings::Get()->mpIsShadowMappingEnabledCVar->Get())
			{
				CSunLightShadowPass
				{
					TPassInvokeContext
					{
						mpGraphicsContext,
						mpResourceManager,
						mpGlobalShaderProperties,
						mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_SHADOW_PASS)],
						mpWindowSystem->GetWidth(),
						mpWindowSystem->GetHeight()
					}
				}.AddPass(mpFrameGraph, frameGraphBlackboard);

				U32 currPointLightIndex = 0;

				// \note omni shadow passes
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

					COmniLightShadowPass
					{
						TPassInvokeContext
						{
							mpGraphicsContext,
							mpResourceManager,
							mpGlobalShaderProperties,
							mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_SHADOW_PASS)],
							mpWindowSystem->GetWidth(),
							mpWindowSystem->GetHeight()
						}
					}.AddPass(mpFrameGraph, frameGraphBlackboard, currPointLightIndex);

					++currPointLightIndex;
				}
			}

			// \note depth pre-pass
			CDepthPrePass
			{
				TPassInvokeContext
				{
					mpGraphicsContext,
					mpResourceManager,
					mpGlobalShaderProperties,
					mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_DEPTH_PREPASS)],
					mpWindowSystem->GetWidth(),
					mpWindowSystem->GetHeight()
				}
			}.AddPass(mpFrameGraph, frameGraphBlackboard);

			// \note upload lights buffer pass
			CUploadLightsPass
			{ 
				TPassInvokeContext
				{
					mpGraphicsContext,
					mpResourceManager,
					mpGlobalShaderProperties,
					nullptr,
					mpWindowSystem->GetWidth(),
					mpWindowSystem->GetHeight()
				}
			}.AddPass(mpFrameGraph, frameGraphBlackboard, mActiveLightSources);

			// \note light culling pass
			CLightCullingPass
			{
				TPassInvokeContext
				{
					mpGraphicsContext,
					mpResourceManager,
					mpGlobalShaderProperties,
					nullptr,
					mpWindowSystem->GetWidth(),
					mpWindowSystem->GetHeight()
				}
			}.AddPass(mpFrameGraph, frameGraphBlackboard);

			// 
			// \note main pass
			COpaqueRenderPass
			{ 
				TPassInvokeContext
				{
					mpGraphicsContext,
					mpResourceManager,
					mpGlobalShaderProperties,
					mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_OPAQUE_GEOMETRY)],
					mpWindowSystem->GetWidth(),
					mpWindowSystem->GetHeight()
				}
			}.AddPass(mpFrameGraph, frameGraphBlackboard, true); // \todo replace with configuration of hdr support

			CTransparentRenderPass
			{
				TPassInvokeContext
				{
					mpGraphicsContext,
					mpResourceManager,
					mpGlobalShaderProperties,
					mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_TRANSPARENT_GEOMETRY)],
					mpWindowSystem->GetWidth(),
					mpWindowSystem->GetHeight()
				}
			}.AddPass(mpFrameGraph, frameGraphBlackboard); 

			CSpritesRenderPass
			{
				TPassInvokeContext
				{
					mpGraphicsContext,
					mpResourceManager,
					mpGlobalShaderProperties,
					mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_SPRITES)],
					mpWindowSystem->GetWidth(),
					mpWindowSystem->GetHeight()
				}
			}.AddPass(mpFrameGraph, frameGraphBlackboard);

			CDebugRenderPass
			{
				TPassInvokeContext
				{
					mpGraphicsContext,
					mpResourceManager,
					mpGlobalShaderProperties,
					mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_DEBUG)],
					mpWindowSystem->GetWidth(),
					mpWindowSystem->GetHeight()
				}
			}.AddPass(mpFrameGraph, frameGraphBlackboard);

			if (CGameUserSettings::Get()->mpIsVolumetricCloudsEnabledCVar->Get())
			{
				// \note volumetric clouds main pass
				CVolumetricCloudsMainPass
				{
					TPassInvokeContext
					{
						mpGraphicsContext,
						mpResourceManager,
						mpGlobalShaderProperties,
						nullptr,
						mpWindowSystem->GetWidth(),
						mpWindowSystem->GetHeight()
					}
				}.AddPass(mpFrameGraph, frameGraphBlackboard);

				// \note volumetric clouds blur pass
				CVolumetricCloudsUpscalePass
				{
					TPassInvokeContext
					{
						mpGraphicsContext,
						mpResourceManager,
						mpGlobalShaderProperties,
						nullptr,
						mpWindowSystem->GetWidth(),
						mpWindowSystem->GetHeight()
					}
				}.AddPass(mpFrameGraph, frameGraphBlackboard);

				// \note volumetric clouds compose pass
				CVolumetricCloudsComposePass
				{
					TPassInvokeContext
					{
						mpGraphicsContext,
						mpResourceManager,
						mpGlobalShaderProperties,
						nullptr,
						mpWindowSystem->GetWidth(),
						mpWindowSystem->GetHeight()
					}
				}.AddPass(mpFrameGraph, frameGraphBlackboard);
			}

			// 
			// \todo lights heatmap debug pass
			// \todo eye-adaptation pass
			// 
			// \todo bloom threshold
			// \todo bloom horizontal pass
			// \todo bloom vertical pass
			// \todo bloom compose
			// 
			// \todo tone-mapping pass
			// \todo ui pass
			// \todo compose pass
			// \todo imgui pass

			mpFrameGraph->Compile();
			mpFrameGraph->Execute();

#if TDE2_EDITORS_ENABLED
#if 1
			{
				E_RESULT_CODE result = RC_OK;
				auto pFileStream = TPtr<TDEngine2::IStream>(CreateFileOutputStream("forward_renderer.dot", result));
				ITextFileWriter* pDumpWriter = dynamic_cast<ITextFileWriter*>(CreateTextFileWriter(nullptr, pFileStream, result));

				mpFrameGraph->Dump(pDumpWriter);
				pDumpWriter->Close();
			}
#endif

			ProcessEditorSelectionBuffer(mpGraphicsContext, mpResourceManager, mpGlobalShaderProperties, mpSelectionManager, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_EDITOR_ONLY)]);
#endif

			if (CGameUserSettings::Get()->mpIsShadowMappingEnabledCVar->Get())
			{
				ProcessShadowPass(mpGraphicsContext, mpResourceManager, mpGlobalShaderProperties, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_SHADOW_PASS)]);
			}

			RenderMainPasses(mpGraphicsContext, mpResourceManager, mpGlobalShaderProperties, mpFramePostProcessor, mpRenderQueues, mLightGridData);

			{
				TDE_RENDER_SECTION(mpGraphicsContext, "VolumetricCloudsPass");
				mpFramePostProcessor->RunVolumetricCloudsPass();
			}

			RenderOverlayAndPostEffects(mpGraphicsContext, mpResourceManager, mpGlobalShaderProperties, mpFramePostProcessor, 
				mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_OVERLAY)],
				mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_DEBUG_UI)]);
		}

		{
			TDE2_BUILTIN_SPEC_PROFILER_EVENT(E_SPECIAL_PROFILE_EVENT::PRESENT);
			mpGraphicsContext->Present();
		}

		mpDebugUtility->PostRender();

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

		if (!mpFramePostProcessor)
		{
			return RC_FAIL;
		}

		return mpFramePostProcessor->SetProcessingProfile(pProfileResource);
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
		
		if (TBufferHandleId::Invalid != mLightGridData.mOpaqueVisibleLightsBufferHandle)
		{
			result = result | pGraphicsObjectManager->DestroyBuffer(mLightGridData.mOpaqueVisibleLightsBufferHandle);
			result = result | pGraphicsObjectManager->DestroyBuffer(mLightGridData.mTransparentVisibleLightsBufferHandle);
			result = result | pGraphicsObjectManager->DestroyBuffer(mLightGridData.mLightIndexCountersBufferHandle);
		}

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
			perFrameShaderData.mProjMatrix        = Transpose(mpMainCamera->GetProjMatrix());
			perFrameShaderData.mViewMatrix        = Transpose(mpMainCamera->GetViewMatrix());
			perFrameShaderData.mInvProjMatrix     = Transpose(Inverse(mpMainCamera->GetProjMatrix()));
			perFrameShaderData.mInvViewMatrix     = Transpose(Inverse(mpMainCamera->GetViewMatrix()));
			perFrameShaderData.mInvViewProjMatrix = Transpose(mpMainCamera->GetInverseViewProjMatrix());
			perFrameShaderData.mCameraPosition    = TVector4(mpMainCamera->GetPosition(), 1.0f);
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


	TDE2_API IRenderer* CreateForwardRenderer(const TRendererInitParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IRenderer, CForwardRenderer, result, params);
	}
}