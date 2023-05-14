/*!
	\file ISceneManager.h
	\date 13.11.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Utils.h"
#include "../utils/Types.h"
#include "../core/IEngineSubsystem.h"
#include "../core/IBaseObject.h"
#include <functional>
#include <vector>


namespace TDEngine2
{
	class IScene;
	class IFileSystem;
	class IWorld;
	class IPrefabsRegistry;

	enum class TSceneId : U32;
	enum class TEntityId : U32;


	TDE2_DECLARE_SCOPED_PTR(IFileSystem)
	TDE2_DECLARE_SCOPED_PTR(IWorld)
	TDE2_DECLARE_SCOPED_PTR(IPrefabsRegistry)


	/*!
		struct TSceneManagerSettings

		\brief The type contains all information that the scene manager is needed for correct
		start up of itself
	*/

	typedef struct TSceneManagerSettings
	{
		std::string mMainScenePath = "MainScene.scene";
	} TSceneManagerSettings, *TSceneManagerSettingsPtr;


	/*!
		interface ISceneManager

		\brief The interface represents functionality of a scene management facility. In short
		this is the same as a resource manager, but the very specific applied.

		A first loaded scene is always assumed as main and persistent
	*/

	class ISceneManager: public virtual IBaseObject, public IEngineSubsystem
	{
		public:
			typedef std::function<void(const TResult<TSceneId>&)> TLoadSceneCallback;
			
			typedef std::vector<IScene*> TScenesArray;

		public:
			/*!
				\brief The method initializes the internal state of the object

				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in, out] pWorld A pointer to IWorld which is a global game state
				\param[in, out] pPrefabsRegistry A pointer to IPrefabsRegistry 
				\param[in] settings Start up settings

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TPtr<IFileSystem> pFileSystem, TPtr<IWorld> pWorld, TPtr<IPrefabsRegistry> pPrefabsRegistry, const TSceneManagerSettings& settings) = 0;

			/*!
				\brief The method loads a scene based on path to that

				\param[in] scenePath A path to a scene's file

				\return Either a handle of loaded scene or an error code
			*/

			TDE2_API virtual TResult<TSceneId> LoadScene(const std::string& scenePath) = 0;

			/*!
				\brief The method loads in asynchronous manner a scene based on path to that

				\param[in] scenePath A path to a scene's file
				\param[in] onResultCallback A callback is called when either the scene is loaded or some error's happened

				\return Either a handle of loaded scene or an error code
			*/

			TDE2_API virtual void LoadSceneAsync(const std::string& scenePath, const TLoadSceneCallback& onResultCallback) = 0;

#if TDE2_EDITORS_ENABLED
			TDE2_API virtual E_RESULT_CODE SaveSceneChunk(const std::string& path, TSceneId sceneId) = 0;
#endif

			/*!
				\brief The method unloads all resources that're related with the given scene

				\param[in] id An identifier of a scene

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE UnloadScene(TSceneId id) = 0;

			/*!
				\brief The method creates a new instance of a scene

				\param[in] name An identifier of a scene

				\return Either a handle of created scene or an error code
			*/
			
			TDE2_API virtual TResult<TSceneId> CreateScene(const std::string& name) = 0;

			/*!
				\brief The method is used to retrieve a pointer to a scene by its handle

				\param[in] id An identifier of a scene

				\return Returns either a pointer to IScene or an error code
			*/

			TDE2_API virtual TResult<IScene*> GetScene(TSceneId id) const = 0;

			TDE2_API virtual TSceneId GetSceneId(const std::string& id) const = 0;

			/*!
				\return The method returns scene's pointer by entity's identifier if it belongs to the scene.
			*/

			TDE2_API virtual IScene* GetSceneByEntityId(TEntityId entityId) const = 0;

			/*!
				\return The method returns a pointer to instance of IWorld
			*/

			TDE2_API virtual TPtr<IWorld> GetWorld() const = 0;

			TDE2_API virtual TPtr<IPrefabsRegistry> GetPrefabsRegistry() const = 0;

			/*!
				\return The method returns an array of currently loaded chunks. An individual elements can be nullptr if there was unload operation's invocations
			*/

			TDE2_API virtual const TScenesArray& GetLoadedScenes() const = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_SCENE_MANAGER; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISceneManager)
	};


	TDE2_DECLARE_SCOPED_PTR(ISceneManager)
}
