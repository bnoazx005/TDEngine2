#include "./../../include/ecs/CLightingSystem.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/ecs/CTransform.h"
#include "./../../include/graphics/CStaticMesh.h"
#include "./../../include/graphics/CStaticMeshContainer.h"
#include "./../../include/graphics/IRenderer.h"
#include "./../../include/graphics/InternalShaderData.h"
#include "./../../include/graphics/IGraphicsObjectManager.h"
#include "./../../include/graphics/IVertexDeclaration.h"
#include "./../../include/graphics/CRenderQueue.h"
#include "./../../include/graphics/CBaseMaterial.h"
#include "./../../include/graphics/CBaseRenderTarget.h"
#include "./../../include/core/IResourceManager.h"
#include "./../../include/core/IResourceHandler.h"
#include "./../../include/core/IGraphicsContext.h"
#include "./../../include/scene/components/CDirectionalLight.h"
#include "./../../include/scene/components/ShadowMappingComponents.h"
#include "./../../include/utils/CFileLogger.h"
#include "./../../include/utils/CResult.h"


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

	E_RESULT_CODE CLightingSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CLightingSystem::InjectBindings(IWorld* pWorld)
	{
		mDirectionalLightsEntities = pWorld->FindEntitiesWithComponents<CDirectionalLight>();
		mShadowCasterEntities      = pWorld->FindEntitiesWithComponents<CShadowCasterComponent>();
		mShadowReceiverEntities    = pWorld->FindEntitiesWithComponents<CShadowReceiverComponent>();
	}

	void CLightingSystem::Update(IWorld* pWorld, F32 dt)
	{
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
					lightingData.mSunLightDirection = TVector4(Normalize(pSunLight->GetDirection()), 0.0f);
					lightingData.mSunLightColor     = pSunLight->GetColor();
					lightingData.mSunLightMatrix    = _constructSunLightMatrix(pEntity);
				}
			}
		}

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
				if (CStaticMeshContainer* pStaticMeshContainer = pEntity->GetComponent<CStaticMeshContainer>())
				{
					CTransform* pTransform = pEntity->GetComponent<CTransform>();

					IResourceHandler* pMeshResourceHandler = mpResourceManager->Load<CStaticMesh>(pStaticMeshContainer->GetMeshName());
					TDE2_ASSERT(pMeshResourceHandler->IsValid());

					if (IStaticMesh* pStaticMeshResource = pMeshResourceHandler->Get<IStaticMesh>(RAT_BLOCKING))
					{
						if (TDrawIndexedCommand* pDrawCommand = mpShadowPassRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(drawIndex++))
						{
							pDrawCommand->mpVertexBuffer           = pStaticMeshResource->GetPositionOnlyVertexBuffer();
							pDrawCommand->mpIndexBuffer            = pStaticMeshResource->GetSharedIndexBuffer();
							pDrawCommand->mpMaterialHandler        = mpShadowPassMaterial;
							pDrawCommand->mPrimitiveType           = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
							pDrawCommand->mpVertexDeclaration      = mpShadowVertDecl;
							pDrawCommand->mObjectData.mModelMatrix = Transpose(pTransform->GetTransform());
							pDrawCommand->mObjectData.mObjectID    = static_cast<U32>(pEntity->GetId());
							pDrawCommand->mStartIndex              = 0;
							pDrawCommand->mStartVertex             = 0;
							pDrawCommand->mNumOfIndices            = pStaticMeshResource->GetFacesCount() * 3;
						}
					}
				}
			}
		}

		auto pShadowMapTexture = mpResourceManager->Load<CBaseDepthBufferTarget>("ShadowMap")->Get<IDepthBufferTarget>(RAT_BLOCKING);

		// \note Inject shadow map buffer into materials 
		for (TEntityId currEntity : mShadowReceiverEntities)
		{
			if (auto pEntity = pWorld->FindEntity(currEntity))
			{
				if (CStaticMeshContainer* pStaticMeshContainer = pEntity->GetComponent<CStaticMeshContainer>())
				{
					if (auto pMaterial = mpResourceManager->Load<CBaseMaterial>(pStaticMeshContainer->GetMaterialName())->Get<IMaterial>(RAT_BLOCKING))
					{
						pMaterial->SetTextureResource("ShadowMapTexture", pShadowMapTexture);
					}
				}
			}
		}
	}

	E_RESULT_CODE CLightingSystem::_prepareResources()
	{
		if (auto newVertDeclResult = mpGraphicsObjectManager->CreateVertexDeclaration())
		{
			mpShadowVertDecl = newVertDeclResult.Get();
			mpShadowVertDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });
		}

		mpShadowPassRenderQueue = mpRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_SHADOW_PASS);

		const static TMaterialParameters shadowPassMaterialParams
		{
			"ShadowPass", false,
			{ true, true, E_COMPARISON_FUNC::LESS_EQUAL},
			{ E_CULL_MODE::NONE, false, false, 0.0f, 1.0f, false }
		};

		mpShadowPassMaterial = mpResourceManager->Create<CBaseMaterial>("ShadowPassMaterial.material", shadowPassMaterialParams);

		return (mpShadowPassMaterial->IsValid() && mpShadowPassRenderQueue && mpShadowVertDecl) ? RC_OK : RC_FAIL;
	}

	TMatrix4 CLightingSystem::_constructSunLightMatrix(CEntity* pEntity) const
	{
		CTransform* pTransform = pEntity->GetComponent<CTransform>();
		if (!pTransform)
		{
			TDE2_ASSERT(false);
			return IdentityMatrix4;
		}

		TMatrix4 viewMatrix = pTransform->GetTransform();
		{
			viewMatrix.m[0][3] = -viewMatrix.m[0][3];
			viewMatrix.m[1][3] = -viewMatrix.m[1][3];

			const F32 zAxis = mpGraphicsContext->GetPositiveZAxisDirection();

			// \note This thing is a kind of a hack for OpenGL graphics context which is using orthographic projection to make it uniform for both GAPIs
			viewMatrix.m[2][3] *= -zAxis;
			viewMatrix.m[2][2] *= zAxis;
		}

		TMatrix4 projMatrix = mpGraphicsContext->CalcOrthographicMatrix(-10.0f, 10.0f, 10.0f, -10.0f, 1.0f, 7.0f); // \todo Refactor

		return Transpose(Mul(projMatrix, viewMatrix));
	}


	TDE2_API ISystem* CreateLightingSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		CLightingSystem* pSystemInstance = new (std::nothrow) CLightingSystem();

		if (!pSystemInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSystemInstance->Init(pRenderer, pGraphicsObjectManager);

		if (result != RC_OK)
		{
			delete pSystemInstance;

			pSystemInstance = nullptr;
		}

		return dynamic_cast<ISystem*>(pSystemInstance);
	}
}