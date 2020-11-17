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


namespace TDEngine2
{
	class IScene;
	class IFileSystem;
	class IWorld;

	enum class TSceneId : U32;



	/*!
		interface ISceneManager

		\brief The interface represents functionality of a scene management facility. In short
		this is the same as a resource manager, but the very specific applied
	*/

	class ISceneManager: public virtual IBaseObject, public IEngineSubsystem
	{
		public:
			typedef std::function<TResult<TSceneId>> TLoadSceneCallback;
		public:
			/*!
				\brief The method initializes the internal state of the object

				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in, out] pWorld A pointer to IWorld which is a global game state

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IFileSystem* pFileSystem, IWorld* pWorld) = 0;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Free() = 0;

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
				\brief The method removes an instance of a scene with given identifier only if the scene was created
				using CreateScene() method. In other case its behaviour is the same as UnloadScene()

				\param[in] id An identifier of a scene

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE RemoveScene(TSceneId id) = 0;

			/*!
				\brief The method is used to retrieve a pointer to a scene by its handle

				\param[in] id An identifier of a scene

				\return Returns either a pointer to IScene or an error code
			*/

			TDE2_API virtual TResult<IScene*> GetScene(TSceneId id) const = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_SCENE_MANAGER; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISceneManager)
	};
}
