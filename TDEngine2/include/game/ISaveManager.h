/*!
	\file ISaveManager.h
	\date 30.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/IEngineSubsystem.h"
#include "../utils/Types.h"
#include "../utils/Utils.h"
#include <type_traits>
#include <string>


namespace TDEngine2
{
	class IFileSystem;


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

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return E_ENGINE_SUBSYSTEM_TYPE::EST_SAVE_MANAGER; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISaveManager)
	};
}