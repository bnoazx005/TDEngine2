/*!
	\file CScene.h
	\date 14.11.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "IScene.h"
#include "../core/CBaseObject.h"
#include <mutex>
#include <vector>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CScene's type

		\param[in, out] pWorld A pointer to IWorld
		\param[in] id A name of a scene, should be globally unique
		\param[in] isMainScene The flag tells whether the scene is main or not
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CScene's implementation
	*/

	TDE2_API IScene* CreateScene(IWorld* pWorld, const std::string& id, const std::string& scenePath, bool isMainScene, E_RESULT_CODE& result);


	class CScene: public CBaseObject, public IScene
	{
		public:
			friend TDE2_API IScene* CreateScene(IWorld*, const std::string&, const std::string&, bool, E_RESULT_CODE&);
		public:
			typedef std::vector<TEntityId> TEntitiesRegistry;
		public:
			/*!
				\brief The method initializes the internal state of the object

				\param[in, out] pWorld A pointer to IWorld
				\param[in] id A name of a scene, should be globally unique
				\param[in] scenePath A path to a scene's serialized data
				\param[in] isMainScene The flag tells whether the scene is main or not

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IWorld* pWorld, const std::string& id, const std::string& scenePath, bool isMainScene) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

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
				\brief The method iterates over each entity which is linked to current scene

				\param[in] action A predicate that's executed for each valid entity. All expired or invalid entities are skipped
			*/

			TDE2_API void ForEachEntity(const std::function<void(CEntity*)>& action = nullptr) override;

			/*!
				\return The method returns name of the scene
			*/

			TDE2_API const std::string GetName() const override;

			TDE2_API bool IsMainScene() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CScene)
		protected:
			static constexpr U32 mAssetVersionId = 0;

			mutable std::mutex mMutex;

			IWorld* mpWorld;

			std::string mName;
			std::string mPath;

			bool mIsMainScene;

			TEntitiesRegistry mEntities;
	};
}