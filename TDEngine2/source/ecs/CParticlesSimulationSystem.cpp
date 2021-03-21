#include "../../include/ecs/CParticlesSimulationSystem.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/CParticleEmitterComponent.h"
#include "../../include/graphics/CParticleEffect.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/utils/CFileLogger.h"
#include <algorithm>
#include <cassert>
#include <cmath>


namespace TDEngine2
{
	static std::vector<IMaterial*> GetUsedMaterials(const std::vector<TEntityId>& entities, IWorld* pWorld, IResourceManager* pResourceManager)
	{
		std::vector<IMaterial*> usedMaterials;

		IMaterial* pCurrMaterial = nullptr;

		for (TEntityId currParticleEntity : entities)
		{
			if (CEntity* pCurrEntity = pWorld->FindEntity(currParticleEntity))
			{
				if (CParticleEmitter* pParticlesEmitter = pCurrEntity->GetComponent<CParticleEmitter>())
				{
					TResourceId particleEffectResourceHandle = pParticlesEmitter->GetParticleEffectHandle();
					if (TResourceId::Invalid == particleEffectResourceHandle)
					{
						particleEffectResourceHandle = pResourceManager->Load<CParticleEffect>(pParticlesEmitter->GetParticleEffectId());
						pParticlesEmitter->SetParticleEffectHandle(particleEffectResourceHandle);
					}

					IParticleEffect* pParticleEffect = pResourceManager->GetResource<IParticleEffect>(particleEffectResourceHandle);

					const TResourceId materialHandle = pResourceManager->Load<CBaseMaterial>(pParticleEffect->GetMaterialName());
					if (TResourceId::Invalid == materialHandle)
					{
						continue;
					}

					pCurrMaterial = pResourceManager->GetResource<IMaterial>(materialHandle);

					// \note skip duplicates
					if (std::find(usedMaterials.cbegin(), usedMaterials.cend(), pCurrMaterial) != usedMaterials.cend())
					{
						continue;
					}

					usedMaterials.push_back(pCurrMaterial);
				}
			}
		}

		return usedMaterials;
	}


	CParticlesSimulationSystem::CParticlesSimulationSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CParticlesSimulationSystem::Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsObjectManager || !pRenderer)
		{
			return RC_INVALID_ARGS;
		}

		mpRenderQueue = pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_TRANSPARENT_GEOMETRY);

		mpGraphicsObjectManager = pGraphicsObjectManager;

		mpResourceManager = pRenderer->GetResourceManager();

		E_RESULT_CODE result = _initInternalVertexData();
		if (RC_OK != result)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CParticlesSimulationSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		--mRefCounter;

		if (!mRefCounter)
		{
			mIsInitialized = false;

			delete this;
		}		

		return RC_OK;
	}

	void CParticlesSimulationSystem::InjectBindings(IWorld* pWorld)
	{
		mParticleEmitters = pWorld->FindEntitiesWithComponents<CParticleEmitter>();

		mParticlesInstancesData.resize(mParticleEmitters.size());

		const auto& cameras = pWorld->FindEntitiesWithAny<CPerspectiveCamera, COrthoCamera>();
		mpCameraEntity = !cameras.empty() ? pWorld->FindEntity(cameras.front()) : nullptr;

		mUsedMaterials = GetUsedMaterials(mParticleEmitters, pWorld, mpResourceManager);
	}

	void CParticlesSimulationSystem::Update(IWorld* pWorld, F32 dt)
	{
		if (!mpCameraEntity)
		{
			LOG_WARNING("[CParticlesSimulationSystem] An entity with Camera component attached to that wasn't found");
			return;
		}

		ICamera* pCameraComponent = GetValidPtrOrDefault<ICamera*>(mpCameraEntity->GetComponent<CPerspectiveCamera>(), mpCameraEntity->GetComponent<COrthoCamera>());
		TDE2_ASSERT(pCameraComponent);

		// \note Process a new step of particles simulation


		// \note Render particles 
		for (IMaterial* pCurrMaterial : mUsedMaterials)
		{
			_populateCommandsBuffer(mParticleEmitters, pWorld, mpRenderQueue, pCurrMaterial, pCameraComponent);
		}
	}

	E_RESULT_CODE CParticlesSimulationSystem::_initInternalVertexData()
	{
		auto createVertDeclResult = mpGraphicsObjectManager->CreateVertexDeclaration();
		if (createVertDeclResult.HasError())
		{
			return createVertDeclResult.GetError();
		}

		mpParticleVertexDeclaration = createVertDeclResult.Get();

		mpParticleVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });
		mpParticleVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT2, 0, TDEngine2::VEST_TEXCOORDS });
		mpParticleVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_COLOR });
		mpParticleVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 1, TDEngine2::VEST_TEXCOORDS, true }); // xyz - position, w - size of a particle
		mpParticleVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 1, TDEngine2::VEST_TEXCOORDS, true }); // xyz - rotation
		mpParticleVertexDeclaration->AddInstancingDivisor(3, 1);

		static const TParticleVertex vertices[] =
		{
			{ TVector4(-0.5f, 0.5f, 0.0f, 1.0f), TVector2(0.0f, 0.0f) },
			{ TVector4(0.5f, 0.5f, 0.0f, 1.0f), TVector2(1.0f, 0.0f) },
			{ TVector4(-0.5f, -0.5f, 0.0f, 1.0f), TVector2(0.0f, 1.0f) },
			{ TVector4(0.5f, -0.5f, 0.0f, 1.0f), TVector2(1.0f, 1.0f) }
		};

		static const U16 faces[] =
		{
			0, 1, 2,
			2, 1, 3
		};

		auto createVertexBufferResult = mpGraphicsObjectManager->CreateVertexBuffer(BUT_STATIC, sizeof(TParticleVertex) * 4, &vertices[0]);
		if (createVertexBufferResult.HasError())
		{
			return createVertexBufferResult.GetError();
		}

		mpParticleQuadVertexBuffer = createVertexBufferResult.Get();

		auto createIndexBufferResult = mpGraphicsObjectManager->CreateIndexBuffer(BUT_STATIC, IFT_INDEX16, sizeof(U16) * 6, faces);
		if (createIndexBufferResult.HasError())
		{
			return createIndexBufferResult.GetError();
		}

		mpParticleQuadIndexBuffer = createIndexBufferResult.Get();

		/// \note Create an additional buffer for instances data
		auto createInstancesVertexBufferResult = mpGraphicsObjectManager->CreateVertexBuffer(BUT_DYNAMIC, SpriteInstanceDataBufferSize, nullptr);
		if (createInstancesVertexBufferResult.HasError())
		{
			return createInstancesVertexBufferResult.GetError();
		}

		mpParticlesInstancesBuffer = createInstancesVertexBufferResult.Get();

		return RC_OK;
	}

	void CParticlesSimulationSystem::_populateCommandsBuffer(const std::vector<TEntityId>& entities, IWorld* pWorld, CRenderQueue*& pRenderGroup, const IMaterial* pCurrMaterial, const ICamera* pCamera)
	{
		auto&& pCastedMaterial = dynamic_cast<const CBaseMaterial*>(pCurrMaterial);
		const std::string& currMaterialName = pCastedMaterial->GetName();

		TResourceId currMaterialId = pCastedMaterial->GetId();

		auto&& viewMatrix = pCamera->GetViewMatrix();

		// \note iterate over all entities with pCurrMaterial attached as main material
		for (TEntityId currEntity : entities)
		{
			if (CEntity* pCurrEntity = pWorld->FindEntity(currEntity))
			{
				if (CParticleEmitter* pParticlesEmitter = pCurrEntity->GetComponent<CParticleEmitter>())
				{
					IParticleEffect* pParticleEffect = mpResourceManager->GetResource<IParticleEffect>(pParticlesEmitter->GetParticleEffectHandle());
					if (!pParticleEffect)
					{
						continue;
					}

					const TResourceId materialHandle = mpResourceManager->Load<CBaseMaterial>(pParticleEffect->GetMaterialName());
					if (TResourceId::Invalid == materialHandle || currMaterialName != pParticleEffect->GetMaterialName())
					{
						continue;
					}

					// \note We've found a particle system which uses pCurrMaterial as a main material, so push command to render it
					CTransform* pTransform = pCurrEntity->GetComponent<CTransform>();

					auto&& objectTransformMatrix = pTransform->GetLocalToWorldTransform();

					const F32 distanceToCamera = ((viewMatrix * objectTransformMatrix) * TVector4(0.0f, 0.0f, 1.0f, 1.0f)).z;

					// \note Create a command for the renderer
					auto pCommand = pRenderGroup->SubmitDrawCommand<TDrawIndexedInstancedCommand>(static_cast<U32>(pCastedMaterial->GetGeometrySubGroupTag()) + 
																								  _computeRenderCommandHash(currMaterialId, distanceToCamera));

					pCommand->mpVertexBuffer              = mpParticleQuadVertexBuffer;
					pCommand->mpIndexBuffer               = mpParticleQuadIndexBuffer;
					pCommand->mpInstancingBuffer          = mpParticlesInstancesBuffer;
					pCommand->mMaterialHandle             = materialHandle;
					pCommand->mpVertexDeclaration         = mpParticleVertexDeclaration; 
					pCommand->mIndicesPerInstance         = 6;
					pCommand->mNumOfInstances             = 1; // \todo Add computation of particles count
					pCommand->mPrimitiveType              = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
					pCommand->mObjectData.mModelMatrix    = Transpose(objectTransformMatrix);
					pCommand->mObjectData.mInvModelMatrix = Transpose(Inverse(objectTransformMatrix));
				}
			}
		}
	}
		
	U32 CParticlesSimulationSystem::_computeRenderCommandHash(TResourceId materialId, F32 distanceToCamera)
	{
		return (static_cast<U32>(materialId) << 16) | static_cast<U16>(fabs(distanceToCamera));
	}


	TDE2_API ISystem* CreateParticlesSimulationSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CParticlesSimulationSystem, result, pRenderer, pGraphicsObjectManager);
	}
}