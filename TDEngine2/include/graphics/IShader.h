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

			/*!
				\brief The method binds a shader to a rendering pipeline
			*/

			TDE2_API virtual void Bind() = 0;

			/*!
				\brief The method rejects a shader from a rendering pipeline
			*/

			TDE2_API virtual void Unbind() = 0;

			/*!
				\brief The method writes data into an internal uniforms buffer with specified register

				\param[in] slot A slot specifies a uniforms buffer, in which data will be written

				\param[in] pData A pointer to data that should be written into a buffer
				
				\param[in] dataSize A size of data in bytes
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetInternalUniformsBuffer(E_INTERNAL_UNIFORM_BUFFER_REGISTERS slot, const U8* pData, U32 dataSize) = 0;

			/*!
				\brief The method writes data into a user-defined uniforms buffer with specified register

				\param[in] slot A slot specifies a uniforms buffer

				\param[in] pData A pointer to data that should be written into a buffer

				\param[in] dataSize A size of data in bytes

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetUserUniformsBuffer(U8 slot, const U8* pData, U32 dataSize) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IShader)
	};
}
