#include "../../include/ecs/CParticlesSimulationSystem.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/effects/CParticleEmitterComponent.h"
#include "../../include/graphics/effects/CParticleEffect.h"
#include "../../include/graphics/effects/TParticle.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/CProjectSettings.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/graphics/IShader.h"
#include "../../include/graphics/ITexture2D.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/core/CGameUserSettings.h"
#include "../../include/editor/CPerfProfiler.h"
#include <algorithm>
#include <cmath>
#include <vector>


namespace TDEngine2
{
#if TDE2_EDITORS_ENABLED
	CFloatConsoleVarDecl SimulationTimeCoeffCfgVar(SIMULATION_TIME_COEFF_CFG_VAR_ID, "", 1.0f);
#endif

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
					if (!pParticleEffect)
					{
						continue;
					}

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


	/*!
		class CParticlesCPUSimulationSystem

		\brief The class is a system that processes CParticleEmitter components on CPU
	*/

	class CParticlesCPUSimulationSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateParticlesSimulationSystem(IRenderer*, IGraphicsObjectManager*, E_RESULT_CODE&);

		private:
			typedef struct TParticleVertex
			{
				TVector4 mPosition;
				TVector2 mUVs;
			} TParticleVertex, * TParticleVertexPtr;

			typedef struct TParticleInstanceData
			{
				TColor32F mColor;
				TVector4 mPositionAndSize;
				TVector4 mRotation;
			} TParticleInstanceData, * TParticleInstanceDataPtr;

			typedef std::vector<std::vector<TParticleInstanceData>> TParticlesArray;
			typedef std::vector<std::vector<TParticleInfo>> TParticlesInfoArray;

			typedef std::vector<CParticleEmitter*> TParticleEmmitters;

			struct TSystemContext
			{
				std::vector<CTransform*>       mpTransform;
				std::vector<CParticleEmitter*> mpParticleEmitters;
			};
		public:
			TDE2_SYSTEM(CParticlesCPUSimulationSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pRenderer A pointer to IRenderer implementation

				\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager)
			{
				TDE2_PROFILER_SCOPE("CParticlesCPUSimulationSystem::Init");

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

			/*!
				\brief The method inject components array into a system

				\param[in] pWorld A pointer to a main scene's object
			*/

			TDE2_API void InjectBindings(IWorld* pWorld) override
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

						const size_t particlesCount = pCurrEffectResource ? static_cast<size_t>(pCurrEffectResource->GetMaxParticlesCount()) : 0;

						mParticlesInstancesData[i].resize(particlesCount);
						mParticles[i].resize(particlesCount);
					}
				}
			}

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] pWorld A pointer to a main scene's object

				\param[in] dt A delta time's value
			*/

			TDE2_API void Update(IWorld* pWorld, F32 dt) override
			{
				TDE2_PROFILER_SCOPE("CParticlesCPUSimulationSystem::Update");

				if (!mpCameraEntity)
				{
					LOG_WARNING("[CParticlesCPUSimulationSystem] An entity with Camera component attached to that wasn't found");
					return;
				}

				ICamera* pCameraComponent = GetValidPtrOrDefault<ICamera*>(mpCameraEntity->GetComponent<CPerspectiveCamera>(), mpCameraEntity->GetComponent<COrthoCamera>());
				TDE2_ASSERT(pCameraComponent);

				// \note Process a new step of particles simulation
				_simulateParticles(pWorld, 
#if TDE2_EDITORS_ENABLED
					SimulationTimeCoeffCfgVar.Get() * 
#endif
					dt
				);

				// \note Render particles 
				for (auto&& pCurrMaterial : mUsedMaterials)
				{
					_populateCommandsBuffer(mParticleEmitters, mpRenderQueue, pCurrMaterial.Get(), pCameraComponent);
				}
			}
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CParticlesCPUSimulationSystem)

			E_RESULT_CODE _initInternalVertexData()
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

				static const U16 faces[] =
				{
					0, 1, 2,
					2, 1, 3
				};

				auto createIndexBufferResult = mpGraphicsObjectManager->CreateBuffer({ E_BUFFER_USAGE_TYPE::STATIC, E_BUFFER_TYPE::INDEX, sizeof(U16) * 6, faces });
				if (createIndexBufferResult.HasError())
				{
					return createIndexBufferResult.GetError();
				}

				mParticleQuadIndexBufferHandle = createIndexBufferResult.Get();

				return RC_OK;
			}

			void _simulateParticles(IWorld* pWorld, F32 dt)
			{
				U32 currInstancesBufferIndex = 0;

				// \note Do main update logic here
				for (USIZE i = 0; i < mParticleEmitters.mpParticleEmitters.size(); ++i)
				{
					CParticleEmitter* pEmitterComponent = mParticleEmitters.mpParticleEmitters[i];
					if (!pEmitterComponent)
					{
						continue;
					}

					auto pCurrEffectResource = mpResourceManager->GetResource<IParticleEffect>(pEmitterComponent->GetParticleEffectHandle());
					if (!pCurrEffectResource)
					{
						continue;
					}

					auto& particles = mParticles[i];

#if TDE2_EDITORS_ENABLED
					if (pEmitterComponent->mResetStateOnNextFrame)
					{
						std::fill(particles.begin(), particles.end(), TParticleInfo{});
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

			void _populateCommandsBuffer(TSystemContext& context, CRenderQueue*& pRenderGroup, const IMaterial* pCurrMaterial, const ICamera* pCamera)
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

					pCommand->mVertexBufferHandle = TBufferHandleId::Invalid;
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

			static U32 _computeRenderCommandHash(TResourceId materialId, F32 distanceToCamera)
			{
				return (static_cast<U32>(materialId) << 16) | static_cast<U16>(fabs(distanceToCamera));
			}

		protected:
			IRenderer*                   mpRenderer = nullptr;

			TPtr<IResourceManager>       mpResourceManager = nullptr;

			CRenderQueue*                mpRenderQueue = nullptr;

			IGraphicsObjectManager*      mpGraphicsObjectManager = nullptr;

			TBufferHandleId              mParticleQuadIndexBufferHandle;

			IVertexDeclaration*          mpParticleVertexDeclaration = nullptr;

			CEntity*                     mpCameraEntity = nullptr;

			TParticlesArray              mParticlesInstancesData;

			TParticlesInfoArray          mParticles;

			TSystemContext               mParticleEmitters;

			std::vector<U32>             mActiveParticlesCount;

			std::vector<TPtr<IMaterial>> mUsedMaterials;

			std::vector<TBufferHandleId> mParticlesInstancesBufferHandles;
	};


	CParticlesCPUSimulationSystem::CParticlesCPUSimulationSystem() :
		CBaseSystem(),
		mParticleQuadIndexBufferHandle(TBufferHandleId::Invalid)
	{
	}


	TDE2_API ISystem* CreateParticlesSimulationSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CParticlesCPUSimulationSystem, result, pRenderer, pGraphicsObjectManager);
	}


	/*!
		class CParticlesGPUSimulationSystem

		\brief The class is a system that processes CParticleEmitter via compute shaders (both emission and simulation)
	*/

	class CParticlesGPUSimulationSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateParticlesGPUSimulationSystem(IRenderer*, IGraphicsObjectManager*, E_RESULT_CODE&);

		private:
			typedef struct TParticleVertex
			{
				TVector4 mPosition;
				TVector2 mUVs;
			} TParticleVertex, * TParticleVertexPtr;

			typedef struct TParticleInstanceData
			{
				TColor32F mColor;
				TVector4 mPositionAndSize;
				TVector4 mRotation;
			} TParticleInstanceData, * TParticleInstanceDataPtr;

			typedef std::vector<std::vector<TParticleInstanceData>> TParticlesArray;
			typedef std::vector<std::vector<TParticleInfo>> TParticlesInfoArray;

			typedef std::vector<CParticleEmitter*> TParticleEmmitters;

			struct TSystemContext
			{
				std::vector<CTransform*>       mpTransform;
				std::vector<CParticleEmitter*> mpParticleEmitters;
			};
		public:
			TDE2_SYSTEM(CParticlesGPUSimulationSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pRenderer A pointer to IRenderer implementation

				\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager)
			{
				TDE2_PROFILER_SCOPE("CParticlesGPUSimulationSystem::Init");

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

				mEmitParticlesShaderHandle = mpResourceManager->Load<IShader>(CProjectSettings::Get()->mGraphicsSettings.mEmitParticlesComputeShader);
				mSimulateParticlesShaderHandle = mpResourceManager->Load<IShader>(CProjectSettings::Get()->mGraphicsSettings.mSimulateParticlesComputeShader);

				TDE2_ASSERT(TResourceId::Invalid != mEmitParticlesShaderHandle);
				TDE2_ASSERT(TResourceId::Invalid != mSimulateParticlesShaderHandle);

				mIsInitialized = true;

				return RC_OK;
			}

			/*!
				\brief The method inject components array into a system

				\param[in] pWorld A pointer to a main scene's object
			*/

			TDE2_API void InjectBindings(IWorld* pWorld) override
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

						const size_t particlesCount = pCurrEffectResource ? static_cast<size_t>(pCurrEffectResource->GetMaxParticlesCount()) : 0;

						mParticlesInstancesData[i].resize(particlesCount);
						mParticles[i].resize(particlesCount);
					}
				}
			}

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] pWorld A pointer to a main scene's object

				\param[in] dt A delta time's value
			*/

			TDE2_API void Update(IWorld* pWorld, F32 dt) override
			{
				TDE2_PROFILER_SCOPE("CParticlesGPUSimulationSystem::Update");

				if (!mpCameraEntity)
				{
					LOG_WARNING("[CParticlesGPUSimulationSystem] An entity with Camera component attached to that wasn't found");
					return;
				}

				ICamera* pCameraComponent = GetValidPtrOrDefault<ICamera*>(mpCameraEntity->GetComponent<CPerspectiveCamera>(), mpCameraEntity->GetComponent<COrthoCamera>());
				TDE2_ASSERT(pCameraComponent);

				IGraphicsContext* pGraphicsContext = mpGraphicsObjectManager->GetGraphicsContext();
				pGraphicsContext->BeginSectionMarker("GPUParticlesSimulationPass");
				{
					// \todo reset particles if the flag is true

					const F32 deltaTime =
#if TDE2_EDITORS_ENABLED
						SimulationTimeCoeffCfgVar.Get() *
#endif
						dt;

					_emitParticles(pWorld, deltaTime);
					_simulateParticles(pWorld, deltaTime);
				}
				pGraphicsContext->EndSectionMarker();

				// \note Render particles 
				for (auto&& pCurrMaterial : mUsedMaterials)
				{
					_populateCommandsBuffer(mParticleEmitters, mpRenderQueue, pCurrMaterial.Get(), pCameraComponent);
				}
			}
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CParticlesGPUSimulationSystem)

				E_RESULT_CODE _initInternalVertexData()
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

				static const U16 faces[] =
				{
					0, 1, 2,
					2, 1, 3
				};

				auto createIndexBufferResult = mpGraphicsObjectManager->CreateBuffer({ E_BUFFER_USAGE_TYPE::STATIC, E_BUFFER_TYPE::INDEX, sizeof(U16) * 6, faces });
				if (createIndexBufferResult.HasError())
				{
					return createIndexBufferResult.GetError();
				}

				mParticleQuadIndexBufferHandle = createIndexBufferResult.Get();

				return RC_OK;
			}

			void _emitParticles(IWorld* pWorld, F32 dt)
			{
				TPtr<IShader> pEmitParticlesShader = mpResourceManager->GetResource<IShader>(mEmitParticlesShaderHandle);
				if (!pEmitParticlesShader)
				{
					return;
				}

				IGraphicsContext* pGraphicsContext = mpGraphicsObjectManager->GetGraphicsContext();
				pGraphicsContext->BeginSectionMarker("EmitParticles");

				for (USIZE i = 0; i < mParticleEmitters.mpParticleEmitters.size(); ++i)
				{
					CParticleEmitter* pEmitterComponent = mParticleEmitters.mpParticleEmitters[i];
					if (!pEmitterComponent)
					{
						continue;
					}

					auto pCurrEffectResource = mpResourceManager->GetResource<IParticleEffect>(pEmitterComponent->GetParticleEffectHandle());
					if (!pCurrEffectResource)
					{
						continue;
					}

					if (!pCurrEffectResource->GetEmissionRate())
					{
						continue;
					}

					auto pSharedEmitter = pCurrEffectResource->GetSharedEmitter();
					if (!pSharedEmitter)
					{
						continue;
					}

					// \note Fill in constant buffer with current emitter's data
					TEmitterUniformsData currEmitterShaderData = pSharedEmitter->GetShaderUniformsData();
					currEmitterShaderData.mPosition = TVector4(mParticleEmitters.mpTransform[i]->GetPosition(), 1.0f);
					
					// \note Bind the buffer
					pEmitParticlesShader->SetStructuredBufferResource("TileFrustums", TBufferHandleId::Invalid);
					pEmitParticlesShader->SetTextureResource("RandTexture", mpResourceManager->GetResource<ITexture2D>(mpResourceManager->Load<ITexture2D>(CProjectSettings::Get()->mGraphicsSettings.mRandomTextureId)).Get());
					pEmitParticlesShader->SetUserUniformsBuffer(0, reinterpret_cast<U8*>(&currEmitterShaderData), sizeof(currEmitterShaderData));
					pEmitParticlesShader->Bind();

					// \todo update constant buffer with dead particles count
					
					pGraphicsContext->DispatchCompute((currEmitterShaderData.mEmitRate + (currEmitterShaderData.mEmitRate % EMIT_DISPATCH_WORK_GROUP_SIZE)) / EMIT_DISPATCH_WORK_GROUP_SIZE, 1, 1);

					pGraphicsContext->SetStructuredBuffer(0, TBufferHandleId::Invalid, true);
				}

				pGraphicsContext->EndSectionMarker();
			}

			void _simulateParticles(IWorld* pWorld, F32 dt)
			{
				U32 currInstancesBufferIndex = 0;

				// \note Do main update logic here
				for (USIZE i = 0; i < mParticleEmitters.mpParticleEmitters.size(); ++i)
				{
					CParticleEmitter* pEmitterComponent = mParticleEmitters.mpParticleEmitters[i];
					if (!pEmitterComponent)
					{
						continue;
					}

					auto pCurrEffectResource = mpResourceManager->GetResource<IParticleEffect>(pEmitterComponent->GetParticleEffectHandle());
					if (!pCurrEffectResource)
					{
						continue;
					}

					auto& particles = mParticles[i];

	#if TDE2_EDITORS_ENABLED
					if (pEmitterComponent->mResetStateOnNextFrame)
					{
						std::fill(particles.begin(), particles.end(), TParticleInfo{});
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

			void _populateCommandsBuffer(TSystemContext& context, CRenderQueue*& pRenderGroup, const IMaterial* pCurrMaterial, const ICamera* pCamera)
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

					pCommand->mVertexBufferHandle = TBufferHandleId::Invalid;
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

			static U32 _computeRenderCommandHash(TResourceId materialId, F32 distanceToCamera)
			{
				return (static_cast<U32>(materialId) << 16) | static_cast<U16>(fabs(distanceToCamera));
			}

		protected:
			TDE2_STATIC_CONSTEXPR U32    EMIT_DISPATCH_WORK_GROUP_SIZE = 1024;

			IRenderer* mpRenderer = nullptr;

			TPtr<IResourceManager>       mpResourceManager = nullptr;

			CRenderQueue*                mpRenderQueue = nullptr;

			IGraphicsObjectManager*      mpGraphicsObjectManager = nullptr;

			TBufferHandleId              mParticleQuadIndexBufferHandle;

			IVertexDeclaration*          mpParticleVertexDeclaration = nullptr;

			CEntity*                     mpCameraEntity = nullptr;

			TParticlesArray              mParticlesInstancesData;

			TParticlesInfoArray          mParticles;

			TSystemContext               mParticleEmitters;

			std::vector<U32>             mActiveParticlesCount;

			std::vector<TPtr<IMaterial>> mUsedMaterials;

			std::vector<TBufferHandleId> mParticlesInstancesBufferHandles;

			TResourceId                  mEmitParticlesShaderHandle = TResourceId::Invalid;
			TResourceId                  mSimulateParticlesShaderHandle = TResourceId::Invalid;
	};


	CParticlesGPUSimulationSystem::CParticlesGPUSimulationSystem() :
		CBaseSystem(),
		mParticleQuadIndexBufferHandle(TBufferHandleId::Invalid)
	{
	}


	TDE2_API ISystem* CreateParticlesGPUSimulationSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CParticlesGPUSimulationSystem, result, pRenderer, pGraphicsObjectManager);
	}
}