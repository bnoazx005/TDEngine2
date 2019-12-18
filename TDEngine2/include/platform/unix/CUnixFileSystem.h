/*!
	\file CUnixFileSystem.h
	\date 12.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../../core/CBaseFileSystem.h"


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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CUnixFileSystem)

			TDE2_API bool _isPathValid(const std::string& path, bool isVirtualPath = false) const override;

			TDE2_API E_RESULT_CODE _onInit() override;

			TDE2_API const C8& _getPathSeparatorChar() const override;
			TDE2_API const C8& _getAltPathSeparatorChar() const override;
		protected:
			static C8 mPathSeparator;

			static C8 mAltPathSeparator;

			static C8 mForbiddenChars[];
	};
}