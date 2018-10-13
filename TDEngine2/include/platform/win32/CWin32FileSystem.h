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
				\brief The method converts virtual path into a real one. For example, if
				a following virtual directory /vdir/ exists and is binded to c:/data/,
				then input /vdir/foo.txt will be replaced with c:/data/foo.txt.

				\param[in] path A virtual path's value

				\return A string that contains a physical path
			*/

			TDE2_API std::string ResolveVirtualPath(const std::string& path) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CWin32FileSystem)

			TDE2_API std::string _unifyPathView(const std::string& path, bool isVirtualPath = false) const override;

			TDE2_API bool _isPathValid(const std::string& path, bool isVirtualPath = false) const override;

			TDE2_API E_RESULT_CODE _onInit() override;
		protected:
			static C8 mPathSeparator;

			static C8 mAltPathSeparator;
			
			static C8 mForbiddenChars[];
	};
}