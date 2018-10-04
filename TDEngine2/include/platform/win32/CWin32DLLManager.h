/*!
	\file CWin32DLLManager.h
	\date 04.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../../core/IDLLManager.h"
#include "./../../core/CBaseObject.h"
#include <unordered_map>
#include <vector>
#include <list>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	/*!
		class CWin32DLLManager

		\brief The class implements IDLLManager functionality for Win32 plaftorm
	*/

	class CWin32DLLManager : public IDLLManager, public CBaseObject
	{
		public:
			friend TDE2_API IDLLManager* CreateWin32DLLManager(E_RESULT_CODE& result);
		protected:
			typedef std::unordered_map<std::string, TDynamicLibraryHandler> TDynLibHandlersMap;

			typedef std::vector<TDynamicLibrary>                            TDynLibArray;
		public:
			/*!
				\brief The method initializes the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method loads specified library into the memory and returns its handler

				\param[in] filename A filename without file extension
				\param[out] result The output contains RC_OK if everything went ok, or some other code, which describes an error

				\return A handler of a dynamic library (if it wasn't load then the handler equals to InvalidDynamicLibHandlerValue
			*/

			TDE2_API TDynamicLibraryHandler Load(const std::string& filename, E_RESULT_CODE& result) override;

			/*!
				\brief The method unloads specified library from the memory

				\param[in] filename A filename without file extension

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Unload(const std::string& filename) override;

			/*!
				\brief The method unloads specified library from the memory

				\param[out] libraryHandler A handler of a dynamic library (if it wasn't load then the handler equals to InvalidDynamicLibHandlerValue

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Unload(TDynamicLibraryHandler& libraryHandler) override;

			/*!
				\brief The method unloads all loaded libraries

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnloadAll() override;

			/*!
				\brief The method returns a pointer to a symbol, which is stored within specified library

				\param[in] libraryHandler A handler of a dynamic library
				\param[in] symbol A symbol's name

				\return A pointer to a symbol
			*/

			TDE2_API void* GetSymbol(const TDynamicLibraryHandler& libraryHandler, const std::string& symbol) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CWin32DLLManager)
		protected:
			TDynLibHandlersMap                mHandlersTable;
			TDynLibArray                      mLoadedLibraries;
			TDynamicLibraryHandler            mNextFreeHandler;
			std::list<TDynamicLibraryHandler> mFreeHandlersList;
	};

	/*!
		\brief A factory function for creation objects of CWin32DLLManager's type.

		\return A pointer to CWin32DLLManager's implementation
	*/

	TDE2_API IDLLManager* CreateWin32DLLManager(E_RESULT_CODE& result);
}

#endif