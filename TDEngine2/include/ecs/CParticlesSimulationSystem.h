/*!
	\file CParticlesSimulationSystem.h
	\date 21.03.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include "../graphics/effects/TParticle.h"
#include "../math/TVector2.h"
#include "../math/TVector4.h"
#include "../utils/Color.h"
#include <vector>


namespace TDEngine2
{
	class IResourceManager;
	class IRenderer;
	class IGraphicsObjectManager;
	class CRenderQueue;
	class IVertexBuffer;
	class IIndexBuffer;
	class IVertexDeclaration;
	class CEntity;
	class IMaterial;
	class ICamera;
	

	enum class TEntityId : U32;


	/*!
		\brief A factory function for creation objects of CParticlesSimulationSystem's type.

		\param[in, out] pRenderer A pointer to IRenderer implementation
		\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CParticlesSimulationSystem's implementation
	*/

	TDE2_API ISystem* CreateParticlesSimulationSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);


	/*!
		class CParticlesSimulationSystem

		\brief The class is a system that processes CParticleEmitter components
	*/

	class CParticlesSimulationSystem : public CBaseSystem
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
			} TParticleInstanceData, *TParticleInstanceDataPtr;			

			typedef std::vector<std::vector<TParticleInstanceData>> TParticlesArray;
			typedef std::vector<std::vector<TParticleInfo>> TParticlesInfoArray;

	public:
			TDE2_SYSTEM(CParticlesSimulationSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pRenderer A pointer to IRenderer implementation

				\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method inject components array into a system

				\param[in] pWorld A pointer to a main scene's object
			*/

			TDE2_API void InjectBindings(IWorld* pWorld) override;

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] pWorld A pointer to a main scene's object

				\param[in] dt A delta time's value
			*/

			TDE2_API void Update(IWorld* pWorld, F32 dt) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CParticlesSimulationSystem)

			TDE2_API E_RESULT_CODE _initInternalVertexData();
			
			TDE2_API void _simulateParticles(IWorld* pWorld, F32 dt);

			TDE2_API void _populateCommandsBuffer(const std::vector<TEntityId>& entities, IWorld* pWorld, CRenderQueue*& pRenderGroup, const IMaterial* pCurrMaterial, const ICamera* pCamera);

			TDE2_API U32 _computeRenderCommandHash(TResourceId materialId, F32 distanceToCamera);

		protected:
			IRenderer*              mpRenderer;

			IResourceManager*       mpResourceManager;

			CRenderQueue*           mpRenderQueue;

			IGraphicsObjectManager* mpGraphicsObjectManager;

			IVertexBuffer*          mpParticleQuadVertexBuffer;
			
			IIndexBuffer*           mpParticleQuadIndexBuffer;

			IVertexDeclaration*     mpParticleVertexDeclaration;

			CEntity*                mpCameraEntity;

			TParticlesArray         mParticlesInstancesData;

			TParticlesInfoArray     mParticles;

			std::vector<TEntityId>  mParticleEmitters;

			std::vector<U32>        mActiveParticlesCount;

			std::vector<IMaterial*> mUsedMaterials;

			std::vector<IVertexBuffer*> mpParticlesInstancesBuffers;

	};
}