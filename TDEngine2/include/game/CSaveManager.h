/*!
	\file CSaveManager.h
	\date 30.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "ISaveManager.h"
#include "../core/CBaseObject.h"
#include <mutex>
#include <unordered_map>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CLocalizationManager's type

		\param[in] desc An object with data for initialization of the manager
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CLocalizationManager's implementation
	*/

	TDE2_API ISaveManager* CreateSaveManager(const TSaveManagerParametersDesc& desc, E_RESULT_CODE& result);



	/*!
		class CSaveManager

		\brief The general implementation of a save manager
	*/

	class CSaveManager : public CBaseObject, public ISaveManager
	{
		public:
			friend TDE2_API ISaveManager* CreateSaveManager(const TSaveManagerParametersDesc&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of the object

				\param[in] desc An object with data for initialization of the manager

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(const TSaveManagerParametersDesc& desc) override;

			/*!
				\brief The method creates a new save at given directory

				\param[in] saveId An identifer of the save
				\param[in] path A path to a file wheere the save will be stored
				\param[in] onOverwriteFile A callback is called if the file at given path's already exist, if it returns true the file will be overwritten

				\return A numeric identifier of the save
			*/

			TDE2_API TSaveDataId CreateSaveData(const std::string& saveId, const std::string& path, const std::function<bool()>& onOverwriteFile = nullptr) override;

			/*!
				\brief The method removes both storages of the save (from file system and from the manager)

				\param[in] id An identifer of the save

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RemoveSaveData(TSaveDataId id) override;

			// \todo StoreSaveData 

			/*!
				\brief The method returns a numeric identifier of a save based on its string identifier

				\param[in] A string literal which identifies the save

				\return A numeric identifier of the save
			*/

			TDE2_API TSaveDataId FindSaveData(const std::string& saveId) const override;

			/*!
				\brief The method returns a pointer to a save data by an identifier

				\param[in] id An identifier of a save data

				\return A pointer to ISaveData implementation
			*/

			TDE2_API ISaveData* GetSaveData(TSaveDataId id) const override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSaveManager)

			TDE2_API E_RESULT_CODE _scanAndLoadSaveFiles(const std::string& path);

			TDE2_API E_RESULT_CODE _freeSaveGamesObjects();			

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			mutable std::mutex mMutex;

			IFileSystem* mpFileSystem;

			std::unordered_map<TSaveDataId, ISaveData*> mpSaves;
	};
}