/*!
	\file CD3D11ShaderCompiler.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/IShaderCompiler.h>
#include <utils/Utils.h>
#include <utils/Types.h>
#include <vector>
#include <unordered_map>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CD3D11ShaderCompiler's type
		
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11ShaderCompiler's implementation
	*/

	TDE2_API IShaderCompiler* CreateD3D11ShaderCompiler(E_RESULT_CODE& result);


	/*!
		class CD3D11ShaderCompiler

		\brief The class represents main compiler of shaders for D3D11 GAPI
	*/

	class CD3D11ShaderCompiler: public IShaderCompiler
	{
		public:
			friend TDE2_API IShaderCompiler* CreateD3D11ShaderCompiler(E_RESULT_CODE& result);
		public:
			typedef std::pair<std::string, std::string>          TShaderDefineDesc;

			typedef std::unordered_map<std::string, std::string> TDefinesMap;

			typedef struct TShaderMetadata
			{
				std::string             mVertexShaderEntrypointName;
				std::string             mPixelShaderEntrypointName;
				std::string             mGeometryShaderEntrypointName;

				E_SHADER_TARGET_VERSION mTargetVersion;

				TDefinesMap             mDefines;
			} TShaderMetadata;

			typedef struct TCompileShaderStageResult
			{
				E_RESULT_CODE   mResultCode;
							    
				std::vector<U8> mByteCode;
			} TCompileShaderStageResult;
		public:
			/*!
				\brief The method initializes an initial state of a compiler

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method compiles specified source code into the bytecode representation

				\param[in] source A string that contains a source code of a shader

				\return An object that contains a result code and an array of bytes
			*/

			TDE2_API TShaderCompilerResult Compile(const std::string& source) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11ShaderCompiler)

			TDE2_API TCompileShaderStageResult _compileShaderStage(E_SHADER_STAGE_TYPE shaderStage, const std::string& source, const std::string& entryPointName,
																   const TDefinesMap& shaderDefinesMap, E_SHADER_TARGET_VERSION targetVersion) const;

			TDE2_API const C8* _getShaderStageDefineName(E_SHADER_STAGE_TYPE shaderStage) const;

			TDE2_API TShaderMetadata _parseShader(const std::string& sourceCode) const;

			TDE2_API std::string _removeComments(const std::string& sourceCode) const;

			TDE2_API TDefinesMap _processDefines(const std::string& sourceCode) const;

			TDE2_API E_SHADER_TARGET_VERSION _getTargetVersionFromStr(const std::string& ver) const;

			TDE2_API bool _isShaderStageEnabled(E_SHADER_STAGE_TYPE shaderStage, const TShaderMetadata& shaderMeta) const;
		protected:
			bool             mIsInitialized;

			static U32       mMaxStepsCount; ///< The value is used within _removeComments to bound a maximum number of steps of an automata

			static const C8* mEntryPointsDefineNames[3];

			static const C8* mTargetVersionDefineName;
	};
}

#endif