#include "../../include/ecs/CLightingSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/graphics/CStaticMesh.h"
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
#include "../../include/core/CProjectSettings.h"
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
		mDirectionalLightsEntities = pWorld->FindEntitiesWithComponents<CDirectionalLight>();
		mPointLightsEntities       = pWorld->FindEntitiesWithComponents<CPointLight>();

		mShadowCasterEntities      = pWorld->FindEntitiesWithComponents<CShadowCasterComponent>();
		mShadowReceiverEntities    = pWorld->FindEntitiesWithComponents<CShadowReceiverComponent>();
	}


	struct TProcessParams
	{
		CEntity*            mpEntity;
		IResourceManager*   mpResourceManager;
		IVertexDeclaration* mpVertexDeclaration;
		TResourceId         mMaterialId;
		U32                 mDrawIndex;
		CRenderQueue*       mpRenderQueue;
	};


	static U32 ProcessStaticMeshCasterEntity(const TProcessParams& params)
	{
		IResourceManager* pResourceManager = params.mpResourceManager;
		CEntity* pEntity = params.mpEntity;

		CStaticMeshContainer* pStaticMeshContainer = pEntity->GetComponent<CStaticMeshContainer>();
		if (!pStaticMeshContainer)
		{
			return params.mDrawIndex;
		}

		CTransform* pTransform = pEntity->GetComponent<CTransform>();

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

			if (TDrawIndexedCommand* pDrawCommand = params.mpRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(params.mDrawIndex))
			{
				pDrawCommand->mpVertexBuffer = pStaticMeshResource->GetPositionOnlyVertexBuffer();
				pDrawCommand->mpIndexBuffer = pStaticMeshResource->GetSharedIndexBuffer();
				pDrawCommand->mMaterialHandle = params.mMaterialId;
				pDrawCommand->mPrimitiveType = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
				pDrawCommand->mpVertexDeclaration = params.mpVertexDeclaration;
				pDrawCommand->mObjectData.mModelMatrix = Transpose(pTransform->GetLocalToWorldTransform());
				pDrawCommand->mObjectData.mObjectID = static_cast<U32>(pEntity->GetId());
				pDrawCommand->mStartIndex = 0;
				pDrawCommand->mStartVertex = 0;
				pDrawCommand->mNumOfIndices = pStaticMeshResource->GetFacesCount() * 3;
			}
		}

		return params.mDrawIndex + 1;
	}

	static U32 ProcessSkinnedMeshCasterEntity(const TProcessParams& params)
	{
		IResourceManager* pResourceManager = params.mpResourceManager;
		CEntity* pEntity = params.mpEntity;

		CSkinnedMeshContainer* pSkinnedMeshContainer = pEntity->GetComponent<CSkinnedMeshContainer>();
		if (!pSkinnedMeshContainer)
		{
			return params.mDrawIndex;
		}

		CTransform* pTransform = pEntity->GetComponent<CTransform>();

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

			if (TDrawIndexedCommand* pDrawCommand = params.mpRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(params.mDrawIndex))
			{
				pDrawCommand->mpVertexBuffer = pSkinnedMeshResource->GetPositionOnlyVertexBuffer();
				pDrawCommand->mpIndexBuffer = pSkinnedMeshResource->GetSharedIndexBuffer();
				pDrawCommand->mMaterialHandle = params.mMaterialId;
				pDrawCommand->mPrimitiveType = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
				pDrawCommand->mpVertexDeclaration = params.mpVertexDeclaration;
				pDrawCommand->mObjectData.mModelMatrix = Transpose(pTransform->GetLocalToWorldTransform());
				pDrawCommand->mObjectData.mObjectID = static_cast<U32>(pEntity->GetId());
				pDrawCommand->mStartIndex = 0;
				pDrawCommand->mStartVertex = 0;
				pDrawCommand->mNumOfIndices = static_cast<U32>(pSkinnedMeshResource->GetIndices().size());
			}
		}

		return params.mDrawIndex + 1;
	}


	void CLightingSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CLightingSystem::Update");

		TDE2_ASSERT(mDirectionalLightsEntities.size() <= 1); // \note For now only single sun light source is supported
		TDE2_ASSERT(mpRenderer);

		if (mDirectionalLightsEntities.empty())
		{
			return;
		}

		TLightingShaderData lightingData;

		// \note Prepare lighting data
		for (TEntityId currEntity : mDirectionalLightsEntities)
		{
			if (auto pEntity = pWorld->FindEntity(currEntity))
			{
				if (auto pSunLight = pEntity->GetComponent<CDirectionalLight>())
				{
					CTransform* pLightTransform = pEntity->GetComponent<CTransform>();

					lightingData.mSunLightDirection      = Normalize(TVector4(pLightTransform->GetForwardVector(), 0.0f)); //TVector4(Normalize(pSunLight->GetDirection()), 0.0f);
					lightingData.mSunLightPosition       = TVector4(pLightTransform->GetPosition(), 1.0f);
					lightingData.mSunLightColor          = pSunLight->GetColor();
					lightingData.mSunLightMatrix         = _constructSunLightMatrix(pEntity);
					lightingData.mIsShadowMappingEnabled = static_cast<U32>(CProjectSettings::Get()->mGraphicsSettings.mRendererSettings.mIsShadowMappingEnabled);
				}
			}
		}

		_processPointLights(pWorld, lightingData);

		if (mpRenderer)
		{
			PANIC_ON_FAILURE(mpRenderer->SetLightingData(lightingData));
		}

		U32 drawIndex = 0;

		// \note Prepare commands for the renderer
		for (TEntityId currEntity : mShadowCasterEntities)
		{
			if (auto pEntity = pWorld->FindEntity(currEntity))
			{
				drawIndex = ProcessStaticMeshCasterEntity({ pEntity, mpResourceManager.Get(), mpShadowVertDecl, mShadowPassMaterialHandle, drawIndex, mpShadowPassRenderQueue });
				drawIndex = ProcessSkinnedMeshCasterEntity({ pEntity, mpResourceManager.Get(), mpSkinnedShadowVertDecl, mShadowPassSkinnedMaterialHandle, drawIndex, mpShadowPassRenderQueue });
			}
		}

		if (CProjectSettings::Get()->mGraphicsSettings.mRendererSettings.mIsShadowMappingEnabled)
		{
			auto pShadowMapTexture = mpResourceManager->GetResource<ITexture>(mpResourceManager->Load<IDepthBufferTarget>("ShadowMap"));

			// \note Inject shadow map buffer into materials 
			for (TEntityId currEntity : mShadowReceiverEntities)
			{
				if (auto pEntity = pWorld->FindEntity(currEntity))
				{
					if (CStaticMeshContainer* pStaticMeshContainer = pEntity->GetComponent<CStaticMeshContainer>())
					{
						if (auto pMaterial = mpResourceManager->GetResource<IMaterial>(mpResourceManager->Load<IMaterial>(pStaticMeshContainer->GetMaterialName())))
						{
							pMaterial->SetTextureResource("DirectionalShadowMapTexture", pShadowMapTexture.Get());
						}
					}
				}
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

		mpShadowPassRenderQueue = mpRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_SHADOW_PASS);

		const static TMaterialParameters shadowPassMaterialParams = CreateShadowPassMaterialParams("ShadowPass");
		const static TMaterialParameters shadowPassSkinnedMaterialParams = CreateShadowPassMaterialParams("SkinnedShadowPass");

		mShadowPassMaterialHandle        = mpResourceManager->Create<IMaterial>("ShadowPassMaterial.material", shadowPassMaterialParams);
		mShadowPassSkinnedMaterialHandle = mpResourceManager->Create<IMaterial>("ShadowPassSkinnedMaterial.material", shadowPassSkinnedMaterialParams);

		return (mShadowPassMaterialHandle != TResourceId::Invalid && mpShadowPassRenderQueue && mpShadowVertDecl) ? RC_OK : RC_FAIL;
	}

	TMatrix4 CLightingSystem::_constructSunLightMatrix(CEntity* pEntity) const
	{
		CTransform* pTransform = pEntity->GetComponent<CTransform>();
		if (!pTransform)
		{
			TDE2_ASSERT(false);
			return IdentityMatrix4;
		}

		const F32 handedness = mpGraphicsContext->GetPositiveZAxisDirection();

		TMatrix4 viewMatrix = LookAt(handedness * pTransform->GetPosition(), UpVector3, ZeroVector3, -handedness);
		
		const F32 halfSize = 10.0f;

		TMatrix4 projMatrix = mpGraphicsContext->CalcOrthographicMatrix(-halfSize, halfSize, halfSize, -halfSize, 0.001f, 1000.0f); // \todo Refactor

		return Transpose(Mul(projMatrix, viewMatrix));
	}

	void CLightingSystem::_processPointLights(IWorld* pWorld, TLightingShaderData& lightingData)
	{
		CEntity* pEntity = nullptr;

		lightingData.mPointLightsCount = std::min<U32>(MaxPointLightsCount, static_cast<U32>(mPointLightsEntities.size()));

		auto& pointLights = lightingData.mPointLights;

		for (U32 i = 0; i < static_cast<U32>(mPointLightsEntities.size()); ++i)
		{
			pEntity = pWorld->FindEntity(mPointLightsEntities[i]);
			if (!pEntity)
			{
				continue;
			}

			auto& currPointLight = pointLights[i];

			if (auto pLight = pEntity->GetComponent<CPointLight>())
			{
				CTransform* pLightTransform = pEntity->GetComponent<CTransform>();

				currPointLight.mPosition  = TVector4(pLightTransform->GetPosition(), 1.0f);
				currPointLight.mColor     = pLight->GetColor();
				currPointLight.mIntensity = pLight->GetIntensity();
				currPointLight.mRange     = pLight->GetRange();
			}
		}
	}


	TDE2_API ISystem* CreateLightingSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CLightingSystem, result, pRenderer, pGraphicsObjectManager);
	}
}