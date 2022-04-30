/*!
	\file CUnixFileSystem.h
	\date 12.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../../core/CBaseFileSystem.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CUnixFileSystem's type

		\return A pointer to CUnixFileSystem's implementation
	*/

	TDE2_API IFileSystem* CreateUnixFileSystem(E_RESULT_CODE& result);


	/*!
		class CUnixFileSystem

		\brief The class is an implementation of IFileSystem interface for
		UNIX platform
	*/

	class CUnixFileSystem : public CBaseFileSystem
	{
		public:
			friend TDE2_API IFileSystem* CreateUnixFileSystem(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method returns a path separator charater
				\return The method returns a path separator charater
			*/

			TDE2_API const C8& GetPathSeparatorChar() const override;

			/*!
				\brief The method returns an alternative version of a path separator charater
				\return The method returns an alternative version of a path separator charater
			*/

			TDE2_API const C8& GetAltPathSeparatorChar() const override;

			/*!
				\brief Under Linux the method returns path to /var/lib/appname directory. If there is no
				specified directory creates one

				\return The method returns a path to application data directory
			*/

			TDE2_API std::string GetApplicationDataPath() const override;

			/*!
				\return The method returns a path of a user's directory
			*/

			TDE2_API std::string GetUserDirectory() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CUnixFileSystem)

			TDE2_API bool IsPathValid(const std::string& path, bool isVirtualPath = false) const override;

			TDE2_API E_RESULT_CODE _onInit() override;
		protected:
			static C8 mPathSeparator;

			static C8 mAltPathSeparator;

			static C8 mForbiddenChars[];
	};
}