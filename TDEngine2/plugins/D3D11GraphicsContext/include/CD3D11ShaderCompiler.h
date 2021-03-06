/*!
	\file CD3D11ShaderCompiler.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CBaseShaderCompiler.h>
#include <vector>
#include <unordered_map>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	/*!
		struct TD3D11ShaderCompilerOutput

		\brief The structure contains shader compiler's output data for D3D11 GAPI
	*/

	typedef struct TD3D11ShaderCompilerOutput: public TShaderCompilerOutput
	{
		virtual ~TD3D11ShaderCompilerOutput() = default;

		std::vector<U8> mVSByteCode;

		std::vector<U8> mPSByteCode;

		std::vector<U8> mGSByteCode;
	} TD3D11ShaderCompilerOutput, *TD3D11ShaderCompilerOutputPtr;


	/*!
		\brief A factory function for creation objects of CD3D11ShaderCompiler's type
		
		\param[in, out] pFileSystem A pointer to IFileSystem implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11ShaderCompiler's implementation
	*/

	TDE2_API IShaderCompiler* CreateD3D11ShaderCompiler(IFileSystem* pFileSystem, E_RESULT_CODE& result);


	/*!
		class CD3D11ShaderCompiler

		\brief The class represents main compiler of shaders for D3D11 GAPI
	*/

	class CD3D11ShaderCompiler: public CBaseShaderCompiler
	{
		public:
			friend TDE2_API IShaderCompiler* CreateD3D11ShaderCompiler(IFileSystem* pFileSystem, E_RESULT_CODE& result);

			/*!
				\brief The method compiles specified source code into the bytecode representation.
				Note that the method allocates memory for TShaderCompilerOutput object on heap so it should be
				released manually

				\param[in] source A string that contains a source code of a shader

				\return An object that contains either bytecode or some error code. Note that the
				method allocates memory for TShaderCompilerOutput object on heap so it should be
				released manually
			*/

			TDE2_API TResult<TShaderCompilerOutput*> Compile(const std::string& source) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11ShaderCompiler)

			TDE2_API TResult<std::vector<U8>> _compileShaderStage(E_SHADER_STAGE_TYPE shaderStage, const std::string& source, TShaderMetadata& shaderMetadata, 
																  E_SHADER_FEATURE_LEVEL targetVersion) const;

			TDE2_API TUniformBuffersMap _processUniformBuffersDecls(const TStructDeclsMap& structsMap, CTokenizer& tokenizer) const override;

			TDE2_API E_SHADER_FEATURE_LEVEL _getTargetVersionFromStr(const std::string& ver) const override;

			TDE2_API U32 _getBuiltinTypeSize(const std::string& type) const override;

			TDE2_API TShaderResourcesMap _processShaderResourcesDecls(CTokenizer& tokenizer) const override;

			TDE2_API E_SHADER_RESOURCE_TYPE _isShaderResourceType(const std::string& token) const;
	};
}

#endif