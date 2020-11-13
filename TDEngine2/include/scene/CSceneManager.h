/*!
	\file CSceneManager.h
	\date 13.11.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "ISceneManager.h"
#include "../core/CBaseObject.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CSceneManager's type

		\param[in, out] pFileSystem A pointer to IFileSystem implementation
		\param[in, out] pWorld A pointer to IWorld which is a global game state
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFileInputStream's implementation
	*/

	TDE2_API ISceneManager* CreateSceneManager(IFileSystem* pFileSystem, IWorld* pWorld, E_RESULT_CODE& result);


	class CSceneManager : public CBaseObject, public ISceneManager
	{
		public:
			friend TDE2_API ISceneManager* CreateSceneManager(IFileSystem*, IWorld*, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes the internal state of the object

				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in, out] pWorld A pointer to IWorld which is a global game state

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IFileSystem* pFileSystem, IWorld* pWorld) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

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

				\return Either a handle of created scene or an error code
			*/

			TDE2_API TResult<TSceneId> CreateScene() override;

			/*!
				\brief The method removes an instance of a scene with given identifier only if the scene was created
				using CreateScene() method. In other case its behaviour is the same as UnloadScene()

				\param[in] id An identifier of a scene

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RemoveScene(TSceneId id) override;

			/*!
				\brief The method is used to retrieve a pointer to a scene by its handle

				\param[in] id An identifier of a scene

				\return Returns either a pointer to IScene or an error code
			*/

			TDE2_API TResult<IScene*> GetScene(TSceneId id) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSceneManager)
		protected:
			IFileSystem* mpFileSystem;
			IWorld* mpWorld;
	};
}
