/*!
	\file IShader.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Utils.h"
#include "./../utils/Types.h"
#include <string>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IShaderCompiler;


	/*!
		interface IShader

		\brief The interface describes a functionality of a shader instance,
		 which unites all programmable stages of the graphics pipeline
	*/

	class IShader
	{
		public:
			/*!
				\brief The method initializes an internal state of a shader

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\param[in] id An identifier of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, TResourceId id) = 0;

			/*!
				\brief The method compiles specified source code into shader's bytecode using IShaderCompiler's 
				implementation

				\param[in] pShaderCompiler A pointer to IShaderCompiler's implementation
				\param[in] sourceCode A string that contains a source code of a shader (including all its stages)

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Compile(const IShaderCompiler* pShaderCompiler, const std::string& sourceCode) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IShader)
	};
}
