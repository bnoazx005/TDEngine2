/*!
	\file ISaveManager.h
	\date 30.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/IEngineSubsystem.h"
#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../core/Serialization.h"
#include "../core/IBaseObject.h"
#include <type_traits>
#include <string>
#include <functional>


namespace TDEngine2
{
	class IFileSystem;


	/*!
		interface ISaveData
	*/

	class ISaveData : public virtual IBaseObject, public ISerializable
	{
		public:
			/*!
				\brief The method initializes an internal state of the object
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			TDE2_API virtual E_RESULT_CODE SetId(const std::string& saveId) = 0;
			TDE2_API virtual E_RESULT_CODE SetPath(const std::string& savePath) = 0;

			TDE2_API virtual const std::string& GetId() const = 0;
			TDE2_API virtual const std::string& GetPath() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISaveData)
	};


	TDE2_DECLARE_HANDLE_TYPE(TSaveDataId);


	typedef struct TSaveManagerParametersDesc
	{
		IFileSystem* mpFileSystem;

		std::string mUserDataDirectoryPath;
	} TSaveManagerParametersDesc, *TSaveManagerParametersDescPtr;


	/*!
		interface ISaveManager
	*/

	class ISaveManager : public IEngineSubsystem
	{
		public:
			/*!
				\brief The method initializes an internal state of the object

				\param[in] desc An object with data for initialization of the manager

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const TSaveManagerParametersDesc& desc) = 0;

			/*!
				\brief The method creates a new save at given directory

				\param[in] saveId An identifer of the save
				\param[in] path A path to a file wheere the save will be stored
				\param[in] onOverwriteFile A callback is called if the file at given path's already exist, if it returns true the file will be overwritten

				\return A numeric identifier of the save
			*/

			TDE2_API virtual TSaveDataId CreateSaveData(const std::string& saveId, const std::string& path, const std::function<bool()>& onOverwriteFile = nullptr) = 0;

			/*!
				\brief The method removes both storages of the save (from file system and from the manager)

				\param[in] id An identifer of the save

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE RemoveSaveData(TSaveDataId id) = 0;

			/*!
				\brief The method returns a numeric identifier of a save based on its string identifier

				\param[in] A string literal which identifies the save

				\return A numeric identifier of the save
			*/

			TDE2_API virtual TSaveDataId FindSaveData(const std::string& saveId) const = 0;

			/*!
				\brief The method returns a pointer to a save data by an identifier
				
				\param[in] id An identifier of a save data

				\return A pointer to ISaveData implementation
			*/

			TDE2_API virtual ISaveData* GetSaveData(TSaveDataId id) const = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return E_ENGINE_SUBSYSTEM_TYPE::EST_SAVE_MANAGER; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISaveManager)
	};
}