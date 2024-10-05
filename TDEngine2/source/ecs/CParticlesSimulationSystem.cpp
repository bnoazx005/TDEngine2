#include "../../include/ecs/CParticlesSimulationSystem.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/effects/CParticleEmitterComponent.h"
#include "../../include/graphics/effects/CParticleEffect.h"
#include "../../include/graphics/effects/TParticle.h"
#include "../../include/graphics/effects/ParticleEmitters.h"
#include "../../include/graphics/CFramePacketsStorage.h"
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


	constexpr int Align(I32 value, I32 alignment) 
	{ 
		return (value + (alignment - 1)) & ~(alignment - 1); 
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
			} TParticleVertex, *TParticleVertexPtr;

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

				mpGraphicsObjectManager = pGraphicsObjectManager;
				mpFramePacketsStorage   = pRenderer->GetFramePacketsStorage().Get();

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

				CRenderQueue* pRenderQueue = mpFramePacketsStorage->GetCurrentFrameForGameLogic().mpRenderQueues[static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_TRANSPARENT_GEOMETRY)].Get();

				// \note Render particles 
				for (auto&& pCurrMaterial : mUsedMaterials)
				{
					_populateCommandsBuffer(mParticleEmitters, pRenderQueue, pCurrMaterial.Get(), pCameraComponent);
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

					if (pEmitterComponent->mResetStateOnNextFrame)
					{
						std::fill(particles.begin(), particles.end(), TParticleInfo{});
						pEmitterComponent->mResetStateOnNextFrame = false;
					}

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

			CFramePacketsStorage*        mpFramePacketsStorage = nullptr;

			IGraphicsObjectManager*      mpGraphicsObjectManager = nullptr;

			TBufferHandleId              mParticleQuadIndexBufferHandle = TBufferHandleId::Invalid;

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
		CBaseSystem()
	{
	}


	TDE2_API ISystem* CreateParticlesSimulationSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CParticlesCPUSimulationSystem, result, pRenderer, pGraphicsObjectManager);
	}


	struct TGPUSortLibContext
	{
		TResourceId mInitSortShaderHandle  = TResourceId::Invalid;
		TResourceId mSortInnerShaderHandle = TResourceId::Invalid;
		TResourceId mSortStepShaderHandle  = TResourceId::Invalid;
		TResourceId mSortShaderHandle      = TResourceId::Invalid;

		TBufferHandleId mDispatchArgsBufferHandle = TBufferHandleId::Invalid;
	} static GpuSortLibContext;


	static E_RESULT_CODE InitGPUSort(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, U32 maxCount, TBufferHandleId elementsBuffer, TBufferHandleId countBuffer)
	{
		TDE2_PROFILER_SCOPE("InitGPUSort");

		GpuSortLibContext.mInitSortShaderHandle = pResourceManager->Load<IShader>(CProjectSettings::Get()->mGraphicsSettings.mInitSortComputeShader);
		TDE2_ASSERT(TResourceId::Invalid != GpuSortLibContext.mInitSortShaderHandle);

		TPtr<IShader> pInitSortShader = pResourceManager->GetResource<IShader>(GpuSortLibContext.mInitSortShaderHandle);
		if (!pInitSortShader)
		{
			return RC_FAIL;
		}

		if (TBufferHandleId::Invalid == GpuSortLibContext.mDispatchArgsBufferHandle)
		{
			auto indirectDispatchArgsBufferCreateResult = pGraphicsContext->GetGraphicsObjectManager()->CreateBuffer(
				{
					E_BUFFER_USAGE_TYPE::DEFAULT,
					E_BUFFER_TYPE::STRUCTURED,
					sizeof(U32) * 4,
					nullptr,
					sizeof(U32) * 4,
					true,
					sizeof(U32),
					E_STRUCTURED_BUFFER_TYPE::INDIRECT_DRAW_BUFFER,
					E_INDEX_FORMAT_TYPE::INDEX16, // unused
					"IndirectDispatchArgsBuffer"
				});

			if (indirectDispatchArgsBufferCreateResult.HasError())
			{
				return indirectDispatchArgsBufferCreateResult.GetError();
			}

			GpuSortLibContext.mDispatchArgsBufferHandle = indirectDispatchArgsBufferCreateResult.Get();
		}

		pInitSortShader->SetStructuredBufferResource("DispatchArgsBuffer", GpuSortLibContext.mDispatchArgsBufferHandle);
		pInitSortShader->SetStructuredBufferResource("ElementsCount", countBuffer);
		pInitSortShader->Bind();

		pGraphicsContext->DispatchCompute(1, 1, 1);

		return RC_OK;
	}

	static bool GPUSortInitial(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, U32 maxCount, TBufferHandleId elementsBuffer, TBufferHandleId countBuffer, TBufferHandleId indirectDispatchBufferHandle)
	{
		TDE2_PROFILER_SCOPE("GPUSortInitial");

		const U32 threadGroupsCount = ((maxCount - 1) >> 9) + 1;
		TDE2_ASSERT(threadGroupsCount <= 1024);

		GpuSortLibContext.mSortShaderHandle = pResourceManager->Load<IShader>(CProjectSettings::Get()->mGraphicsSettings.mSortComputeShader);
		TDE2_ASSERT(TResourceId::Invalid != GpuSortLibContext.mSortShaderHandle);

		TPtr<IShader> pSortShader = pResourceManager->GetResource<IShader>(GpuSortLibContext.mSortShaderHandle);
		if (!pSortShader)
		{
			return RC_FAIL;
		}

		// sort all buffers of size 512 (and presort bigger ones)

		pSortShader->SetStructuredBufferResource("OutputData", elementsBuffer);
		pSortShader->SetStructuredBufferResource("ElementsCount", countBuffer);
		pSortShader->Bind();

		pGraphicsContext->DispatchIndirectCompute(indirectDispatchBufferHandle, 0);

		return threadGroupsCount <= 1;
	}

	static bool GPUSortIncremental(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, U32 presortedCount, U32 maxCount, TBufferHandleId elementsBuffer, TBufferHandleId countBuffer)
	{
		TDE2_PROFILER_SCOPE("GPUSortIncremental");

		GpuSortLibContext.mSortStepShaderHandle = pResourceManager->Load<IShader>(CProjectSettings::Get()->mGraphicsSettings.mSortStepComputeShader);
		TDE2_ASSERT(TResourceId::Invalid != GpuSortLibContext.mSortStepShaderHandle);

		TPtr<IShader> pSortStepShader = pResourceManager->GetResource<IShader>(GpuSortLibContext.mSortStepShaderHandle);
		if (!pSortStepShader)
		{
			return RC_FAIL;
		}

		bool isDone = true;
		U32 threadGroupsCount = 0;

		if (maxCount > presortedCount)
		{
			if (maxCount > presortedCount * 2)
			{
				isDone = false;
			}

			U32 pow2 = presortedCount;
			while (pow2 < maxCount)
			{
				pow2 <<= 1;
			}

			threadGroupsCount = pow2 >> 9;
		}

		pSortStepShader->SetStructuredBufferResource("OutputData", elementsBuffer);
		pSortStepShader->SetStructuredBufferResource("ElementsCount", countBuffer);

		U32 mergeSize = presortedCount << 1;

		for (U32 mergeSubSize = mergeSize >> 1; mergeSubSize > 256; mergeSubSize = mergeSubSize >> 1)
		{
			U32 jobParams[4] { 0 };
			jobParams[0] = mergeSubSize;

			if (mergeSubSize == mergeSize >> 1)
			{
				jobParams[1] = (2 * mergeSubSize - 1);
				jobParams[2] = -1;
			}
			else
			{
				jobParams[1] = mergeSubSize;
				jobParams[2] = 1;
			}

			pSortStepShader->SetUserUniformsBuffer(0, reinterpret_cast<const U8*>(jobParams), sizeof(jobParams));
			pSortStepShader->Bind();

			pGraphicsContext->DispatchCompute(threadGroupsCount, 1, 1);
		}

		GpuSortLibContext.mSortInnerShaderHandle = pResourceManager->Load<IShader>(CProjectSettings::Get()->mGraphicsSettings.mSortInnerComputeShader);
		TDE2_ASSERT(TResourceId::Invalid != GpuSortLibContext.mSortInnerShaderHandle);

		TPtr<IShader> pSortInnerShader = pResourceManager->GetResource<IShader>(GpuSortLibContext.mSortInnerShaderHandle);
		if (!pSortInnerShader)
		{
			return RC_FAIL;
		}

		pSortInnerShader->SetStructuredBufferResource("OutputData", elementsBuffer);
		pSortInnerShader->SetStructuredBufferResource("ElementsCount", countBuffer);
		pSortInnerShader->Bind();

		pGraphicsContext->DispatchCompute(threadGroupsCount, 1, 1);

		return isDone;
	}


	// bitonic sort algorithm executed on GPU device
	E_RESULT_CODE GPUSort(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, U32 maxCount, TBufferHandleId elementsBuffer, TBufferHandleId countBuffer)
	{
		TDE2_PROFILER_SCOPE("GPUSort");

		TDE2_ASSERT(pGraphicsContext);
		TDE2_ASSERT(pResourceManager);

		if (!pGraphicsContext || !pResourceManager || TBufferHandleId::Invalid == elementsBuffer || TBufferHandleId::Invalid == countBuffer)
		{
			return RC_INVALID_ARGS;
		}

#if TDE2_DEBUG_MODE
		pGraphicsContext->BeginSectionMarker("GPUSort");
#endif
		
		E_RESULT_CODE result = InitGPUSort(pGraphicsContext, pResourceManager, maxCount, elementsBuffer, countBuffer);
		if (RC_OK != result)
		{
			return result;
		}

		bool isDone = GPUSortInitial(pGraphicsContext, pResourceManager, maxCount, elementsBuffer, countBuffer, GpuSortLibContext.mDispatchArgsBufferHandle);
		I32 presorted = 512;

		while (!isDone)
		{
			isDone = GPUSortIncremental(pGraphicsContext, pResourceManager, static_cast<U32>(presorted), maxCount, elementsBuffer, countBuffer);
			presorted <<= 1;
		}

#if TDE2_DEBUG_MODE
		pGraphicsContext->EndSectionMarker();
#endif

		return RC_OK;
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
			typedef std::vector<CParticleEmitter*> TParticleEmmitters;

			struct TSystemContext
			{
				std::vector<CTransform*>       mpTransform;
				std::vector<CParticleEmitter*> mpParticleEmitters;
			};

			// duplicates the structure from TDEngine2ParticleUtils.inc, needed to compute size for storage buffer
			struct TGPUParticle
			{
				TVector4 mPosition;
				TVector4 mVelocity;
				TVector4 mColor;
				TVector4 mLifeParams;
				U32      mEmitterParams[4];
				TVector4 mForceParams;
			};

			struct TActiveParticleIndexElement
			{
				F32 mDistance;
				F32 mIndex;
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

				mpGraphicsObjectManager = pGraphicsObjectManager;
				mpFramePacketsStorage   = pRenderer->GetFramePacketsStorage().Get();

				mpResourceManager = pRenderer->GetResourceManager();

				E_RESULT_CODE result = _initInternalVertexData();
				if (RC_OK != result)
				{
					return result;
				}

				mEmitParticlesShaderHandle = mpResourceManager->Load<IShader>(CProjectSettings::Get()->mGraphicsSettings.mEmitParticlesComputeShader);
				mSimulateParticlesShaderHandle = mpResourceManager->Load<IShader>(CProjectSettings::Get()->mGraphicsSettings.mSimulateParticlesComputeShader);
				mInitDeadParticlesListShaderHandle = mpResourceManager->Load<IShader>(CProjectSettings::Get()->mGraphicsSettings.mInitDeadParticlesListComputeShader);

				TDE2_ASSERT(TResourceId::Invalid != mEmitParticlesShaderHandle);
				TDE2_ASSERT(TResourceId::Invalid != mSimulateParticlesShaderHandle);
				TDE2_ASSERT(TResourceId::Invalid != mInitDeadParticlesListShaderHandle);

				_initDeadParticlesList();

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

				if (mParticleEmitters.mpParticleEmitters.empty())
				{
					return;
				}

				IGraphicsContext* pGraphicsContext = mpGraphicsObjectManager->GetGraphicsContext();
#if TDE2_DEBUG_MODE
				pGraphicsContext->BeginSectionMarker("GPUParticlesSimulationPass");
#endif
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

				GPUSort(pGraphicsContext, mpResourceManager.Get(), MAX_PARTICLES_COUNT, mAliveIndexBufferHandle, mCountersBufferHandle);

#if TDE2_DEBUG_MODE
				pGraphicsContext->EndSectionMarker();
#endif

				_prepareRenderCommand(); // \note All particles for all emitters are batched and drawn in the single command
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

				auto countersBufferCreateResult = mpGraphicsObjectManager->CreateBuffer(
					{
						E_BUFFER_USAGE_TYPE::DEFAULT,
						E_BUFFER_TYPE::STRUCTURED,
						sizeof(U32) * COUNTERS_COUNT,
						nullptr,
						sizeof(U32) * COUNTERS_COUNT,
						true,
						sizeof(U32),
						E_STRUCTURED_BUFFER_TYPE::DEFAULT,
						E_INDEX_FORMAT_TYPE::INDEX16, // unused
						"CountersBuffer"
					});

				if (countersBufferCreateResult.HasError())
				{
					return countersBufferCreateResult.GetError();
				}

				mCountersBufferHandle = countersBufferCreateResult.Get();

				auto deadParticlesListBufferCreateResult = mpGraphicsObjectManager->CreateBuffer(
					{
						E_BUFFER_USAGE_TYPE::DEFAULT,
						E_BUFFER_TYPE::STRUCTURED,
						sizeof(U32) * MAX_PARTICLES_COUNT,
						nullptr,
						sizeof(U32) * MAX_PARTICLES_COUNT,
						true,
						sizeof(U32),
						E_STRUCTURED_BUFFER_TYPE::DEFAULT,
						E_INDEX_FORMAT_TYPE::INDEX16, // unused
						"DeadParticlesListBuffer"
					});

				if (deadParticlesListBufferCreateResult.HasError())
				{
					return deadParticlesListBufferCreateResult.GetError();
				}

				mDeadListBufferHandle = deadParticlesListBufferCreateResult.Get();

				// particles buffer
				auto particlesBufferCreateResult = mpGraphicsObjectManager->CreateBuffer(
					{
						E_BUFFER_USAGE_TYPE::DEFAULT,
						E_BUFFER_TYPE::STRUCTURED,
						sizeof(TGPUParticle) * MAX_PARTICLES_COUNT,
						nullptr,
						sizeof(TGPUParticle) * MAX_PARTICLES_COUNT,
						true,
						sizeof(TGPUParticle),
						E_STRUCTURED_BUFFER_TYPE::DEFAULT,
						E_INDEX_FORMAT_TYPE::INDEX16, // unused
						"ParticlesBuffer"
					});

				if (particlesBufferCreateResult.HasError())
				{
					return particlesBufferCreateResult.GetError();
				}

				mParticlesBufferHandle = particlesBufferCreateResult.Get();

				auto aliveIndexBufferCreateResult = mpGraphicsObjectManager->CreateBuffer(
					{
						E_BUFFER_USAGE_TYPE::DEFAULT,
						E_BUFFER_TYPE::STRUCTURED,
						sizeof(TActiveParticleIndexElement) * MAX_PARTICLES_COUNT,
						nullptr,
						sizeof(TActiveParticleIndexElement) * MAX_PARTICLES_COUNT,
						true,
						sizeof(TActiveParticleIndexElement),
						E_STRUCTURED_BUFFER_TYPE::DEFAULT,
						E_INDEX_FORMAT_TYPE::INDEX16, // unused
						"AliveIndexParticlesBuffer"
					});

				if (aliveIndexBufferCreateResult.HasError())
				{
					return aliveIndexBufferCreateResult.GetError();
				}

				mAliveIndexBufferHandle = aliveIndexBufferCreateResult.Get();
				
				auto indirectDrawArgsBufferCreateResult = mpGraphicsObjectManager->CreateBuffer(
					{
						E_BUFFER_USAGE_TYPE::DEFAULT,
						E_BUFFER_TYPE::STRUCTURED,
						sizeof(U32) * 5,
						nullptr,
						sizeof(U32) * 5,
						true,
						sizeof(U32),
						E_STRUCTURED_BUFFER_TYPE::INDIRECT_DRAW_BUFFER,
						E_INDEX_FORMAT_TYPE::INDEX16, // unused
						"IndirectDrawArgsBuffer"
					});

				if (indirectDrawArgsBufferCreateResult.HasError())
				{
					return indirectDrawArgsBufferCreateResult.GetError();
				}

				mIndirectDrawArgsBufferHandle = indirectDrawArgsBufferCreateResult.Get();

				return _initEmittersBakedDataAtlasTexture();
			}

			E_RESULT_CODE _initEmittersBakedDataAtlasTexture()
			{
				mEmittersBakedParamsAtlasHandle = mpResourceManager->Create<ITexture2D>("ParticlesEmittersBakedParamsAtlas", TTexture2DParameters(EMITTERS_BAKED_DATA_ATLAS_SIZES, EMITTERS_BAKED_DATA_ATLAS_SIZES, FT_FLOAT4));
				if (TResourceId::Invalid == mEmittersBakedParamsAtlasHandle)
				{
					return RC_FAIL;
				}

				return RC_OK;
			}

			static void _writeColorCurve(std::vector<TColor32F>& output, const TParticleColorParameter& curve)
			{
				for (U32 i = 0; i < EMITTERS_BAKED_CURVE_WIDTH; ++i)
				{
					const F32 t = i / static_cast<F32>(EMITTERS_BAKED_CURVE_WIDTH);

					switch (curve.mType)
					{
						case E_PARTICLE_COLOR_PARAMETER_TYPE::SINGLE_COLOR:
							output.emplace_back(curve.mFirstColor);
							break;
						case E_PARTICLE_COLOR_PARAMETER_TYPE::TWEEN_RANDOM:
							output.emplace_back(LerpColors(curve.mFirstColor, curve.mSecondColor, t));
							break;

						case E_PARTICLE_COLOR_PARAMETER_TYPE::GRADIENT_LERP:
						case E_PARTICLE_COLOR_PARAMETER_TYPE::GRADIENT_RANDOM:
							output.emplace_back(curve.mGradientColor ? curve.mGradientColor->Sample(t) : TColorUtils::mWhite);
							break;
					}
				}
			}

			void _updateEmittersBakedData(U32 emitterIndex, TPtr<IParticleEffect> pParticleEffect)
			{
				const U32 xStartOffset = emitterIndex / EMITTERS_BAKED_DATA_ATLAS_SIZES;
				const U32 yStartOffset = emitterIndex % EMITTERS_BAKED_DATA_ATLAS_SIZES;

				std::vector<TVector4> velocitySizeCurves;

				auto&& pSizeCurve = pParticleEffect->GetSizeCurve();
				auto&& velocityInfo = pParticleEffect->GetVelocityOverTime();

				for (U32 i = 0; i < EMITTERS_BAKED_CURVE_WIDTH; ++i)
				{
					const F32 t = i / static_cast<F32>(EMITTERS_BAKED_CURVE_WIDTH);

					if (E_PARTICLE_VELOCITY_PARAMETER_TYPE::CONSTANTS == velocityInfo.mType)
					{
						velocitySizeCurves.emplace_back(Normalize(velocityInfo.mVelocityConst) * velocityInfo.mSpeedFactorConst, pSizeCurve ? pSizeCurve->Sample(t) : 1.0f);
						continue;
					}

					TVector3 velocity = Normalize({ velocityInfo.mXCurve->Sample(t), velocityInfo.mYCurve->Sample(t), velocityInfo.mZCurve->Sample(t) });
					velocity = velocity * velocityInfo.mSpeedFactorCurve->Sample(t);

					velocitySizeCurves.emplace_back(velocity, pSizeCurve ? pSizeCurve->Sample(t) : 1.0f);
				}

				std::vector<TColor32F> colorCurves;

				_writeColorCurve(colorCurves, pParticleEffect->GetColorOverLifeTime());

				TPtr<ITexture2D> pEmittersAtlasTexture = mpResourceManager->GetResource<ITexture2D>(mEmittersBakedParamsAtlasHandle);
				pEmittersAtlasTexture->WriteData({ static_cast<I32>(xStartOffset * EMITTERS_BAKED_CURVE_WIDTH), static_cast<I32>(yStartOffset), static_cast<I32>(EMITTERS_BAKED_CURVE_WIDTH), 1 }, reinterpret_cast<const U8*>(velocitySizeCurves.data()));
				pEmittersAtlasTexture->WriteData({ static_cast<I32>(xStartOffset * EMITTERS_BAKED_CURVE_WIDTH), static_cast<I32>(yStartOffset + 1), static_cast<I32>(EMITTERS_BAKED_CURVE_WIDTH), 1 }, reinterpret_cast<const U8*>(colorCurves.data()));
			}

			void _emitParticles(IWorld* pWorld, F32 dt)
			{
				TDE2_PROFILER_SCOPE("CParticlesGPUSimulationSystem::EmitParticle");

				TPtr<IShader> pEmitParticlesShader = mpResourceManager->GetResource<IShader>(mEmitParticlesShaderHandle);
				if (!pEmitParticlesShader)
				{
					return;
				}

				IGraphicsContext* pGraphicsContext = mpGraphicsObjectManager->GetGraphicsContext();
#if TDE2_DEBUG_MODE
				pGraphicsContext->BeginSectionMarker("EmitParticles");
#endif

				for (USIZE i = 0; i < mParticleEmitters.mpParticleEmitters.size(); ++i)
				{
					CParticleEmitter* pEmitterComponent = mParticleEmitters.mpParticleEmitters[i];
					if (!pEmitterComponent)
					{
						continue;
					}

					TResourceId particleEffectResourceHandle = pEmitterComponent->GetParticleEffectHandle();
					if (TResourceId::Invalid == particleEffectResourceHandle)
					{
						particleEffectResourceHandle = mpResourceManager->Load<IParticleEffect>(pEmitterComponent->GetParticleEffectId());
						pEmitterComponent->SetParticleEffectHandle(particleEffectResourceHandle);

						_updateEmittersBakedData(static_cast<U32>(i), mpResourceManager->GetResource<IParticleEffect>(particleEffectResourceHandle));
					}

					auto pCurrEffectResource = mpResourceManager->GetResource<IParticleEffect>(particleEffectResourceHandle);
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
					currEmitterShaderData.mEmitterIndex = static_cast<U32>(i);
					currEmitterShaderData.mEmittersAtlasStartPos[0] = static_cast<U32>(i / EMITTERS_BAKED_DATA_ATLAS_SIZES) * EMITTERS_BAKED_CURVE_WIDTH;
					currEmitterShaderData.mEmittersAtlasStartPos[1] = i % EMITTERS_BAKED_DATA_ATLAS_SIZES;
					
					// \note Bind the buffer
					pEmitParticlesShader->SetStructuredBufferResource("OutputParticles", mParticlesBufferHandle);
					pEmitParticlesShader->SetStructuredBufferResource("DeadParticlesIndexList", mDeadListBufferHandle);
					pEmitParticlesShader->SetStructuredBufferResource("Counters", mCountersBufferHandle);
					pEmitParticlesShader->SetTextureResource("EmittersParamsAtlas", mpResourceManager->GetResource<ITexture2D>(mEmittersBakedParamsAtlasHandle).Get());
					pEmitParticlesShader->SetTextureResource("RandTexture", mpResourceManager->GetResource<ITexture2D>(mpResourceManager->Load<ITexture2D>(CProjectSettings::Get()->mGraphicsSettings.mRandomTextureId)).Get());
					pEmitParticlesShader->SetTextureResource("EmittersCurvesAtlasTexture", mpResourceManager->GetResource<ITexture2D>(mEmittersBakedParamsAtlasHandle).Get());
					pEmitParticlesShader->SetUserUniformsBuffer(0, reinterpret_cast<U8*>(&currEmitterShaderData), sizeof(currEmitterShaderData));
					pEmitParticlesShader->Bind();
										
					pGraphicsContext->DispatchCompute(Align(currEmitterShaderData.mEmitRate, EMIT_DISPATCH_WORK_GROUP_SIZE) / EMIT_DISPATCH_WORK_GROUP_SIZE, 1, 1);

					pGraphicsContext->SetStructuredBuffer(pEmitParticlesShader->GetResourceBindingSlot("OutputParticles"), TBufferHandleId::Invalid, true);
					pGraphicsContext->SetStructuredBuffer(pEmitParticlesShader->GetResourceBindingSlot("DeadParticlesIndexList"), TBufferHandleId::Invalid, true);
					pGraphicsContext->SetStructuredBuffer(pEmitParticlesShader->GetResourceBindingSlot("Counters"), TBufferHandleId::Invalid, true);
				}

#if TDE2_DEBUG_MODE
				pGraphicsContext->EndSectionMarker();
#endif
			}

			void _simulateParticles(IWorld* pWorld, F32 dt)
			{
				TDE2_PROFILER_SCOPE("CParticlesGPUSimulationSystem::SimulateParticles");

				TPtr<IShader> pSimulateParticlesShader = mpResourceManager->GetResource<IShader>(mSimulateParticlesShaderHandle);
				if (!pSimulateParticlesShader)
				{
					return;
				}

				auto&& pCamerasContext = pWorld->FindEntity(pWorld->FindEntityWithUniqueComponent<CCamerasContextComponent>())->GetComponent<CCamerasContextComponent>();
				auto&& pActiveCameraTransform = pWorld->FindEntity(pCamerasContext->GetActiveCameraEntityId())->GetComponent<CTransform>();
				if (!pActiveCameraTransform)
				{
					LOG_WARNING("[CParticlesCPUSimulationSystem] An entity with Camera component attached to that wasn't found");
					return;
				}

				IGraphicsContext* pGraphicsContext = mpGraphicsObjectManager->GetGraphicsContext();
#if TDE2_DEBUG_MODE
				pGraphicsContext->BeginSectionMarker("SimulateParticles");
#endif

				struct
				{
					TVector4 mCameraPosition;
					F32      mDeltaTime;
					U32      mMaxParticlesCount;
				} simulationParams;

				simulationParams.mCameraPosition = TVector4(pActiveCameraTransform->GetPosition(), 1.0f);
				simulationParams.mDeltaTime = dt;
				simulationParams.mMaxParticlesCount = MAX_PARTICLES_COUNT;

				pSimulateParticlesShader->SetStructuredBufferResource("OutputParticles", mParticlesBufferHandle);
				pSimulateParticlesShader->SetStructuredBufferResource("DeadParticlesIndexList", mDeadListBufferHandle);
				pSimulateParticlesShader->SetStructuredBufferResource("ParticlesIndexBuffer", mAliveIndexBufferHandle);
				pSimulateParticlesShader->SetStructuredBufferResource("DrawArgsBuffer", mIndirectDrawArgsBufferHandle);
				pSimulateParticlesShader->SetStructuredBufferResource("Counters", mCountersBufferHandle);
				pSimulateParticlesShader->SetTextureResource("RandTexture", mpResourceManager->GetResource<ITexture2D>(mpResourceManager->Load<ITexture2D>(CProjectSettings::Get()->mGraphicsSettings.mRandomTextureId)).Get());
				pSimulateParticlesShader->SetTextureResource("EmittersCurvesAtlasTexture", mpResourceManager->GetResource<ITexture2D>(mEmittersBakedParamsAtlasHandle).Get());
				pSimulateParticlesShader->SetUserUniformsBuffer(0, reinterpret_cast<U8*>(&simulationParams), sizeof(simulationParams));
				pSimulateParticlesShader->Bind();

				pGraphicsContext->DispatchCompute(Align(MAX_PARTICLES_COUNT, SIMULATE_DISPATCH_WORK_GROUP_SIZE) / SIMULATE_DISPATCH_WORK_GROUP_SIZE, 1, 1);

				pGraphicsContext->SetStructuredBuffer(pSimulateParticlesShader->GetResourceBindingSlot("OutputParticles"), TBufferHandleId::Invalid, true);
				pGraphicsContext->SetStructuredBuffer(pSimulateParticlesShader->GetResourceBindingSlot("DeadParticlesIndexList"), TBufferHandleId::Invalid, true);
				pGraphicsContext->SetStructuredBuffer(pSimulateParticlesShader->GetResourceBindingSlot("ParticlesIndexBuffer"), TBufferHandleId::Invalid, true);
				pGraphicsContext->SetStructuredBuffer(pSimulateParticlesShader->GetResourceBindingSlot("DrawArgsBuffer"), TBufferHandleId::Invalid, true);
				pGraphicsContext->SetStructuredBuffer(pSimulateParticlesShader->GetResourceBindingSlot("Counters"), TBufferHandleId::Invalid, true);

#if TDE2_DEBUG_MODE
				pGraphicsContext->EndSectionMarker();
#endif
			}

			void _prepareRenderCommand()
			{
				const TResourceId materialHandle = mpResourceManager->Load<IMaterial>(CProjectSettings::Get()->mGraphicsSettings.mParticleRenderMaterial);
				if (TResourceId::Invalid == materialHandle)
				{
					return;
				}

				auto pMaterial = mpResourceManager->GetResource<IMaterial>(materialHandle);
				if (!pMaterial)
				{
					return;
				}

				// \fixme Replace these calls with methods of IMaterial later
				auto pShader = mpResourceManager->GetResource<IShader>(pMaterial->GetShaderHandle());
				pShader->SetStructuredBufferResource("Particles", mParticlesBufferHandle);
				pShader->SetStructuredBufferResource("AliveParticlesIndexBuffer", mAliveIndexBufferHandle);
				pShader->SetStructuredBufferResource("Counters", mCountersBufferHandle);

				TPtr<CRenderQueue> pRenderQueue = mpFramePacketsStorage->GetCurrentFrameForGameLogic().mpRenderQueues[static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_TRANSPARENT_GEOMETRY)];
				if (!pRenderQueue)
				{
					return;
				}

				auto pCommand = pRenderQueue->SubmitDrawCommand<TDrawIndirectIndexedInstancedCommand>(static_cast<U32>(pMaterial->GetGeometrySubGroupTag()) + _computeRenderCommandHash(materialHandle, 0.0f));

				pCommand->mUseIndexedCommand = true;
				pCommand->mAlignedOffset = 0;
				pCommand->mArgsBufferHandle = mIndirectDrawArgsBufferHandle;
				pCommand->mVertexBufferHandle = TBufferHandleId::Invalid;
				pCommand->mIndexBufferHandle = mParticleQuadIndexBufferHandle;
				pCommand->mMaterialHandle = materialHandle;
				pCommand->mpVertexDeclaration = mpParticleVertexDeclaration;
				pCommand->mPrimitiveType = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
				pCommand->mObjectData.mModelMatrix = IdentityMatrix4;
				pCommand->mObjectData.mInvModelMatrix = IdentityMatrix4;
			}

			void _initDeadParticlesList()
			{
				TPtr<IShader> pInitDeadParticlesListShader = mpResourceManager->GetResource<IShader>(mInitDeadParticlesListShaderHandle);
				if (!pInitDeadParticlesListShader)
				{
					return;
				}

				IGraphicsContext* pGraphicsContext = mpGraphicsObjectManager->GetGraphicsContext();
#if TDE2_DEBUG_MODE
				pGraphicsContext->BeginSectionMarker("InitDeadParticles");
#endif

				struct
				{
					U32 mMaxParticlesCount = MAX_PARTICLES_COUNT;
				} shaderParams{};

				pInitDeadParticlesListShader->SetUserUniformsBuffer(0, reinterpret_cast<const U8*>(&shaderParams), sizeof(shaderParams));
				pInitDeadParticlesListShader->SetStructuredBufferResource("DeadParticlesIndexList", mDeadListBufferHandle);
				pInitDeadParticlesListShader->SetStructuredBufferResource("Counters", mCountersBufferHandle);
				pInitDeadParticlesListShader->Bind();

				pGraphicsContext->DispatchCompute(Align(MAX_PARTICLES_COUNT + 1, INIT_DEAD_PARTICLES_DISPATCH_WORK_GROUP_SIZE) / INIT_DEAD_PARTICLES_DISPATCH_WORK_GROUP_SIZE, 1, 1);

				pGraphicsContext->SetStructuredBuffer(pInitDeadParticlesListShader->GetResourceBindingSlot("DeadParticlesIndexList"), TBufferHandleId::Invalid, true);
				pGraphicsContext->SetStructuredBuffer(pInitDeadParticlesListShader->GetResourceBindingSlot("Counters"), TBufferHandleId::Invalid, true);

#if TDE2_DEBUG_MODE
				pGraphicsContext->EndSectionMarker();
#endif
			}

			static U32 _computeRenderCommandHash(TResourceId materialId, F32 distanceToCamera)
			{
				return (static_cast<U32>(materialId) << 16) | static_cast<U16>(fabs(distanceToCamera));
			}

		protected:
			TDE2_STATIC_CONSTEXPR U32    MAX_PARTICLES_COUNT = 512 * 1024;
			TDE2_STATIC_CONSTEXPR U32    COUNTERS_COUNT = 2;
			
			TDE2_STATIC_CONSTEXPR U32    EMIT_DISPATCH_WORK_GROUP_SIZE = 1024;
			TDE2_STATIC_CONSTEXPR U32    SIMULATE_DISPATCH_WORK_GROUP_SIZE = 256;
			TDE2_STATIC_CONSTEXPR U32    INIT_DEAD_PARTICLES_DISPATCH_WORK_GROUP_SIZE = 256;

			TDE2_STATIC_CONSTEXPR U32    EMITTERS_BAKED_DATA_ATLAS_SIZES = 1024;
			TDE2_STATIC_CONSTEXPR U32    EMITTERS_BAKED_CURVE_WIDTH = 128;

			IRenderer*                   mpRenderer = nullptr;

			TPtr<IResourceManager>       mpResourceManager = nullptr;

			CFramePacketsStorage*        mpFramePacketsStorage = nullptr;

			IGraphicsObjectManager*      mpGraphicsObjectManager = nullptr;

			TBufferHandleId              mParticleQuadIndexBufferHandle;

			IVertexDeclaration*          mpParticleVertexDeclaration = nullptr;

			TSystemContext               mParticleEmitters;

			TResourceId                  mEmitParticlesShaderHandle = TResourceId::Invalid;
			TResourceId                  mSimulateParticlesShaderHandle = TResourceId::Invalid;
			TResourceId                  mInitDeadParticlesListShaderHandle = TResourceId::Invalid;

			TResourceId                  mEmittersBakedParamsAtlasHandle = TResourceId::Invalid;

			TBufferHandleId              mDeadListBufferHandle = TBufferHandleId::Invalid;
			TBufferHandleId              mParticlesBufferHandle = TBufferHandleId::Invalid;
			TBufferHandleId              mAliveIndexBufferHandle = TBufferHandleId::Invalid;
			TBufferHandleId              mIndirectDrawArgsBufferHandle = TBufferHandleId::Invalid;
			TBufferHandleId              mCountersBufferHandle = TBufferHandleId::Invalid;
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