/*!
	\file CBaseShader.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/CBaseResource.h"
#include "IShader.h"
#include <vector>


namespace TDEngine2
{
	struct TShaderCompilerOutput;

	class IConstantBuffer;


	/*!
		class CBaseShader

		\brief The class is a common implementation for all platforms
	*/
	
	class CBaseShader: public IShader, public CBaseResource
	{
		public:
			TDE2_REGISTER_TYPE(CBaseShader)

			/*!
				\brief The method initializes an internal state of a shader

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) override;

			/*!
				\brief The method compiles specified source code into shader's bytecode using IShaderCompiler's
				implementation

				\param[in] pShaderCompiler A pointer to IShaderCompiler's implementation
				\param[in] sourceCode A string that contains a source code of a shader (including all its stages)

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Compile(const IShaderCompiler* pShaderCompiler, const std::string& sourceCode) override;

			/*!
				\brief The method loads resource data into memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load() override;

			/*!
				\brief The method unloads resource data from memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Unload() override;
			
			/*!
				\brief The method writes data into a user-defined uniforms buffer with specified register

				\param[in] slot A slot specifies a uniforms buffer

				\param[in] pData A pointer to data that should be written into a buffer

				\param[in] dataSize A size of data in bytes

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetUserUniformsBuffer(U8 slot, const U8* pData, U32 dataSize) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseShader)

			TDE2_API virtual E_RESULT_CODE _createInternalHandlers(const TShaderCompilerOutput* pCompilerData) = 0;

			TDE2_API virtual E_RESULT_CODE _freeUniformBuffers();

			TDE2_API virtual void _bindUniformBuffer(U32 slot, IConstantBuffer* pBuffer) = 0;
		protected:
			IGraphicsContext*             mpGraphicsContext;

			std::string                   mSourceCode;

			std::vector<IConstantBuffer*> mUniformBuffers;
	};
}
