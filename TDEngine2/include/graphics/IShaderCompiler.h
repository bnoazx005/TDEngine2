/*!
	\file IShaderCompiler.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../core/IBaseObject.h"
#include <vector>


namespace TDEngine2
{
	/*!
		interface IShaderCompiler

		\brief The interface represents a functionality of a shaders compiler
	*/

	class IShaderCompiler : public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an initial state of a compiler

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method compiles specified source code into the bytecode representation

				\param[in] source A string that contains a source code of a shader

				\return An object that contains a result code and an array of bytes
			*/

			TDE2_API virtual TShaderCompilerResult Compile(const std::string& source) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IShaderCompiler)
	};
}