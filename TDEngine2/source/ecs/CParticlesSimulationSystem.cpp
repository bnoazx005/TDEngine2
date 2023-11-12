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
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CPerfProfiler.h"
#include <algorithm>
#include <cassert>
#include <cmath>


namespace TDEngine2
{
	static std::vector<TPtr<IMaterial>> GetUsedMaterials(const std::vector<TEntityId>& entities, IWorld* pWorld, IResourceManager* pResourceManager)
	{
		std::vector<TPtr<IMaterial>> usedMaterials;

		TPtr<IMaterial> pCurrMaterial;

		for (TEntityId currParticleEntity : entities)
		{
			if (CEntity* pCurrEntity = pWorld->FindEntity(currParticleEntity))
			{
				if (CParticleEmitter* pParticlesEmitter = pCurrEntity->GetComponent<CParticleEmitter>())
				{
					TResourceId particleEffectResourceHandle = pParticlesEmitter->GetParticleEffectHandle();
					if (TResourceId::Invalid == particleEffectResourceHandle)
					{
						particleEffectResourceHandle = pResourceManager->Load<IParticleEffect>(pParticlesEmitter->GetParticleEffectId());
						pParticlesEmitter->SetParticleEffectHandle(particleEffectResourceHandle);
					}

					TPtr<IParticleEffect> pParticleEffect = pResourceManager->GetResource<IParticleEffect>(particleEffectResourceHandle);

					const TResourceId materialHandle = pResourceManager->Load<IMaterial>(pParticleEffect->GetMaterialName());
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
		TDE2_PROFILER_SCOPE("CParticlesSimulationSystem::Init");

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

	void CParticlesSimulationSystem::InjectBindings(IWorld* pWorld)
	{
		auto&& entities = pWorld->FindEntitiesWithComponents<CParticleEmitter>();
		
		auto& particleEmitters = mParticleEmitters.mpParticleEmitters;
		auto& transforms = mParticleEmitters.mpTransform;

		particleEmitters.clear();
		transforms.clear();

		for (auto currEntityId : entities)
		{
			if (auto pEntity = pWorld->FindEntity(currEntityId))
			{
				particleEmitters.push_back(pEntity->GetComponent<CParticleEmitter>());
				transforms.push_back(pEntity->GetComponent<CTransform>());
			}
		}

		mParticlesInstancesData.resize(particleEmitters.size());
		mParticles.resize(particleEmitters.size());
		mParticlesInstancesBufferHandles.resize(particleEmitters.size());
		mActiveParticlesCount.resize(particleEmitters.size());

		const auto& cameras = pWorld->FindEntitiesWithAny<CPerspectiveCamera, COrthoCamera>();
		mpCameraEntity = !cameras.empty() ? pWorld->FindEntity(cameras.front()) : nullptr;

		mUsedMaterials = GetUsedMaterials(entities, pWorld, mpResourceManager.Get());

		for (TBufferHandleId& currVertexBufferHandle : mParticlesInstancesBufferHandles)
		{
			auto createBufferResult = mpGraphicsObjectManager->CreateBuffer({ E_BUFFER_USAGE_TYPE::DYNAMIC, E_BUFFER_TYPE::VERTEX, SpriteInstanceDataBufferSize, nullptr });
			if (createBufferResult.HasError())
			{
				continue;
			}

			currVertexBufferHandle = createBufferResult.Get();
		}

		for (U32& currCount : mActiveParticlesCount)
		{
			currCount = 0;
		}

		/// \note Initialize arrays
		for (USIZE i = 0; i < particleEmitters.size(); ++i)
		{
			if (CParticleEmitter* pEmitterComponent = particleEmitters[i])
			{
				auto pCurrEffectResource = mpResourceManager->GetResource<IParticleEffect>(pEmitterComponent->GetParticleEffectHandle());
				
				const size_t particlesCount = static_cast<size_t>(pCurrEffectResource->GetMaxParticlesCount());

				mParticlesInstancesData[i].resize(particlesCount);
				mParticles[i].resize(particlesCount);
			}
		}
	}

	void CParticlesSimulationSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CParticlesSimulationSystem::Update");

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
		for (auto&& pCurrMaterial : mUsedMaterials)
		{
			_populateCommandsBuffer(mParticleEmitters, mpRenderQueue, pCurrMaterial.Get(), pCameraComponent);
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

		auto createVertexBufferResult = mpGraphicsObjectManager->CreateBuffer({ E_BUFFER_USAGE_TYPE::STATIC, E_BUFFER_TYPE::VERTEX, sizeof(TParticleVertex) * 4, &vertices[0] });
		if (createVertexBufferResult.HasError())
		{
			return createVertexBufferResult.GetError();
		}

		mParticleQuadVertexBufferHandle = createVertexBufferResult.Get();

		auto createIndexBufferResult = mpGraphicsObjectManager->CreateBuffer({ E_BUFFER_USAGE_TYPE::STATIC, E_BUFFER_TYPE::INDEX, sizeof(U16) * 6, faces });
		if (createIndexBufferResult.HasError())
		{
			return createIndexBufferResult.GetError();
		}

		mParticleQuadIndexBufferHandle = createIndexBufferResult.Get();

		return RC_OK;
	}

	void CParticlesSimulationSystem::_populateCommandsBuffer(TSystemContext& context, CRenderQueue*& pRenderGroup, const IMaterial* pCurrMaterial, const ICamera* pCamera)
	{
		auto&& pCastedMaterial = dynamic_cast<const CBaseMaterial*>(pCurrMaterial);
		const std::string& currMaterialName = pCastedMaterial->GetName();

		TResourceId currMaterialId = pCastedMaterial->GetId();

		auto&& viewMatrix = pCamera->GetViewMatrix();

		U32 currBufferIndex = 0;

		// \note iterate over all entities with pCurrMaterial attached as main material
		for (USIZE i = 0; i < context.mpParticleEmitters.size(); ++i)
		{
			CParticleEmitter* pParticlesEmitter = context.mpParticleEmitters[i];

			auto pParticleEffect = mpResourceManager->GetResource<IParticleEffect>(pParticlesEmitter->GetParticleEffectHandle());
			if (!pParticleEffect)
			{
				continue;
			}

			const TResourceId materialHandle = mpResourceManager->Load<IMaterial>(pParticleEffect->GetMaterialName());
			if (TResourceId::Invalid == materialHandle || currMaterialName != pParticleEffect->GetMaterialName())
			{
				continue;
			}

			// \note We've found a particle system which uses pCurrMaterial as a main material, so push command to render it
			CTransform* pTransform = context.mpTransform[i];

			auto&& objectTransformMatrix = pTransform->GetLocalToWorldTransform();

			const F32 distanceToCamera = ((viewMatrix * objectTransformMatrix) * TVector4(0.0f, 0.0f, 1.0f, 1.0f)).z;

			// \note Create a command for the renderer
			auto pCommand = pRenderGroup->SubmitDrawCommand<TDrawIndexedInstancedCommand>(static_cast<U32>(pCastedMaterial->GetGeometrySubGroupTag()) +
				_computeRenderCommandHash(currMaterialId, distanceToCamera));

			const bool isLocalSpaceParticles = E_PARTICLE_SIMULATION_SPACE::LOCAL == pParticleEffect->GetSimulationSpaceType();

			pCommand->mVertexBufferHandle = mParticleQuadVertexBufferHandle;
			pCommand->mIndexBufferHandle = mParticleQuadIndexBufferHandle;
			pCommand->mInstancingBufferHandle = mParticlesInstancesBufferHandles[currBufferIndex];
			pCommand->mMaterialHandle = materialHandle;
			pCommand->mpVertexDeclaration = mpParticleVertexDeclaration;
			pCommand->mIndicesPerInstance = 6;
			pCommand->mNumOfInstances = mActiveParticlesCount[currBufferIndex];
			pCommand->mPrimitiveType = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
			pCommand->mObjectData.mModelMatrix = Transpose(isLocalSpaceParticles ? objectTransformMatrix : IdentityMatrix4);
			pCommand->mObjectData.mInvModelMatrix = Transpose(isLocalSpaceParticles ? Inverse(objectTransformMatrix) : IdentityMatrix4);

			++currBufferIndex;
		}
	}


	static U32 GetFirstDeadParticleIndex(std::vector<TParticleInfo>& particles, bool isLoopedModeEnabled)
	{
		U32 index = 0;

		for (TParticleInfo& currParticle : particles)
		{
			if (CMathUtils::IsGreatOrEqual(currParticle.mAge, currParticle.mLifeTime, 1e-3f) && (isLoopedModeEnabled || (!isLoopedModeEnabled && !currParticle.mHasBeenUsed)))
			{
				currParticle.mHasBeenUsed = true;
				return index;
			}

			++index;
		}

		return index;
	}


	void CParticlesSimulationSystem::_simulateParticles(IWorld* pWorld, F32 dt)
	{
		U32 currInstancesBufferIndex = 0;

		// \note Do main update logic here
		for (USIZE i = 0; i < mParticleEmitters.mpParticleEmitters.size(); ++i)
		{
			if (CParticleEmitter* pEmitterComponent = mParticleEmitters.mpParticleEmitters[i])
			{
				auto pCurrEffectResource = mpResourceManager->GetResource<IParticleEffect>(pEmitterComponent->GetParticleEffectHandle());

				auto& particles = mParticles[i];

#if TDE2_EDITORS_ENABLED
				if (pEmitterComponent->mResetStateOnNextFrame)
				{
					std::fill(particles.begin(), particles.end(), TParticleInfo {});
					pEmitterComponent->mResetStateOnNextFrame = false;
				}
#endif

				// \note Process emission
				if ((mActiveParticlesCount[i] < pCurrEffectResource->GetMaxParticlesCount()))
				{
					const U32 emissionRate = pCurrEffectResource->GetEmissionRate();

					if (auto pSharedEmitter = pCurrEffectResource->GetSharedEmitter())
					{
						CTransform* pTransform = mParticleEmitters.mpTransform[i];

						for (U32 k = 0; k < emissionRate; ++k)
						{
							const U32 firstDeadParticleIndex = GetFirstDeadParticleIndex(particles, pCurrEffectResource->IsLoopModeActive());
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

					// \note Apply gravity
					if (E_PARTICLE_EFFECT_INFO_FLAGS::E_GRAVITY_FORCE_ENABLED == (modifierFlags & E_PARTICLE_EFFECT_INFO_FLAGS::E_GRAVITY_FORCE_ENABLED))
					{
						currParticle.mVelocity = currParticle.mVelocity + UpVector3 * -pCurrEffectResource->GetGravityModifier();
					}

					// \note Apply force
					if (E_PARTICLE_EFFECT_INFO_FLAGS::E_FORCE_OVER_LIFETIME_ENABLED == (modifierFlags & E_PARTICLE_EFFECT_INFO_FLAGS::E_FORCE_OVER_LIFETIME_ENABLED))
					{
						currParticle.mVelocity = currParticle.mVelocity + pCurrEffectResource->GetForceOverTime();
					}

					currParticle.mAge += dt;
					currParticle.mPosition = currParticle.mPosition + dt * currParticle.mVelocity;

					/// \note Update rotation over lifetime
					if (E_PARTICLE_EFFECT_INFO_FLAGS::E_ROTATION_OVER_LIFETIME_ENABLED == (modifierFlags & E_PARTICLE_EFFECT_INFO_FLAGS::E_ROTATION_OVER_LIFETIME_ENABLED))
					{
						currParticle.mRotation += dt * pCurrEffectResource->GetRotationOverTime(); /// \note mRotation is computed in degrees
					}

					particlesInstancesBuffer[currInstancesBufferIndex].mColor = currParticle.mColor;
					particlesInstancesBuffer[currInstancesBufferIndex].mPositionAndSize = TVector4(currParticle.mPosition, currParticle.mSize.x); // \todo For now use only uniform size
					particlesInstancesBuffer[currInstancesBufferIndex].mRotation = TVector4(CMathConstants::Deg2Rad * currParticle.mRotation, 0.0f, 0.0f, 0.0f);

					++currInstancesBufferIndex;
				}

				// \note Copy data into GPU buffers
				if (!particlesInstancesBuffer.empty())
				{
					if (auto pInstancesBuffer = mpGraphicsObjectManager->GetBufferPtr(mParticlesInstancesBufferHandles[i]))
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