#include "../../include/ecs/CParticlesSimulationSystem.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/effects/CParticleEmitterComponent.h"
#include "../../include/graphics/effects/CParticleEffect.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/graphics/IVertexBuffer.h"
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
		mParticles.resize(mParticleEmitters.size());
		mpParticlesInstancesBuffers.resize(mParticleEmitters.size());
		mActiveParticlesCount.resize(mParticleEmitters.size());

		const auto& cameras = pWorld->FindEntitiesWithAny<CPerspectiveCamera, COrthoCamera>();
		mpCameraEntity = !cameras.empty() ? pWorld->FindEntity(cameras.front()) : nullptr;

		mUsedMaterials = GetUsedMaterials(mParticleEmitters, pWorld, mpResourceManager);

		for (IVertexBuffer*& pCurrVertexBuffer : mpParticlesInstancesBuffers)
		{
			auto createBufferResult = mpGraphicsObjectManager->CreateVertexBuffer(BUT_DYNAMIC, SpriteInstanceDataBufferSize, nullptr);
			if (createBufferResult.HasError())
			{
				continue;
			}

			pCurrVertexBuffer = createBufferResult.Get();
		}

		for (U32& currCount : mActiveParticlesCount)
		{
			currCount = 0;
		}

		/// \note Initialize arrays
		CEntity* pCurrEntity = nullptr;

		for (U32 i = 0; i < static_cast<U32>(mParticleEmitters.size()); ++i)
		{
			pCurrEntity = pWorld->FindEntity(mParticleEmitters[i]);
			if (!pCurrEntity)
			{
				continue;
			}

			if (CParticleEmitter* pEmitterComponent = pCurrEntity->GetComponent<CParticleEmitter>())
			{
				IParticleEffect* pCurrEffectResource = mpResourceManager->GetResource<IParticleEffect>(pEmitterComponent->GetParticleEffectHandle());
				
				const size_t particlesCount = static_cast<size_t>(pCurrEffectResource->GetMaxParticlesCount());

				mParticlesInstancesData[i].resize(particlesCount);
				mParticles[i].resize(particlesCount);
			}
		}
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
		_simulateParticles(pWorld, dt);

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
		mpParticleVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 1, TDEngine2::VEST_COLOR, true });
		mpParticleVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 1, TDEngine2::VEST_TEXCOORDS, true }); // xyz - position, w - size of a particle
		mpParticleVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 1, TDEngine2::VEST_TEXCOORDS, true }); // xyz - rotation
		mpParticleVertexDeclaration->AddInstancingDivisor(2, 1);

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

		return RC_OK;
	}

	void CParticlesSimulationSystem::_populateCommandsBuffer(const std::vector<TEntityId>& entities, IWorld* pWorld, CRenderQueue*& pRenderGroup, const IMaterial* pCurrMaterial, const ICamera* pCamera)
	{
		auto&& pCastedMaterial = dynamic_cast<const CBaseMaterial*>(pCurrMaterial);
		const std::string& currMaterialName = pCastedMaterial->GetName();

		TResourceId currMaterialId = pCastedMaterial->GetId();

		auto&& viewMatrix = pCamera->GetViewMatrix();

		U32 currBufferIndex = 0;

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

					const bool isWorldSpaceParticles = E_PARTICLE_SIMULATION_SPACE::WORLD == pParticleEffect->GetSimulationSpaceType();

					pCommand->mpVertexBuffer              = mpParticleQuadVertexBuffer;
					pCommand->mpIndexBuffer               = mpParticleQuadIndexBuffer;
					pCommand->mpInstancingBuffer          = mpParticlesInstancesBuffers[currBufferIndex];
					pCommand->mMaterialHandle             = materialHandle;
					pCommand->mpVertexDeclaration         = mpParticleVertexDeclaration; 
					pCommand->mIndicesPerInstance         = 6;
					pCommand->mNumOfInstances             = mActiveParticlesCount[currBufferIndex];
					pCommand->mPrimitiveType              = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
					pCommand->mObjectData.mModelMatrix    = Transpose(isWorldSpaceParticles ? objectTransformMatrix : IdentityMatrix4);
					pCommand->mObjectData.mInvModelMatrix = Transpose(isWorldSpaceParticles ? Inverse(objectTransformMatrix) : IdentityMatrix4);

					++currBufferIndex;
				}
			}
		}
	}


	static U32 GetFirstDeadParticleIndex(const std::vector<TParticleInfo>& particles)
	{
		U32 index = 0;

		for (const TParticleInfo& currParticle : particles)
		{
			if (CMathUtils::IsGreatOrEqual(currParticle.mAge, currParticle.mLifeTime, 1e-3f))
			{
				return index;
			}

			++index;
		}

		return index;
	}


	void CParticlesSimulationSystem::_simulateParticles(IWorld* pWorld, F32 dt)
	{
		U32 currInstancesBufferIndex = 0;

		CEntity* pCurrEntity = nullptr;

		// \note Do main update logic here
		for (U32 i = 0; i < static_cast<U32>(mParticleEmitters.size()); ++i)
		{
			pCurrEntity = pWorld->FindEntity(mParticleEmitters[i]);
			if (!pCurrEntity)
			{
				continue;
			}

			if (CParticleEmitter* pEmitterComponent = pCurrEntity->GetComponent<CParticleEmitter>())
			{
				IParticleEffect* pCurrEffectResource = mpResourceManager->GetResource<IParticleEffect>(pEmitterComponent->GetParticleEffectHandle());

				auto& particles = mParticles[i];

				// \note Process emission
				if ((mActiveParticlesCount[i] < pCurrEffectResource->GetMaxParticlesCount()))
				{
					const U32 emissionRate = pCurrEffectResource->GetEmissionRate();

					if (auto pSharedEmitter = pCurrEffectResource->GetSharedEmitter())
					{
						CTransform* pTransform = pCurrEntity->GetComponent<CTransform>();

						for (U32 k = 0; k < emissionRate; ++k)
						{
							const U32 firstDeadParticleIndex = GetFirstDeadParticleIndex(particles);
							if (firstDeadParticleIndex >= particles.size())
							{
								break; /// \note We're reach out of free particles 
							}

							pSharedEmitter->EmitParticle(pTransform, particles[firstDeadParticleIndex]);
						}
					}
				}

				auto& particlesInstancesBuffer = mParticlesInstancesData[i];

				mActiveParticlesCount[i] = 0;

				const auto modifierFlags = pCurrEffectResource->GetEnabledModifiersFlags();

				auto pSizeCurve = pCurrEffectResource->GetSizeCurve();

				// \note Update existing particles
				for (TParticle& currParticle : particles)
				{
					if (CMathUtils::IsGreatOrEqual(currParticle.mAge, currParticle.mLifeTime, 1e-3f))
					{
						continue;
					}

					++mActiveParticlesCount[i];

					const F32 t = CMathUtils::Clamp01(currParticle.mAge / std::max<F32>(1e-3f, currParticle.mLifeTime));

					// \note Update size over lifetime
					if (E_PARTICLE_EFFECT_INFO_FLAGS::E_SIZE_OVER_LIFETIME_ENABLED == (modifierFlags & E_PARTICLE_EFFECT_INFO_FLAGS::E_SIZE_OVER_LIFETIME_ENABLED))
					{
						currParticle.mSize = pSizeCurve->Sample(t);
					}

					if (E_PARTICLE_EFFECT_INFO_FLAGS::E_COLOR_OVER_LIFETIME_ENABLED == (modifierFlags & E_PARTICLE_EFFECT_INFO_FLAGS::E_COLOR_OVER_LIFETIME_ENABLED))
					{
						currParticle.mColor = CBaseParticlesEmitter::GetColorData(pCurrEffectResource->GetColorOverLifeTime(), t);
					}

					// \note Update velocity over lifetime
					if (E_PARTICLE_EFFECT_INFO_FLAGS::E_VELOCITY_OVER_LIFETIME_ENABLED == (modifierFlags & E_PARTICLE_EFFECT_INFO_FLAGS::E_VELOCITY_OVER_LIFETIME_ENABLED))
					{
						currParticle.mVelocity = CBaseParticlesEmitter::GetVelocityData(pCurrEffectResource->GetVelocityOverTime(), t);
					}

					currParticle.mAge += dt;
					currParticle.mPosition = currParticle.mPosition + dt * currParticle.mVelocity;

					// \todo currParticle.mRotationAngle

					particlesInstancesBuffer[currInstancesBufferIndex].mColor = currParticle.mColor;
					particlesInstancesBuffer[currInstancesBufferIndex].mPositionAndSize = TVector4(currParticle.mPosition, currParticle.mSize.x); // \todo For now use only uniform size

					++currInstancesBufferIndex;
				}

				// \note Copy data into GPU buffers
				if (!particlesInstancesBuffer.empty())
				{
					if (auto pInstancesBuffer = mpParticlesInstancesBuffers[i])
					{
						pInstancesBuffer->Map(E_BUFFER_MAP_TYPE::BMT_WRITE_DISCARD);
						pInstancesBuffer->Write(&particlesInstancesBuffer[0], sizeof(TParticleInstanceData) * mActiveParticlesCount[i]);
						pInstancesBuffer->Unmap();
					}
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