#include "../../include/ecs/CLightingSystem.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/graphics/CStaticMesh.h"
#include "../../include/graphics/ICamera.h"
#include "../../include/graphics/CStaticMeshContainer.h"
#include "../../include/graphics/CSkinnedMesh.h"
#include "../../include/graphics/CSkinnedMeshContainer.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/InternalShaderData.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/CFramePacketsStorage.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/CGameUserSettings.h"
#include "../../include/utils/ITimer.h"
#include "../../include/scene/components/CDirectionalLight.h"
#include "../../include/scene/components/CPointLight.h"
#include "../../include/scene/components/CSpotLight.h"
#include "../../include/scene/components/ShadowMappingComponents.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CPerfProfiler.h"


namespace TDEngine2
{
	CLightingSystem::CLightingSystem() :
		CBaseSystem(),
		mShadowPassMaterialHandle(TResourceId::Invalid),
		mShadowPassSkinnedMaterialHandle(TResourceId::Invalid)
	{
	}

	E_RESULT_CODE CLightingSystem::Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, TPtr<ITimer> pTimer)
	{
		TDE2_PROFILER_SCOPE("CLightingSystem::Init");

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pRenderer || !pGraphicsObjectManager || !pTimer)
		{
			return RC_INVALID_ARGS;
		}

		mpRenderer              = pRenderer;
		mpGraphicsObjectManager = pGraphicsObjectManager;
		mpResourceManager       = mpRenderer->GetResourceManager();
		mpFramePacketsStorage   = mpRenderer->GetFramePacketsStorage().Get();
		mpGraphicsContext       = pGraphicsObjectManager->GetGraphicsContext();
		mpTimer                 = pTimer;

		E_RESULT_CODE result = _prepareResources();
		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	void CLightingSystem::InjectBindings(IWorld* pWorld)
	{
		mDirectionalLightsContext = pWorld->CreateLocalComponentsSlice<CDirectionalLight, CTransform>();
		mPointLightsContext       = pWorld->CreateLocalComponentsSlice<CPointLight, CTransform>();
		mSpotLightsContext        = pWorld->CreateLocalComponentsSlice<CSpotLight, CTransform>();
		
		mStaticShadowCastersContext  = pWorld->CreateLocalComponentsSlice<CShadowCasterComponent, CStaticMeshContainer, CTransform>();
		mSkinnedShadowCastersContext = pWorld->CreateLocalComponentsSlice<CShadowCasterComponent, CSkinnedMeshContainer, CTransform>();

		mStaticShadowReceiversContext = pWorld->CreateLocalComponentsSlice<CShadowReceiverComponent, CStaticMeshContainer>();
		mSkinnedShadowReceiversContext = pWorld->CreateLocalComponentsSlice<CShadowReceiverComponent, CSkinnedMeshContainer>();
	}


	struct TProcessParams
	{
		IResourceManager*   mpResourceManager;
		IVertexDeclaration* mpVertexDeclaration;
		TResourceId         mMaterialId;
		U32                 mDrawIndex;
		CRenderQueue*       mpRenderQueue;
	};


	static U32 ProcessStaticMeshCasterEntity(const TProcessParams& params, CLightingSystem::TStaticShadowCastersContext& staticShadowCasters, USIZE id)
	{
		IResourceManager* pResourceManager = params.mpResourceManager;

		auto&& staticMeshContainers = std::get<std::vector<CStaticMeshContainer*>>(staticShadowCasters.mComponentsSlice);
		auto&& transforms = std::get<std::vector<CTransform*>>(staticShadowCasters.mComponentsSlice);

		CStaticMeshContainer* pStaticMeshContainer = staticMeshContainers[id];
		if (!pStaticMeshContainer)
		{
			return params.mDrawIndex;
		}

		CTransform* pTransform = transforms[id];

		TResourceId meshResourceHandle = pResourceManager->Load<IStaticMesh>(pStaticMeshContainer->GetMeshName());
		TDE2_ASSERT(meshResourceHandle != TResourceId::Invalid);

		if (auto pStaticMeshResource = pResourceManager->GetResource<IStaticMesh>(meshResourceHandle))
		{
			/// \note Skip rest steps if the resource isn't loaded yet
			if (auto pResource = pResourceManager->GetResource(meshResourceHandle))
			{
				if (E_RESOURCE_STATE_TYPE::RST_LOADED != pResource->GetState())
				{
					return params.mDrawIndex;
				}
			}

			auto&& subMeshInfo = pStaticMeshContainer->GetSubMeshInfo();

			if (TDrawIndexedCommand* pDrawCommand = params.mpRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(params.mDrawIndex))
			{
				pDrawCommand->mVertexBufferHandle = pStaticMeshResource->GetPositionOnlyVertexBuffer();
				pDrawCommand->mIndexBufferHandle = pStaticMeshResource->GetSharedIndexBuffer();
				pDrawCommand->mMaterialHandle = params.mMaterialId;
				pDrawCommand->mPrimitiveType = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
				pDrawCommand->mpVertexDeclaration = params.mpVertexDeclaration;
				pDrawCommand->mObjectData.mModelMatrix = Transpose(pTransform->GetLocalToWorldTransform());
				pDrawCommand->mObjectData.mObjectID = static_cast<U32>(id);
				pDrawCommand->mStartIndex = subMeshInfo.mStartIndex;
				pDrawCommand->mStartVertex = 0;
				pDrawCommand->mNumOfIndices = subMeshInfo.mIndicesCount;
			}
		}

		return params.mDrawIndex + 1;
	}

	static U32 ProcessSkinnedMeshCasterEntity(const TProcessParams& params, CLightingSystem::TSkinnedShadowCastersContext& skinnedShadowCasters, USIZE id)
	{
		IResourceManager* pResourceManager = params.mpResourceManager;

		auto&& skinnedMeshContainers = std::get<std::vector<CSkinnedMeshContainer*>>(skinnedShadowCasters.mComponentsSlice);
		auto&& transforms = std::get<std::vector<CTransform*>>(skinnedShadowCasters.mComponentsSlice);

		CSkinnedMeshContainer* pSkinnedMeshContainer = skinnedMeshContainers[id];
		if (!pSkinnedMeshContainer)
		{
			return params.mDrawIndex;
		}

		CTransform* pTransform = transforms[id];

		TResourceId meshResourceHandle = pResourceManager->Load<ISkinnedMesh>(pSkinnedMeshContainer->GetMeshName());
		TDE2_ASSERT(meshResourceHandle != TResourceId::Invalid);

		if (auto pSkinnedMeshResource = pResourceManager->GetResource<ISkinnedMesh>(meshResourceHandle))
		{
			/// \note Skip rest steps if the resource isn't loaded yet
			if (auto pResource = pResourceManager->GetResource(meshResourceHandle))
			{
				if (E_RESOURCE_STATE_TYPE::RST_LOADED != pResource->GetState())
				{
					return params.mDrawIndex;
				}
			}

			const auto& currAnimationPose = pSkinnedMeshContainer->GetCurrentAnimationPose();
			const U32 jointsCount = static_cast<U32>(currAnimationPose.size());

			if (auto pMaterial = pResourceManager->GetResource<IMaterial>(params.mMaterialId))
			{
				if (E_RESOURCE_STATE_TYPE::RST_LOADED != pResourceManager->GetResource<IResource>(params.mMaterialId)->GetState())
				{
					return params.mDrawIndex;
				}

				pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, CSkinnedMeshContainer::mJointsArrayUniformVariableId, &currAnimationPose.front(), static_cast<U32>(sizeof(TMatrix4) * currAnimationPose.size()));
				pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, CSkinnedMeshContainer::mJointsCountUniformVariableId, &jointsCount, sizeof(U32));
			}

			auto&& subMeshInfo = pSkinnedMeshContainer->GetSubMeshInfo();

			if (TDrawIndexedCommand* pDrawCommand = params.mpRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(params.mDrawIndex))
			{
				pDrawCommand->mVertexBufferHandle = pSkinnedMeshResource->GetPositionOnlyVertexBuffer();
				pDrawCommand->mIndexBufferHandle = pSkinnedMeshResource->GetSharedIndexBuffer();
				pDrawCommand->mMaterialHandle = params.mMaterialId;
				pDrawCommand->mPrimitiveType = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
				pDrawCommand->mpVertexDeclaration = params.mpVertexDeclaration;
				pDrawCommand->mObjectData.mModelMatrix = Transpose(pTransform->GetLocalToWorldTransform());
				pDrawCommand->mObjectData.mObjectID = static_cast<U32>(id);
				pDrawCommand->mStartIndex = subMeshInfo.mStartIndex;
				pDrawCommand->mStartVertex = 0;
				pDrawCommand->mNumOfIndices = subMeshInfo.mIndicesCount;
			}
		}

		return params.mDrawIndex + 1;
	}


	static std::vector<F32> CalcShadowSplitsPlane(IGraphicsContext* pGraphicsContext, ICamera* pActiveCamera)
	{
		const auto& gameSettings = CGameUserSettings::Get();

		std::vector<F32> output;
		output.resize(static_cast<USIZE>(CGameUserSettings::Get()->mpShadowCascadesCountCVar->Get()) + 1);

		const F32 zn = pActiveCamera->GetNearPlane();
		const F32 zf = pActiveCamera->GetFarPlane();

		output.front() = zn;
		
		for (USIZE i = 1; i < output.size() - 1; i++)
		{
			output[i] = CMathUtils::Lerp(zn, zf, CGameUserSettings::Get()->mpShadowCascadesSplitsCVar[i - 1]->Get());
		}

		output.back() = zf;

		return output;
	}


	static std::array<TVector4, 8> GetCascadeFrustumVertices(IGraphicsContext* pGraphicsContext, IPerspectiveCamera* pActiveCamera, F32 zn, F32 zf) /// zMin - least value for Z axis in NDC space (depends on GAPI)
	{
		if (!pActiveCamera)
		{
			TDE2_ASSERT(pActiveCamera);
			return {};
		}

		auto&& ndcBox = pGraphicsContext->GetContextInfo().mNDCBox;

		auto&& invViewProj = Inverse(pGraphicsContext->CalcPerspectiveMatrix(pActiveCamera->GetFOV(), pActiveCamera->GetAspect(), zn, zf) * pActiveCamera->GetViewMatrix());

		std::array<TVector4, 8> frustumVertices
		{
			invViewProj * TVector4(-1.0f,  1.0f, ndcBox.min.z, 1.0f), // near plane of the cube
			invViewProj * TVector4(1.0f,  1.0f, ndcBox.min.z, 1.0f),
			invViewProj * TVector4(-1.0f, -1.0f, ndcBox.min.z, 1.0f),
			invViewProj * TVector4(1.0f, -1.0f, ndcBox.min.z, 1.0f),

			invViewProj * TVector4(-1.0f,  1.0f, 1.0f, 1.0f),
			invViewProj * TVector4(1.0f,  1.0f, 1.0f, 1.0f),
			invViewProj * TVector4(-1.0f, -1.0f, 1.0f, 1.0f),
			invViewProj * TVector4(1.0f, -1.0f, 1.0f, 1.0f),
		};

		for (TVector4& currVertex : frustumVertices)
		{
			currVertex = currVertex * (1.0f / currVertex.w);
		}

		return frustumVertices;
	}


	static TMatrix4 ConstructSunLightMatrix(IGraphicsContext* pGraphicsContext, ICamera* pActiveCamera, CTransform* pLightTransform, F32 zn, F32 zf)
	{
		if (!pLightTransform || !pActiveCamera)
		{
			TDE2_ASSERT(false);
			return IdentityMatrix4;
		}

		const F32 handedness = pGraphicsContext->GetPositiveZAxisDirection();

		auto&& cascadeFrustumCorners = GetCascadeFrustumVertices(pGraphicsContext, dynamic_cast<IPerspectiveCamera*>(pActiveCamera), zn, zf);

		TVector3 frustumCenter;
		{
			for (auto&& v : cascadeFrustumCorners)
			{
				frustumCenter = frustumCenter + v;
			}

			frustumCenter = frustumCenter * 0.125f;
		}

		const TMatrix4 viewMatrix = LookAt(frustumCenter + handedness * Normalize(pLightTransform->GetForwardVector()) /*pLightTransform->GetPosition()*/, UpVector3, frustumCenter, -handedness);

		F32 minX = std::numeric_limits<F32>::max();
		F32 maxX = std::numeric_limits<F32>::lowest();
		F32 minY = std::numeric_limits<F32>::max();
		F32 maxY = std::numeric_limits<F32>::lowest();
		F32 minZ = std::numeric_limits<F32>::max();
		F32 maxZ = std::numeric_limits<F32>::lowest();

		for (const auto& v : cascadeFrustumCorners)
		{
			const auto& lightSpaceVert = viewMatrix * v;

			minX = std::min(minX, lightSpaceVert.x);
			maxX = std::max(maxX, lightSpaceVert.x);
			minY = std::min(minY, lightSpaceVert.y);
			maxY = std::max(maxY, lightSpaceVert.y);
			minZ = std::min(minZ, lightSpaceVert.z);
			maxZ = std::max(maxZ, lightSpaceVert.z);
		}

		constexpr F32 zMult = 10.0f;

		minZ = (minZ < 0) ? minZ * zMult : minZ / zMult;
		maxZ = (maxZ < 0) ? maxZ / zMult : maxZ * zMult;

		TMatrix4 projMatrix = pGraphicsContext->CalcOrthographicMatrix(minX, maxY, maxX, minY, minZ, maxZ); 

		return Transpose(Mul(projMatrix, viewMatrix));
	}


	static std::vector<TMatrix4> CalcSunLightCascadesMatrices(IGraphicsContext* pGraphicsContext, ICamera* pActiveCamera, CTransform* pLightTransform, const std::vector<F32>& cascadesSplits)
	{
		std::vector<TMatrix4> cascadesMats;

		for (U32 cascadeIndex = 0; cascadeIndex < static_cast<U32>(CGameUserSettings::Get()->mpShadowCascadesCountCVar->Get()); cascadeIndex++)
		{
			cascadesMats.emplace_back(ConstructSunLightMatrix(pGraphicsContext, pActiveCamera, pLightTransform, cascadesSplits[cascadeIndex], cascadesSplits[cascadeIndex + 1]));
		}

		return cascadesMats;
	}


	static void ProcessDirectionalLights(IGraphicsContext* pGraphicsContext, ICamera* pCamera, TLightingShaderData& lightingData, CLightingSystem::TDirLightsContext& directionalLightsContext)
	{
		TDE2_PROFILER_SCOPE("CLightingSystem::ProcessDirectionalLights");

		const auto& transforms = std::get<std::vector<CTransform*>>(directionalLightsContext.mComponentsSlice);
		const auto& dirLights = std::get<std::vector<CDirectionalLight*>>(directionalLightsContext.mComponentsSlice);

		TDE2_ASSERT(transforms.size() <= 1); // \note For now only single sun light source is supported

		if (transforms.empty())
		{
			LOG_WARNING("[LightingSystem] There is no a directional light source. Use default sun light's direction");

			lightingData.mIsShadowMappingEnabled = false;
			lightingData.mSunLightDirection      = Normalize(TVector4(-0.5f, -0.5f, 0.0f, 0.0f));
			lightingData.mSunLightPosition       = Normalize(TVector4(0.0f, 10.0f, 0.0f, 1.0f));
			lightingData.mSunLightColor          = TColorUtils::mWhite;
			lightingData.mShadowCascadesCount    = CGameUserSettings::Get()->mpShadowCascadesCountCVar->Get();
			lightingData.mShadowCascadesSplits =
				TVector4
				{
					CGameUserSettings::Get()->mpShadowCascadesSplitsCVar[0]->Get(),
					CGameUserSettings::Get()->mpShadowCascadesSplitsCVar[1]->Get(),
					CGameUserSettings::Get()->mpShadowCascadesSplitsCVar[2]->Get(),
					CGameUserSettings::Get()->mpShadowCascadesSplitsCVar[3]->Get(),
				};

			return;
		}

		CDirectionalLight* pCurrLight = nullptr;
		CTransform* pLightTransform = nullptr;

		auto&& cascadesSplitsPlanes = CalcShadowSplitsPlane(pGraphicsContext, pCamera);

		// \note Prepare lighting data
		for (USIZE i = 0; i < transforms.size(); ++i)
		{
			pLightTransform = transforms[i];
			pCurrLight = dirLights[i];

			lightingData.mSunLightDirection = Normalize(TVector4(pLightTransform->GetForwardVector(), 0.0f)); //TVector4(Normalize(pSunLight->GetDirection()), 0.0f);
			lightingData.mSunLightPosition  = TVector4(pLightTransform->GetPosition(), 1.0f);
			lightingData.mSunLightColor     = pCurrLight->GetColor();

			auto&& sunLightMatrices = CalcSunLightCascadesMatrices(pGraphicsContext, pCamera, pLightTransform, cascadesSplitsPlanes);

			for (USIZE cascadeIndex = 0; cascadeIndex < sunLightMatrices.size(); cascadeIndex++)
			{
				lightingData.mSunLightMatrix[cascadeIndex] = sunLightMatrices[cascadeIndex];
			}

			lightingData.mIsShadowMappingEnabled = static_cast<U32>(CGameUserSettings::Get()->mpIsShadowMappingEnabledCVar->Get());
			lightingData.mShadowCascadesCount    = CGameUserSettings::Get()->mpShadowCascadesCountCVar->Get();
			lightingData.mShadowCascadesSplits   = TVector4(&cascadesSplitsPlanes[1]);
		}
	}


	static TMatrix4 ConstructPointLightMatrix(IGraphicsContext* pGraphicsContext, IPointLight* pPointLight, const TVector3& lightPos, USIZE i)
	{
		const F32 handedness = -pGraphicsContext->GetPositiveZAxisDirection();
		auto&& ndcInfo = pGraphicsContext->GetContextInfo().mNDCBox;

		static const std::array<TVector3, 6> lightUpVectors
		{
			TVector3(0.0f, 1.0f, 0.0f),
			TVector3(0.0f, 1.0f, 0.0f),
			TVector3(0.0f, 0.0f, -1.0f),
			TVector3(0.0f, 0.0f, 1.0f),
			TVector3(0.0f, 1.0f, 0.0f),
			TVector3(0.0f, 1.0f, 0.0f)
		};

		static const std::array<TVector3, 6> lightPosOffsets
		{
			TVector3(1.0f, 0.0f, 0.0f), 
			TVector3(-1.0f, 0.0f, 0.0f),
			TVector3(0.0f, 1.0f, 0.0f),
			TVector3(0.0f, -1.0f, 0.0f),
			TVector3(0.0f, 0.0f, 1.0f),
			TVector3(0.0f, 0.0f, -1.0f)
		};

		return Transpose(Mul(PerspectiveProj(90.0f * CMathConstants::Deg2Rad, 1.0f, 1.0f, pPointLight->GetRange(), ndcInfo.min.z, ndcInfo.max.z, handedness),
			LookAt(lightPos, lightUpVectors[i], lightPos + lightPosOffsets[i], handedness)));
	}


	static void ProcessPointLights(IGraphicsContext* pGraphicsContext, TLightingShaderData& lightingData, std::vector<TLightData>& lightsData, 
		CLightingSystem::TPointLightsContext& pointLightsContext)
	{
		TDE2_PROFILER_SCOPE("CLightingSystem::ProcessPointLights");

		const auto& transforms = std::get<std::vector<CTransform*>>(pointLightsContext.mComponentsSlice);
		const auto& lights = std::get<std::vector<CPointLight*>>(pointLightsContext.mComponentsSlice);

		for (USIZE i = 0; i < transforms.size(); ++i)
		{
			CTransform* pLightTransform = transforms[i];
			CPointLight* pLight = lights[i];

			TLightData currPointLight;
			currPointLight.mLightType = static_cast<I32>(E_LIGHT_SOURCE_TYPE::POINT);
			currPointLight.mPosition = TVector4(pLightTransform->GetPosition(), 1.0f);
			currPointLight.mColor = pLight->GetColor();
			currPointLight.mIntensity = pLight->GetIntensity();
			currPointLight.mRange = pLight->GetRange();

			for (USIZE sideIndex = 0; sideIndex < 6; sideIndex++)
			{
				currPointLight.mLightMatrix[sideIndex] = ConstructPointLightMatrix(pGraphicsContext, pLight, pLightTransform->GetPosition(), sideIndex);
			}

			lightsData.emplace_back(currPointLight);
		}

		lightingData.mActiveLightSourcesCount = std::min<U32>(MaxLightsCount, static_cast<U32>(lightsData.size()));
	}


	static void ProcessSpotLights(IGraphicsContext* pGraphicsContext, TLightingShaderData& lightingData, std::vector<TLightData>& lightsData,
		CLightingSystem::TSpotLightsContext& SpotLightsContext)
	{
		TDE2_PROFILER_SCOPE("CLightingSystem::ProcessSpotLights");

		const auto& transforms = std::get<std::vector<CTransform*>>(SpotLightsContext.mComponentsSlice);
		const auto& lights = std::get<std::vector<CSpotLight*>>(SpotLightsContext.mComponentsSlice);

		for (USIZE i = 0; i < transforms.size(); ++i)
		{
			CTransform* pLightTransform = transforms[i];
			CSpotLight* pLight = lights[i];

			TLightData currSpotLight;
			currSpotLight.mLightType = static_cast<I32>(E_LIGHT_SOURCE_TYPE::SPOT);
			currSpotLight.mPosition = TVector4(pLightTransform->GetPosition(), 1.0f);
			currSpotLight.mDirection = Normalize(TVector4(pLightTransform->GetForwardVector(), 0.0f));
			currSpotLight.mColor = pLight->GetColor();
			currSpotLight.mIntensity = pLight->GetIntensity();
			currSpotLight.mAngle = pLight->GetAngle();
			currSpotLight.mRange = pLight->GetRange();

			lightsData.emplace_back(currSpotLight);
		}

		lightingData.mActiveLightSourcesCount = std::min<U32>(MaxLightsCount, static_cast<U32>(lightsData.size()));
	}


	void CLightingSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CLightingSystem::Update");
		TDE2_ASSERT(mpRenderer);
		
		TLightingShaderData lightingData;
		mActiveLightsData.clear();

		ProcessDirectionalLights(mpGraphicsContext, GetCurrentActiveCamera(pWorld), lightingData, mDirectionalLightsContext);
		ProcessPointLights(mpGraphicsContext, lightingData, mActiveLightsData, mPointLightsContext);
		ProcessSpotLights(mpGraphicsContext, lightingData, mActiveLightsData, mSpotLightsContext);

		if (mpFramePacketsStorage)
		{
			///set up global shader properties for TPerFrameShaderData buffer
			TPerFrameShaderData perFrameShaderData{};
			perFrameShaderData.mLightingData = lightingData;

			ICamera* pMainCamera = GetCurrentActiveCamera(pWorld);
			if (pMainCamera)
			{
				perFrameShaderData.mProjMatrix = Transpose(pMainCamera->GetProjMatrix());
				perFrameShaderData.mViewMatrix = Transpose(pMainCamera->GetViewMatrix());
				perFrameShaderData.mInvProjMatrix = Transpose(Inverse(pMainCamera->GetProjMatrix()));
				perFrameShaderData.mInvViewMatrix = Transpose(Inverse(pMainCamera->GetViewMatrix()));
				perFrameShaderData.mInvViewProjMatrix = Transpose(pMainCamera->GetInverseViewProjMatrix());
				perFrameShaderData.mCameraPosition = TVector4(pMainCamera->GetPosition(), 1.0f);
				perFrameShaderData.mCameraProjectionParams = TVector4(pMainCamera->GetNearPlane(), pMainCamera->GetFarPlane(), 0.0f, 0.0f);
			}

			perFrameShaderData.mTime = TVector4(mpTimer->GetCurrTime(), dt, 0.0f, 0.0f);

			TFramePacket& currFramePacket = mpFramePacketsStorage->GetCurrentFrameForGameLogic();

			currFramePacket.mFrameIndex         = TFrameCounter::mGlobalFrameNumber;
			currFramePacket.mDeltaTime          = dt;
			currFramePacket.mPerFrameData       = perFrameShaderData;
			//currFramePacket.mRareUpdatedData    = perFrameShaderData;
			currFramePacket.mActiveLightSources = mActiveLightsData;
		}

		U32 drawIndex = 0;

		CRenderQueue* pShadowPassRenderQueue = mpFramePacketsStorage->GetCurrentFrameForGameLogic().mpRenderQueues[static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_SHADOW_PASS)].Get();

		// \note Prepare commands for the renderer
		{
			TDE2_PROFILER_SCOPE("CLightingSystem::ProcessShadowCasters");

			for (USIZE i = 0; i < mStaticShadowCastersContext.mComponentsCount; ++i)
			{
				drawIndex = ProcessStaticMeshCasterEntity({ mpResourceManager.Get(), mpShadowVertDecl, mShadowPassMaterialHandle, drawIndex, pShadowPassRenderQueue }, mStaticShadowCastersContext, i);
			}

			for (USIZE i = 0; i < mSkinnedShadowCastersContext.mComponentsCount; ++i)
			{
				drawIndex = ProcessSkinnedMeshCasterEntity({ mpResourceManager.Get(), mpSkinnedShadowVertDecl, mShadowPassSkinnedMaterialHandle, drawIndex, pShadowPassRenderQueue }, mSkinnedShadowCastersContext, i);
			}
		}
	}


	static TMaterialParameters CreateShadowPassMaterialParams(const std::string& shaderId)
	{
		const TMaterialParameters shadowPassMaterialParams
		{
			shaderId, false,
			TDepthStencilStateDesc { true, true, E_COMPARISON_FUNC::LESS_EQUAL},
			TRasterizerStateDesc { E_CULL_MODE::NONE, false, false, 0.0f, 1.0f, false }
		};

		return shadowPassMaterialParams;
	}


	E_RESULT_CODE CLightingSystem::_prepareResources()
	{
		if (auto newVertDeclResult = mpGraphicsObjectManager->CreateVertexDeclaration())
		{
			mpShadowVertDecl = newVertDeclResult.Get();

			mpShadowVertDecl->AddElement({ FT_FLOAT4, 0, VEST_POSITION });
		}

		if (auto newVertDeclResult = mpGraphicsObjectManager->CreateVertexDeclaration())
		{
			mpSkinnedShadowVertDecl = newVertDeclResult.Get();

			mpSkinnedShadowVertDecl->AddElement({ FT_FLOAT4, 0, VEST_POSITION });
			mpSkinnedShadowVertDecl->AddElement({ FT_FLOAT4, 0, VEST_JOINT_WEIGHTS });
			mpSkinnedShadowVertDecl->AddElement({ FT_UINT4, 0, VEST_JOINT_INDICES });
		}

		const static TMaterialParameters shadowPassMaterialParams = CreateShadowPassMaterialParams("Shaders/Default/ShadowPass.shader");
		const static TMaterialParameters shadowPassSkinnedMaterialParams = CreateShadowPassMaterialParams("Shaders/Default/SkinnedShadowPass.shader");

		mShadowPassMaterialHandle        = mpResourceManager->Create<IMaterial>("ShadowPassMaterial.material", shadowPassMaterialParams);
		mShadowPassSkinnedMaterialHandle = mpResourceManager->Create<IMaterial>("ShadowPassSkinnedMaterial.material", shadowPassSkinnedMaterialParams);

		return (mShadowPassMaterialHandle != TResourceId::Invalid && mpShadowVertDecl) ? RC_OK : RC_FAIL;
	}


	TDE2_API ISystem* CreateLightingSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, TPtr<ITimer> pTimer, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CLightingSystem, result, pRenderer, pGraphicsObjectManager, pTimer);
	}
}