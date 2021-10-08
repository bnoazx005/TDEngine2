/*!
	\file CProjectSettings.h
	\date 07.10.2021
	\author Kasimov Ildar
*/

#pragma once


#include "CBaseObject.h"


namespace TDEngine2
{
	class IArchiveReader;


	/*!
		class CProjectSettings

		\brief The class is a singleton which contains all bunch of project related settings 
	*/

	class CProjectSettings : public CBaseObject
	{
		public:
			TDE2_API E_RESULT_CODE Init(IArchiveReader* pFileReader);

			/*!
				\brief The function is replacement of factory method for instances of this type.
				The only instance will be created per program's lifetime.

				\return A pointer to an instance of CProjectSettings type
			*/

			TDE2_API static TPtr<CProjectSettings> Get();
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CProjectSettings)
		public:
	};
}