/*!
	\file CLightingSystem.h
	\date 07.05.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include "../math/TMatrix4.h"
#include "../ecs/IWorld.h"
#include "../graphics/InternalShaderData.h"


namespace TDEngine2
{
	class IRenderer;
	class IVertexDeclaration;
	class IGraphicsObjectManager;
	class IResourceManager;
	class CRenderQueue;
	class IGraphicsContext;
	class CEntity;
	struct TLightingShaderData;
	class CDirectionalLight;
	class CPointLight;
	class CSpotLight;
	class CTransform;
	class CShadowCasterComponent;
	class CShadowReceiverComponent;
	class CStaticMeshContainer;
	class CSkinnedMeshContainer;

	TDE2_DECLARE_SCOPED_PTR(IResourceManager)


	/*!
		\brief A factory function for creation objects of CLightingSystem's type.

		\param[in, out] pRenderer A pointer to IRenderer implementation
		\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CLightingSystem's implementation
	*/

	TDE2_API ISystem* CreateLightingSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);


	/*!
		class CLightingSystem

		\brief The class is a system that processes ILighting components
	*/

	class CLightingSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateLightingSystem(IRenderer*, IGraphicsObjectManager*, E_RESULT_CODE& result);
		public:
			typedef TComponentsQueryLocalSlice<CDirectionalLight, CTransform>                             TDirLightsContext;
			typedef TComponentsQueryLocalSlice<CPointLight, CTransform>                                   TPointLightsContext;
			typedef TComponentsQueryLocalSlice<CSpotLight, CTransform>                                    TSpotLightsContext;
			typedef TComponentsQueryLocalSlice<CShadowCasterComponent, CStaticMeshContainer, CTransform>  TStaticShadowCastersContext;
			typedef TComponentsQueryLocalSlice<CShadowReceiverComponent, CStaticMeshContainer>            TStaticShadowReceiverContext;
			typedef TComponentsQueryLocalSlice<CShadowReceiverComponent, CSkinnedMeshContainer>           TSkinnedShadowReceiverContext;
			typedef TComponentsQueryLocalSlice<CShadowCasterComponent, CSkinnedMeshContainer, CTransform> TSkinnedShadowCastersContext;
		public:
			TDE2_SYSTEM(CLightingSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pRenderer A pointer to IRenderer implementation
				\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager);

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLightingSystem)

			TDE2_API E_RESULT_CODE _prepareResources();
		protected:
			IRenderer*                   mpRenderer = nullptr;

			IGraphicsContext*            mpGraphicsContext = nullptr;

			TPtr<IResourceManager>       mpResourceManager = nullptr;

			IGraphicsObjectManager*      mpGraphicsObjectManager = nullptr;

			TDirLightsContext            mDirectionalLightsContext;
			TPointLightsContext          mPointLightsContext;
			TSpotLightsContext           mSpotLightsContext;

			TStaticShadowCastersContext  mStaticShadowCastersContext;
			TSkinnedShadowCastersContext mSkinnedShadowCastersContext;

			TStaticShadowReceiverContext mStaticShadowReceiversContext;
			TSkinnedShadowReceiverContext mSkinnedShadowReceiversContext;

			IVertexDeclaration*          mpShadowVertDecl = nullptr;
			IVertexDeclaration*          mpSkinnedShadowVertDecl = nullptr;

			TResourceId                  mShadowPassMaterialHandle;
			TResourceId                  mShadowPassSkinnedMaterialHandle;

			CRenderQueue*                mpShadowPassRenderQueue = nullptr;

			std::vector<TLightData>      mActiveLightsData;
	};
}