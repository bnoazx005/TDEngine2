/*!
	\file CSaveManager.h
	\date 30.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "ISaveManager.h"
#include "../core/CBaseObject.h"
#include <mutex>


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
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSaveManager)
		protected:
			mutable std::mutex mMutex;

			IFileSystem* mpFileSystem;
	};
}