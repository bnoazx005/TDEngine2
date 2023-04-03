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
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/CBaseRenderTarget.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/CGameUserSettings.h"
#include "../../include/scene/components/CDirectionalLight.h"
#include "../../include/scene/components/CPointLight.h"
#include "../../include/scene/components/ShadowMappingComponents.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CPerfProfiler.h"


namespace TDEngine2
{
	CLightingSystem::CLightingSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CLightingSystem::Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pRenderer || !pGraphicsObjectManager)
		{
			return RC_INVALID_ARGS;
		}

		mpRenderer              = pRenderer;
		mpGraphicsObjectManager = pGraphicsObjectManager;
		mpResourceManager       = mpRenderer->GetResourceManager();
		mpGraphicsContext       = pGraphicsObjectManager->GetGraphicsContext();

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
				pDrawCommand->mpVertexBuffer = pStaticMeshResource->GetPositionOnlyVertexBuffer();
				pDrawCommand->mpIndexBuffer = pStaticMeshResource->GetSharedIndexBuffer();
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
				pDrawCommand->mpVertexBuffer = pSkinnedMeshResource->GetPositionOnlyVertexBuffer();
				pDrawCommand->mpIndexBuffer = pSkinnedMeshResource->GetSharedIndexBuffer();
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


	static TMatrix4 ConstructSunLightMatrix(IGraphicsContext* pGraphicsContext, ICamera* pActiveCamera, CTransform* pLightTransform)
	{
		if (!pLightTransform || !pActiveCamera)
		{
			TDE2_ASSERT(false);
			return IdentityMatrix4;
		}

		const F32 handedness = pGraphicsContext->GetPositiveZAxisDirection();

		TVector3 frustumCenter;

		for (auto&& v : pActiveCamera->GetFrustum()->GetVertices(pActiveCamera->GetInverseViewProjMatrix(), pGraphicsContext->GetContextInfo().mNDCBox.min.z))
		{
			frustumCenter = frustumCenter + v;
		}

		frustumCenter = frustumCenter * 0.125f;

		//const TVector3& frustumCenter = pActiveCamera->GetFrustumCenter(pGraphicsContext->GetContextInfo().mNDCBox.min.z);
		//LOG_MESSAGE(Wrench::StringUtils::Format("Frustum center: {0}", frustumCenter.ToString()));

		TMatrix4 viewMatrix = LookAt(pActiveCamera->GetPosition() + handedness * pLightTransform->GetPosition(), UpVector3, pActiveCamera->GetPosition(), -handedness);

		const F32 halfSize = 50.0f;

		TMatrix4 projMatrix = pGraphicsContext->CalcOrthographicMatrix(-halfSize, halfSize, halfSize, -halfSize, 0.001f, 1000.0f); // \todo Refactor

		return Transpose(Mul(projMatrix, viewMatrix));
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
			lightingData.mShadowCascadesCount    = 1;

			return;
		}

		CDirectionalLight* pCurrLight = nullptr;
		CTransform* pLightTransform = nullptr;

		// \note Prepare lighting data
		for (USIZE i = 0; i < transforms.size(); ++i)
		{
			pLightTransform = transforms[i];
			pCurrLight = dirLights[i];

			lightingData.mSunLightDirection      = Normalize(TVector4(pLightTransform->GetForwardVector(), 0.0f)); //TVector4(Normalize(pSunLight->GetDirection()), 0.0f);
			lightingData.mSunLightPosition       = TVector4(pLightTransform->GetPosition(), 1.0f);
			lightingData.mSunLightColor          = pCurrLight->GetColor();
			lightingData.mSunLightMatrix[0]      = ConstructSunLightMatrix(pGraphicsContext, pCamera, pLightTransform);
			lightingData.mShadowCascadesCount    = 1;
			lightingData.mIsShadowMappingEnabled = static_cast<U32>(CGameUserSettings::Get()->mCurrent.mIsShadowMappingEnabled);
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


	static void ProcessPointLights(IGraphicsContext* pGraphicsContext, TLightingShaderData& lightingData, CLightingSystem::TPointLightsContext& pointLightsContext)
	{
		TDE2_PROFILER_SCOPE("CLightingSystem::ProcessPointLights");

		const auto& transforms = std::get<std::vector<CTransform*>>(pointLightsContext.mComponentsSlice);
		const auto& lights = std::get<std::vector<CPointLight*>>(pointLightsContext.mComponentsSlice);

		lightingData.mPointLightsCount = std::min<U32>(MaxPointLightsCount, static_cast<U32>(lights.size()));

		auto& pointLights = lightingData.mPointLights;

		for (USIZE i = 0; i < transforms.size(); ++i)
		{
			auto& currPointLight = pointLights[i];

			CTransform* pLightTransform = transforms[i];
			CPointLight* pLight = lights[i];

			currPointLight.mPosition = TVector4(pLightTransform->GetPosition(), 1.0f);
			currPointLight.mColor = pLight->GetColor();
			currPointLight.mIntensity = pLight->GetIntensity();
			currPointLight.mRange = pLight->GetRange();

			for (USIZE sideIndex = 0; sideIndex < 6; sideIndex++)
			{
				currPointLight.mLightMatrix[sideIndex] = ConstructPointLightMatrix(pGraphicsContext, pLight, pLightTransform->GetPosition(), sideIndex);
			}
		}
	}


	template <typename TRenderable>
	static void ProcessShadowReceivers(TPtr<IResourceManager> pResourceManager, const std::vector<TRenderable*> shadowReceivers,
		TPtr<ITexture> pSunLightShadowMap, TPtr<ITexture> pPointLightShadowMap)
	{
		// \note Inject shadow map buffer into materials 
		for (USIZE i = 0; i < shadowReceivers.size(); ++i)
		{
			if (TRenderable* pMeshContainer = shadowReceivers[i])
			{
				if (auto pMaterial = pResourceManager->GetResource<IMaterial>(pResourceManager->Load<IMaterial>(pMeshContainer->GetMaterialName())))
				{
					pMaterial->SetTextureResource("DirectionalShadowMapTexture", pSunLightShadowMap.Get());
					pMaterial->SetTextureResource("PointLightShadowMapTexture_0", pPointLightShadowMap.Get());
				}
			}
		}
	}


	void CLightingSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CLightingSystem::Update");
		TDE2_ASSERT(mpRenderer);
		
		TLightingShaderData lightingData;

		ProcessDirectionalLights(mpGraphicsContext, GetCurrentActiveCamera(pWorld), lightingData, mDirectionalLightsContext);
		ProcessPointLights(mpGraphicsContext, lightingData, mPointLightsContext);

		if (mpRenderer)
		{
			PANIC_ON_FAILURE(mpRenderer->SetLightingData(lightingData));
		}

		U32 drawIndex = 0;

		// \note Prepare commands for the renderer
		{
			TDE2_PROFILER_SCOPE("CLightingSystem::ProcessShadowCasters");

			for (USIZE i = 0; i < mStaticShadowCastersContext.mComponentsCount; ++i)
			{
				drawIndex = ProcessStaticMeshCasterEntity({ mpResourceManager.Get(), mpShadowVertDecl, mShadowPassMaterialHandle, drawIndex, mpShadowPassRenderQueue }, mStaticShadowCastersContext, i);
			}

			for (USIZE i = 0; i < mSkinnedShadowCastersContext.mComponentsCount; ++i)
			{
				drawIndex = ProcessSkinnedMeshCasterEntity({ mpResourceManager.Get(), mpSkinnedShadowVertDecl, mShadowPassSkinnedMaterialHandle, drawIndex, mpShadowPassRenderQueue }, mSkinnedShadowCastersContext, i);
			}
		}

		if (CGameUserSettings::Get()->mCurrent.mIsShadowMappingEnabled)
		{
			TDE2_PROFILER_SCOPE("CLightingSystem::ProcessShadowReceivers");

			auto pShadowMapTexture = mpResourceManager->GetResource<ITexture>(mpResourceManager->Load<IDepthBufferTarget>("ShadowMap"));
			auto pPointLightShadowMapTexture = mpResourceManager->GetResource<ITexture>(mpResourceManager->Load<IDepthBufferTarget>("PointShadowMap"));

			ProcessShadowReceivers(mpResourceManager, std::get<std::vector<CStaticMeshContainer*>>(mStaticShadowReceiversContext.mComponentsSlice), pShadowMapTexture, pPointLightShadowMapTexture);
			ProcessShadowReceivers(mpResourceManager, std::get<std::vector<CSkinnedMeshContainer*>>(mSkinnedShadowReceiversContext.mComponentsSlice), pShadowMapTexture, pPointLightShadowMapTexture);
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

		mpShadowPassRenderQueue = mpRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_SHADOW_PASS);

		const static TMaterialParameters shadowPassMaterialParams = CreateShadowPassMaterialParams("ShadowPass");
		const static TMaterialParameters shadowPassSkinnedMaterialParams = CreateShadowPassMaterialParams("SkinnedShadowPass");

		mShadowPassMaterialHandle        = mpResourceManager->Create<IMaterial>("ShadowPassMaterial.material", shadowPassMaterialParams);
		mShadowPassSkinnedMaterialHandle = mpResourceManager->Create<IMaterial>("ShadowPassSkinnedMaterial.material", shadowPassSkinnedMaterialParams);

		return (mShadowPassMaterialHandle != TResourceId::Invalid && mpShadowPassRenderQueue && mpShadowVertDecl) ? RC_OK : RC_FAIL;
	}


	TDE2_API ISystem* CreateLightingSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CLightingSystem, result, pRenderer, pGraphicsObjectManager);
	}
}