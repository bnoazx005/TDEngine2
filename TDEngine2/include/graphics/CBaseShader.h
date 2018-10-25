/*!
	\file CBaseShader.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/CBaseResource.h"
#include "IShader.h"


namespace TDEngine2
{
	/*!
		class CBaseShader

		\brief The class is a common implementation for all platforms
	*/
	
	class CBaseShader: public IShader, public CBaseResource<CBaseShader>
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

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, TResourceId id) override;

			/*!
				\brief The method compiles specified source code into shader's bytecode using IShaderCompiler's
				implementation

				\param[in] pShaderCompiler A pointer to IShaderCompiler's implementation
				\param[in] sourceCode A string that contains a source code of a shader (including all its stages)

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Compile(const IShaderCompiler* pShaderCompiler, const std::string& sourceCode) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseShader)

			TDE2_API virtual E_RESULT_CODE _createInternalHandlers(const TShaderCompilerResult& shaderBytecode) = 0;
		protected:
			IGraphicsContext* mpGraphicsContext;

			std::string       mSourceCode;
	};
}
