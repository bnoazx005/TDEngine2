/*!
	\file CVulkanShaderCompiler.h
	\date 03.11.2023
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CBaseShaderCompiler.h>
#include <vector>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CVulkanShaderCompiler's type
		
		\param[in, out] pFileSystem A pointer to IFileSystem implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CVulkanShaderCompiler's implementation
	*/

	TDE2_API IShaderCompiler* CreateVulkanShaderCompiler(IFileSystem* pFileSystem, E_RESULT_CODE& result);


	/*!
		class CVulkanShaderCompiler

		\brief The class represents main compiler of shaders for Vulkan GAPI
	*/

	class CVulkanShaderCompiler: public CBaseShaderCompiler
	{
		public:
			friend TDE2_API IShaderCompiler* CreateVulkanShaderCompiler(IFileSystem*, E_RESULT_CODE&);

			/*!
				\brief The method compiles specified source code into the bytecode representation.
				Note that the method allocates memory for TShaderCompilerOutput object on heap so it should be
				released manually

				\param[in] source A string that contains a source code of a shader

				\return An object that contains either bytecode or some error code. Note that the
				method allocates memory for TShaderCompilerOutput object on heap so it should be
				released manually
			*/

			TDE2_API TResult<TShaderCompilerOutput*> Compile(const std::string& shaderId, const std::string& source) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanShaderCompiler)

			TDE2_API TResult<std::vector<U8>> _compileShaderStage(E_SHADER_STAGE_TYPE shaderStage, const std::string& source, TShaderMetadata& shaderMetadata) const;

			TDE2_API TUniformBuffersMap _processUniformBuffersDecls(const TStructDeclsMap& structsMap, CTokenizer& tokenizer) const override;

			TDE2_API E_SHADER_FEATURE_LEVEL _getTargetVersionFromStr(const std::string& ver) const override;

			TDE2_API USIZE _getBuiltinTypeSize(const std::string& type, const std::function<void(const std::string&)> typeProcessor = nullptr) const override;

			TDE2_API TShaderResourcesMap _processShaderResourcesDecls(CTokenizer& tokenizer) const override;

			TDE2_API E_SHADER_RESOURCE_TYPE _isShaderResourceType(const std::string& token) const;
	};
}