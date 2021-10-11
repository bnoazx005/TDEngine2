/*!
	\file IDLLManager.h
	\date 04.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IBaseObject.h"
#include "../utils/Utils.h"
#include <string>


namespace TDEngine2
{
	/*!
		interface IDLLManager

		\brief The interface describes a functionality of a dynamic libraries manager
	*/

	class IDLLManager : public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method loads specified library into the memory and returns its handler

				\param[in] filename A filename without file extension
				\param[out] result The output contains RC_OK if everything went ok, or some other code, which describes an error

				\return A handler of a dynamic library (if it wasn't load then the handler equals to InvalidDynamicLibHandlerValue
			*/

			TDE2_API virtual TDynamicLibraryHandler Load(const std::string& filename, E_RESULT_CODE& result) = 0;

			/*!
				\brief The method unloads specified library from the memory

				\param[in] filename A filename without file extension

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Unload(const std::string& filename) = 0;

			/*!
				\brief The method unloads specified library from the memory

				\param[out] libraryHandler A handler of a dynamic library (if it wasn't load then the handler equals to InvalidDynamicLibHandlerValue

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Unload(TDynamicLibraryHandler& libraryHandler) = 0;

			/*!
				\brief The method unloads all loaded libraries

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE UnloadAll() = 0;

			/*!
				\brief The method returns a pointer to a symbol, which is stored within specified library

				\param[in] libraryHandler A handler of a dynamic library
				\param[in] symbol A symbol's name

				\return A pointer to a symbol
			*/

			TDE2_API virtual void* GetSymbol(const TDynamicLibraryHandler& libraryHandler, const std::string& symbol) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IDLLManager)
	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(IDLLManager)
}
