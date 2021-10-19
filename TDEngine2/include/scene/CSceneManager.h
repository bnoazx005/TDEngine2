/*!
	\file CSceneManager.h
	\date 13.11.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "ISceneManager.h"
#include "../core/CBaseObject.h"
#include <mutex>
#include <vector>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CSceneManager's type

		\param[in, out] pFileSystem A pointer to IFileSystem implementation
		\param[in, out] pWorld A pointer to IWorld which is a global game state
		\param[in] settings Start up settings
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFileInputStream's implementation
	*/

	TDE2_API ISceneManager* CreateSceneManager(TPtr<IFileSystem> pFileSystem, TPtr<IWorld> pWorld, const TSceneManagerSettings& settings, E_RESULT_CODE& result);


	class CSceneManager : public CBaseObject, public ISceneManager
	{
		public:
			friend TDE2_API ISceneManager* CreateSceneManager(TPtr<IFileSystem>, TPtr<IWorld>, const TSceneManagerSettings&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes the internal state of the object

				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in, out] pWorld A pointer to IWorld which is a global game state
				\param[in] settings Start up settings

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(TPtr<IFileSystem> pFileSystem, TPtr<IWorld> pWorld, const TSceneManagerSettings& settings) override;

			/*!
				\brief The method loads a scene based on path to that

				\param[in] scenePath A path to a scene's file

				\return Either a handle of loaded scene or an error code
			*/

			TDE2_API TResult<TSceneId> LoadScene(const std::string& scenePath) override;

			/*!
				\brief The method loads in asynchronous manner a scene based on path to that

				\param[in] scenePath A path to a scene's file
				\param[in] onResultCallback A callback is called when either the scene is loaded or some error's happened

				\return Either a handle of loaded scene or an error code
			*/

			TDE2_API void LoadSceneAsync(const std::string& scenePath, const TLoadSceneCallback& onResultCallback) override;

			/*!
				\brief The method unloads all resources that're related with the given scene

				\param[in] id An identifier of a scene

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnloadScene(TSceneId id) override;

			/*!
				\brief The method creates a new instance of a scene

				\param[in] name An identifier of a scene

				\return Either a handle of created scene or an error code
			*/

			TDE2_API TResult<TSceneId> CreateScene(const std::string& name) override;
			
			/*!
				\brief The method is used to retrieve a pointer to a scene by its handle

				\param[in] id An identifier of a scene. 0th id is an identifier of a main scene

				\return Returns either a pointer to IScene or an error code
			*/

			TDE2_API TResult<IScene*> GetScene(TSceneId id) const override;

			TDE2_API TSceneId GetSceneId(const std::string& id) const override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const { return E_ENGINE_SUBSYSTEM_TYPE::EST_SCENE_MANAGER; }

			/*!
				\return The method returns a pointer to instance of IWorld
			*/

			TDE2_API TPtr<IWorld> GetWorld() const override;

			/*!
				\return The method returns an array of currently loaded chunks
			*/

			TDE2_API const TScenesArray& GetLoadedScenes() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSceneManager)

			TDE2_API TResult<TSceneId> _createInternal(const std::string& name);
			TDE2_API TResult<TSceneId> _registerSceneInternal(const std::string& name, IScene* pScene);
			TDE2_API E_RESULT_CODE _unregisterSceneInternal(TSceneId id);

			TDE2_API TResult<TSceneId> _loadSceneInternal(const std::string& scenePath);

			TDE2_API E_RESULT_CODE _onPostInit();

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			mutable std::mutex mMutex;

			TPtr<IFileSystem>  mpFileSystem;
			TPtr<IWorld>       mpWorld;

			TSceneManagerSettings mSettings;

			TScenesArray mpScenes;
	};
}
