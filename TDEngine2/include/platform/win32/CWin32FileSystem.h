/*!
	\file CWin32FileSystem.h
	\date 08.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../../core/CBaseFileSystem.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CWin32FileSystem's type

		\return A pointer to CWin32FileSystem's implementation
	*/

	TDE2_API IFileSystem* CreateWin32FileSystem(E_RESULT_CODE& result);


	/*!
		class CWin32FileSystem

		\brief The class is an implementation of IFileSystem interface for
		Win32 platform
	*/

	class CWin32FileSystem : public CBaseFileSystem
	{
		public:
			friend TDE2_API IFileSystem* CreateWin32FileSystem(E_RESULT_CODE& result);
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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CWin32FileSystem)
			
			TDE2_API bool IsPathValid(const std::string& path, bool isVirtualPath = false) const override;

			TDE2_API E_RESULT_CODE _onInit() override;
		protected:
			static C8 mPathSeparator;

			static C8 mAltPathSeparator;
			
			static C8 mForbiddenChars[];
	};
}