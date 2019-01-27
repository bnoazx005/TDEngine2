/*!
	\file CBaseShaderCompiler.h
	\date 23.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IShaderCompiler.h"
#include "./../utils/Utils.h"
#include "./../utils/Types.h"
#include "./../utils/CResult.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <regex>
#include <tuple>


namespace TDEngine2
{
	/*!
		class CTokenizer

		\brief The class is used within CBaseShaderCompiler and its children to split
		a string into a sequence of tokens
	*/

	class CTokenizer
	{
		public:
			TDE2_API CTokenizer(const std::string& str, 
								const std::string& delims = "\n\r\t ", 
								const std::string& specDelims = "{}(),;:");

			TDE2_API CTokenizer(const CTokenizer& tokenizer);
			TDE2_API CTokenizer(CTokenizer&& tokenizer);
			TDE2_API ~CTokenizer();

			TDE2_API const std::string& GetCurrToken() const;

			TDE2_API const std::string& Peek(U32 offset) const;

			TDE2_API bool HasNext() const;

			TDE2_API void Reset();

			/*!
				\brief The method parses a given source and pastes all tokens 
				after a pointer

				\param[in] source A shader's source code

				\param[in] delims A string with delimiters

				\param[in] specDelims A string with delimiters that are considered as tokens
			*/

			TDE2_API void ParseAndPaste(const std::string& source, const std::string& delims, const std::string& specDelims);

			TDE2_API void RemoveCurrentToken();

			TDE2_API E_RESULT_CODE RemoveRange(U32 count);

			TDE2_API const std::string& SeekByOffset(U32 offset);

			TDE2_API const std::string& GetNextToken();

			TDE2_API std::string GetSourceStr() const;

			TDE2_API U32 GetCurrPos() const;
		private:
			TDE2_API CTokenizer() = default;

			TDE2_API std::vector<std::string> _tokenize(const std::string& str, const std::string& delims, const std::string& specDelims);
		private:
			static std::string       mEmptyStr;

			std::vector<std::string> mTokens;
			
			U32                      mCurrPos;
	};

	
	/*!
		class CShaderPreprocessor

		\brief The static class contains a bunch of methods that preprocesses shaders sources
	*/

	class CShaderPreprocessor
	{
		protected:
			static std::regex mIncludePattern;

			static std::regex mDefinePattern;
		public:
			typedef struct TDefineInfoDesc
			{
				std::vector<std::string> mArgs;

				std::string              mValue;
			} TDefineInfoDesc, *TDefineInfoDescPtr;

			typedef std::unordered_map<std::string, TDefineInfoDesc> TDefinesMap;	

			typedef std::tuple<std::string, TDefineInfoDesc>         TMacroDeclaration;

			typedef std::vector<TMacroDeclaration>                   TDefinesOrderedArray;

			typedef struct TPreprocessorResult
			{
				std::string mPreprocessedSource;

				TDefinesMap mDefinesTable;
			}TPreprocessorResult, *TPreprocessorResultPtr;
		public:
			/*!
				\brief The method preprocess a shader source

				\param[in, out] pFileSystem A pointer to IFileSystem implementation

				\param[in] source A string which contains a shader's source

				\return The method returns either a preprocessed shader source or an error code
			*/

			TDE2_API static TResult<TPreprocessorResult> PreprocessSource(IFileSystem* pFileSystem, const std::string& source);
	protected:
			TDE2_API static std::string _removeComments(const std::string& source);

			TDE2_API static std::string _expandInclusions(IFileSystem* pFileSystem, const std::string& source);

			TDE2_API static TPreprocessorResult _expandMacros(const std::string& source);

			TDE2_API static std::string _expandMacro(const std::string& source, const TDefinesOrderedArray& definesTable);

			TDE2_API static TMacroDeclaration _parseMacroDeclaration(const std::string& declarationStr);

			TDE2_API static std::string _evalFuncMacro(const TMacroDeclaration& macro, const std::string& args);

			TDE2_API static TDefinesMap _buildDefinesTable(const TDefinesOrderedArray& definesArray);
	};



	/*!
		class CBaseShaderCompiler

		\brief The class is a basis for all other compilers of shaders
	*/

	class CBaseShaderCompiler : public IShaderCompiler
	{
		protected:
			typedef CShaderPreprocessor::TDefinesMap                     TDefinesMap;

			typedef std::pair<std::string, std::string>                  TShaderDefineDesc;
			
			typedef std::unordered_map<std::string, U32>                 TStructDeclsMap;

			typedef std::unordered_map<std::string, TUniformBufferDesc>  TUniformBuffersMap;

			typedef std::unordered_map<std::string, TShaderResourceDesc> TShaderResourcesMap;
		
			typedef struct TShaderMetadata
			{
				std::string             mVertexShaderEntrypointName;
				std::string             mPixelShaderEntrypointName;
				std::string             mGeometryShaderEntrypointName;

				E_SHADER_FEATURE_LEVEL  mFeatureLevel;

				TDefinesMap             mDefines;

				TStructDeclsMap         mStructDeclsMap;

				TUniformBuffersMap      mUniformBuffers;

				TShaderResourcesMap     mShaderResources;
			} TShaderMetadata;
		public:
			/*!
				\brief The method initializes an initial state of a compiler

				\param[in, out] pFileSystem A pointer to IFileSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IFileSystem* pFileSystem) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseShaderCompiler)

			TDE2_API virtual const C8* _getShaderStageDefineName(E_SHADER_STAGE_TYPE shaderStage) const;

			TDE2_API virtual TShaderMetadata _parseShader(CTokenizer& tokenizer, const TDefinesMap& definesTable) const;
			
			TDE2_API virtual TStructDeclsMap _processStructDecls(CTokenizer& tokenizer) const;

			TDE2_API virtual U32 _getPaddedStructSize(const TStructDeclsMap& structsMap, CTokenizer& tokenizer) const;

			TDE2_API virtual U32 _getBuiltinTypeSize(const std::string& type) const = 0;

			TDE2_API virtual TUniformBuffersMap _processUniformBuffersDecls(const TStructDeclsMap& structsMap, CTokenizer& tokenizer) const;

			TDE2_API virtual E_SHADER_FEATURE_LEVEL _getTargetVersionFromStr(const std::string& ver) const = 0;

			TDE2_API virtual bool _isShaderStageEnabled(E_SHADER_STAGE_TYPE shaderStage, const TShaderMetadata& shaderMeta) const;

			TDE2_API const C8* _getTargetVersionDefineName() const;

			TDE2_API virtual TShaderResourcesMap _processShaderResourcesDecls(CTokenizer& tokenizer) const = 0;
		protected:
			bool             mIsInitialized;

			static U32       mMaxStepsCount; ///< The value is used within _removeComments to bound a maximum number of steps of an automata

			static const C8* mEntryPointsDefineNames[3];

			static const C8* mTargetVersionDefineName;

			IFileSystem*     mpFileSystem;
	};
}