/*!
	\file CScene.h
	\date 14.11.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "IScene.h"
#include "../core/CBaseObject.h"
#include "IPrefabsRegistry.h"
#include <mutex>
#include <vector>


namespace TDEngine2
{
	class IWorld;
	class IScene;


	TDE2_DECLARE_SCOPED_PTR(IWorld);


	/*!
		\brief A factory function for creation objects of CScene's type

		\param[in, out] pWorld A pointer to IWorld
		\param[in, out] pPrefabsRegistry A pointer to IPrefabsRegistry 
		\param[in] id A name of a scene, should be globally unique
		\param[in] isMainScene The flag tells whether the scene is main or not
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CScene's implementation
	*/

	TDE2_API IScene* CreateScene(TPtr<IWorld> pWorld, TPtr<IPrefabsRegistry> pPrefabsRegistry, const std::string& id, const std::string& scenePath, bool isMainScene, E_RESULT_CODE& result);


	class CScene: public CBaseObject, public IScene
	{
		public:
			friend TDE2_API IScene* CreateScene(TPtr<IWorld>, TPtr<IPrefabsRegistry>, const std::string&, const std::string&, bool, E_RESULT_CODE&);
		public:
			typedef std::vector<TEntityId> TEntitiesRegistry;
		public:
			/*!
				\brief The method initializes the internal state of the object

				\param[in, out] pWorld A pointer to IWorld
				\param[in, out] pPrefabsRegistry A pointer to IPrefabsRegistry 
				\param[in] id A name of a scene, should be globally unique
				\param[in] scenePath A path to a scene's serialized data
				\param[in] isMainScene The flag tells whether the scene is main or not

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(TPtr<IWorld> pWorld, TPtr<IPrefabsRegistry> pPrefabsRegistry, const std::string& id, const std::string& scenePath, bool isMainScene) override;

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

			/*!
				\brief The method creates a new entity which belongs to the scene

				\param[in] name String identifier of the entity

				\return A pointer to created entity
			*/

			TDE2_API CEntity* CreateEntity(const std::string& name) override;

			TDE2_API CEntity* CreateEntityWithUUID(TEntityId id) override;

			/*!
				\brief The method removes an entity if there is one that belongs to the scene

				\param[in] id A numerical identifier of the entity. The entity should belong to the current scene

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RemoveEntity(TEntityId id) override;

			/*!
				\brief The method is just a helper that sequentially creates a new entity and adds a light component
				with given set of parameters
				
				\param[in] tint A color of the light source
				\param[in] instentisy A power of contribution of the light source into the scene
				\param[in] direction A direction of the light, could be non-normalized vector

				\return A pointer to a new created entity with CDirectionalLight component attached to it
			*/

			TDE2_API CEntity* CreateDirectionalLight(const TColor32F& tint, F32 intensity, const TVector3& direction) override;

			/*!
				\brief The method is a helper that creates a new entity with PointLightComponent's attached to it

				\param[in] tint A color of the light source
				\param[in] intensity A power of contribution of the light source into the scene
				\param[in] range A range of a point light

				\return A pointer to a new created entity with CPointLightComponent component attached to it
			*/

			TDE2_API CEntity* CreatePointLight(const TColor32F& tint, F32 intensity, F32 range) override;

			/*!
				\brief The method creates an entity which represents a skybox

				\param[in, out] pResourceManager A pointer to IResourceManager implementation
				\param[in] skyboxTexture A path to cube texure of a skybox

				\return A pointer to a new created entity with CDirectionalLight component attached to it
			*/

			TDE2_API CEntity* CreateSkybox(IResourceManager* pResourceManager, const std::string& skyboxTexture) override;

#if TDE2_EDITORS_ENABLED
			TDE2_API CEntity* CreateEditorCamera(F32 aspect, F32 fov) override;
#endif

			TDE2_API CEntity* CreateCamera(const std::string& id, E_CAMERA_PROJECTION_TYPE cameraType, const TBaseCameraParameters& params) override;
			
			/*!
				\brief The method instantiates a new copy of specified prefab's hierarchy

				\param[in] id An identifier of a prefab which is defined in prefabs collection
				\param[in, out] pParent A pointer which the new instantiated tree will be attached to

				\return A pointer to a root entity of a prefab's instance
			*/

			TDE2_API CEntity* Spawn(const std::string& prefabId, CEntity* pParentEntity = nullptr, TEntityId prefabLinkUUID = TEntityId::Invalid) override;

			/*!
				\brief The method iterates over each entity which is linked to current scene

				\param[in] action A predicate that's executed for each valid entity. All expired or invalid entities are skipped
			*/

			TDE2_API void ForEachEntity(const std::function<void(CEntity*)>& action = nullptr) override;
			
			/*!
				\brief The method traverses the hierarchy of the scene beginning from pRoot entity based on path's value.
				The path consists of entities names separated with slashes for instance
				entity_1/entity_2/.../entity_N
			*/

			TDE2_API CEntity* FindEntityByPath(const std::string& path, CEntity* pRoot) override;

			/*!
				\return The method returns name of the scene
			*/

			TDE2_API const std::string& GetName() const override;

			/*!
				\return The method returns a filepath of the scene
			*/

			TDE2_API const std::string& GetScenePath() const override;

			TDE2_API bool IsMainScene() const override;

			TDE2_API const std::vector<TEntityId>& GetEntities() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CScene)
			
			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			static constexpr U32 mAssetVersionId = 0;

			mutable std::mutex mMutex;

			TPtr<IWorld> mpWorld;
			TPtr<IPrefabsRegistry> mpPrefabsRegistry;

			std::string mName;
			std::string mPath;

			bool mIsMainScene;

			TEntityId mSceneInfoEntityId;

			TEntitiesRegistry mEntities;
	};


	class CSceneLoader
	{
		public:
			TDE2_API static E_RESULT_CODE LoadScene(IArchiveReader* pReader, IWorld* pWorld, IScene* pScene);

			TDE2_API static TResult<IPrefabsRegistry::TPrefabInfoEntity> LoadPrefab(
				IArchiveReader* pReader,
				CEntityManager* pEntityManager,
				const IPrefabsRegistry::TEntityFactoryFunctor& entityFactory, 
				const IPrefabsRegistry::TPrefabFactoryFunctor& prefabFactory);
	};


	class CSceneSerializer
	{
		public:
			TDE2_API static E_RESULT_CODE SaveScene(IArchiveWriter* pWriter, TPtr<IWorld> pWorld, IScene* pScene);
			TDE2_API static E_RESULT_CODE SavePrefab(IArchiveWriter* pWriter, TPtr<IWorld> pWorld, CEntity* pRootEntity);
	};
}