/*!
	\file CBaseShaderCompiler.h
	\date 23.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IShaderCompiler.h"
#include "./../core/CBaseObject.h"
#include "./../utils/Utils.h"
#include "./../utils/Types.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <regex>
#include <tuple>
#include <functional>


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
			std::vector<std::string> mTokens;
			
			U32                      mCurrPos;
	};

	
	/*!
		class CShaderPreprocessor

		\brief The static class contains a bunch of methods that preprocesses shaders sources
	*/

	class CShaderPreprocessor
	{
		public:
			typedef struct TDefineInfoDesc
			{
				std::vector<std::string> mArgs;

				std::string              mValue;
			} TDefineInfoDesc, *TDefineInfoDescPtr;

			typedef std::unordered_map<std::string, TDefineInfoDesc>              TDefinesMap;
			typedef std::unordered_map<E_SHADER_STAGE_TYPE, std::tuple<U32, U32>> TShaderStagesRegionsMap;

			typedef struct TPreprocessorResult
			{
				std::string mPreprocessedSource;

				TDefinesMap mDefinesTable;

				TShaderStagesRegionsMap mStagesRegions;
			}TPreprocessorResult, *TPreprocessorResultPtr;
		public:
			/*!
				\brief The method preprocess a shader source

				\param[in, out] pFileSystem A pointer to IFileSystem implementation

				\param[in] source A string which contains a shader's source

				\return The method returns either a preprocessed shader source or an error code
			*/

			TDE2_API static TResult<TPreprocessorResult> PreprocessSource(IFileSystem* pFileSystem, const std::string& source);
			
			/*!
				\brief The method converts given enumeration's value into its string representation which is used
				within the preprocessor

				\param[in] stageType A value of E_SHADER_STAGE_TYPE

				\return A string which equals to some of these 'vertex', 'pixel' and 'geometry'
			*/

			TDE2_API static std::string ShaderStageToString(const E_SHADER_STAGE_TYPE& stageType);

			/*!
				\brief The method converts given string which equals to some of these 'vertex', 'pixel' and 'geometry'
				into enumeration's value

				\param[in] stageType A string which equals to some of these 'vertex', 'pixel' and 'geometry'

				\return A value of E_SHADER_STAGE_TYPE enumeration's type
			*/

			TDE2_API static E_SHADER_STAGE_TYPE ShaderStageStringToEnum(const std::string& stageType);
	};



	/*!
		class CBaseShaderCompiler

		\brief The class is a basis for all other compilers of shaders
	*/

	class CBaseShaderCompiler : public CBaseObject, public IShaderCompiler
	{
		protected:
			typedef CShaderPreprocessor::TDefinesMap                           TDefinesMap;

			typedef CShaderPreprocessor::TShaderStagesRegionsMap               TStagesRegionsMap;

			typedef std::pair<std::string, std::string>                        TShaderDefineDesc;
			
			typedef std::unordered_map<std::string, USIZE>                     TStructDeclsMap;

			typedef std::unordered_map<std::string, TUniformBufferDesc>        TUniformBuffersMap;

			typedef std::unordered_map<std::string, TShaderResourceDesc>       TShaderResourcesMap;

			typedef std::function<void(const TShaderUniformDesc& uniformInfo)> TUniformVariableFunctor;
		
			typedef struct TShaderMetadata
			{
				std::string             mVertexShaderEntrypointName;
				std::string             mPixelShaderEntrypointName;
				std::string             mGeometryShaderEntrypointName;
				std::string             mComputeShaderEntrypointName;

				E_SHADER_FEATURE_LEVEL  mFeatureLevel;

				TDefinesMap             mDefines;

				TStructDeclsMap         mStructDeclsMap;

				TUniformBuffersMap      mUniformBuffers;

				TShaderResourcesMap     mShaderResources;

				TStagesRegionsMap       mShaderStagesRegionsInfo;
			} TShaderMetadata;
		public:
			/*!
				\brief The method initializes an initial state of a compiler

				\param[in, out] pFileSystem A pointer to IFileSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IFileSystem* pFileSystem) override;

			TDE2_API static TypeId GetShaderBuiltInTypeId(const std::string& typeName);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseShaderCompiler)

			TDE2_API virtual const C8* _getShaderStageDefineName(E_SHADER_STAGE_TYPE shaderStage) const;

			TDE2_API virtual TShaderMetadata _parseShader(CTokenizer& tokenizer, const TDefinesMap& definesTable, const TStagesRegionsMap& stagesRegionsInfo) const;
			
			TDE2_API virtual TStructDeclsMap _processStructDecls(CTokenizer& tokenizer) const;

			TDE2_API virtual USIZE _getPaddedStructSize(const TStructDeclsMap& structsMap, CTokenizer& tokenizer,
														const TUniformVariableFunctor& uniformProcessor = [](auto){}) const;

			TDE2_API virtual USIZE _getBuiltinTypeSize(const std::string& type, const std::function<void(const std::string&)> typeProcessor = nullptr) const = 0;

			TDE2_API virtual TUniformBuffersMap _processUniformBuffersDecls(const TStructDeclsMap& structsMap, CTokenizer& tokenizer) const;

			TDE2_API virtual E_SHADER_FEATURE_LEVEL _getTargetVersionFromStr(const std::string& ver) const = 0;

			TDE2_API virtual bool _isShaderStageEnabled(E_SHADER_STAGE_TYPE shaderStage, const TShaderMetadata& shaderMeta) const;

			TDE2_API const C8* _getTargetVersionDefineName() const;

			TDE2_API virtual TShaderResourcesMap _processShaderResourcesDecls(CTokenizer& tokenizer) const = 0;

			TDE2_API std::string _enableShaderStage(E_SHADER_STAGE_TYPE shaderStage, const TStagesRegionsMap& stagesRegionsInfo, const std::string& source) const;
		protected:
			static U32       mMaxStepsCount; ///< The value is used within _removeComments to bound a maximum number of steps of an automata

			static const C8* mEntryPointsDefineNames[MaxNumOfShaderStages];

			static const C8* mTargetVersionDefineName;

			IFileSystem*     mpFileSystem;
	};
}