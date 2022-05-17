/*!
	\file IScene.h
	\date 14.11.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Utils.h"
#include "../utils/Types.h"
#include "../utils/Color.h"
#include "../core/IBaseObject.h"
#include "../core/Serialization.h"
#include "../math/TVector3.h"


namespace TDEngine2
{
	class IWorld;
	class IResourceManager;
	class CEntity;
	struct TBaseCameraParameters;


	enum class E_CAMERA_PROJECTION_TYPE : U8;


	TDE2_DECLARE_HANDLE_TYPE(TSceneId);

	constexpr TSceneId MainScene = TSceneId(0);


	TDE2_DECLARE_SCOPED_PTR(IWorld)


	/*!
		interface IScene

		\brief The interface represents API of a world chunk aka game scene
	*/

	class IScene: public virtual IBaseObject, public ISerializable
	{
		public:
			/*!
				\brief The method initializes the internal state of the object

				\param[in, out] pWorld A pointer to IWorld 
				\param[in] id A name of a scene, should be globally unique
				\param[in] scenePath A path to a scene's serialized data
				\param[in] isMainScene The flag tells whether the scene is main or not. There could be the only one main scene at the game

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TPtr<IWorld> pWorld, const std::string& id, const std::string& scenePath, bool isMainScene) = 0;

			/*!
				\brief The method creates a new entity which belongs to the scene

				\param[in] name String identifier of the entity

				\return A pointer to created entity
			*/

			TDE2_API virtual CEntity* CreateEntity(const std::string& name) = 0;

			/*!
				\brief The method removes an entity if there is one that belongs to the scene

				\param[in] id A numerical identifier of the entity. The entity should belong to the current scene

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE RemoveEntity(TEntityId id) = 0;

			/*!
				\brief The method is just a helper that sequentially creates a new entity and adds a light component
				with given set of parameters

				\param[in] tint A color of the light source
				\param[in] intensity A power of contribution of the light source into the scene
				\param[in] direction A direction of the light, could be non-normalized vector 

				\return A pointer to a new created entity with CDirectionalLight component attached to it
			*/

			TDE2_API virtual CEntity* CreateDirectionalLight(const TColor32F& tint, F32 intensity, const TVector3& direction) = 0;

			/*!
				\brief The method is a helper that creates a new entity with PointLightComponent's attached to it

				\param[in] tint A color of the light source
				\param[in] intensity A power of contribution of the light source into the scene
				\param[in] range A range of a point light

				\return A pointer to a new created entity with CPointLightComponent component attached to it
			*/

			TDE2_API virtual CEntity* CreatePointLight(const TColor32F& tint, F32 intensity, F32 range) = 0;

			/*!
				\brief The method creates an entity which represents a skybox

				\param[in, out] pResourceManager A pointer to IResourceManager implementation
				\param[in] skyboxTexture A path to cube texure of a skybox

				\return A pointer to a new created entity with CDirectionalLight component attached to it
			*/

			TDE2_API virtual CEntity* CreateSkybox(IResourceManager* pResourceManager, const std::string& skyboxTexture) = 0;

#if TDE2_EDITORS_ENABLED
			TDE2_API virtual CEntity* CreateEditorCamera(F32 aspect, F32 fov) = 0;
#endif

			TDE2_API virtual CEntity* CreateCamera(const std::string& id, E_CAMERA_PROJECTION_TYPE cameraType, const TBaseCameraParameters& params) = 0;

			TDE2_API virtual CEntity* Spawn(const std::string& prefabId, const CEntity* pParentEntity = nullptr) = 0;

			/*!
				\brief The method iterates over each entity which is linked to current scene

				\param[in] action A predicate that's executed for each valid entity. All expired or invalid entities are skipped
			*/

			TDE2_API virtual void ForEachEntity(const std::function<void(CEntity*)>& action = nullptr) = 0;

			/*!
				\return The method returns name of the scene
			*/

			TDE2_API virtual const std::string& GetName() const = 0;

			/*!
				\return The method returns a filepath of the scene
			*/

			TDE2_API virtual const std::string& GetScenePath() const = 0;

			TDE2_API virtual bool IsMainScene() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IScene)
	};
}