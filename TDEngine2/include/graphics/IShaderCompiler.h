/*!
	\file IShaderCompiler.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../core/IBaseObject.h"
#include "./../utils/CResult.h"
#include <vector>
#include <unordered_map>


namespace TDEngine2
{
	class IFileSystem;


	/*!
		struct TShaderCompilerOutput

		\brief The structure contains shader compiler's output data
	*/

	typedef struct TShaderCompilerOutput
	{
		virtual ~TShaderCompilerOutput() = default;

		std::unordered_map<U8, std::unordered_map<std::string, U8>> mUniformBuffersInfo; /// first key is shader stage, the second is buffer's name, the value is the buffer's slot index 
	} TShaderCompilerOutput, *TShaderCompilerOutputPtr;


	/*!
		interface IShaderCompiler

		\brief The interface represents a functionality of a shaders compiler
	*/

	class IShaderCompiler : public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an initial state of a compiler

				\param[in, out] pFileSystem A pointer to IFileSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IFileSystem* pFileSystem) = 0;

			/*!
				\brief The method compiles specified source code into the bytecode representation.
				Note that the method allocates memory for TShaderCompilerOutput object on heap so it should be
				released manually

				\param[in] source A string that contains a source code of a shader

				\return An object that contains either bytecode or some error code. Note that the
				method allocates memory for TShaderCompilerOutput object on heap so it should be
				released manually
			*/

			TDE2_API virtual TResult<TShaderCompilerOutput*> Compile(const std::string& source) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IShaderCompiler)
	};
}