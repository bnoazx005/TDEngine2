#include "../../include/graphics/CBaseShaderCompiler.h"
#include "../../include/graphics/IShader.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/graphics/InternalShaderData.h"
#include "../../include/platform/CTextFileReader.h"
#include "../../include/utils/CFileLogger.h"
#define TCPP_IMPLEMENTATION
#include "../../deps/tcpp/source/tcppLibrary.hpp"
#include "stringUtils.hpp"
#include <algorithm>
#include <cctype>
#include <vector>
#include <sstream>
#include <iterator>
#include <unordered_set>
#include <stack>
#include <iostream>
#include <cassert>
#include <memory>


namespace TDEngine2
{
	CTokenizer::CTokenizer(const std::string& str, const std::string& delims, const std::string& specDelims) :
		mCurrPos(0)
	{
		mTokens = _tokenize(str, delims, specDelims);
	}

	CTokenizer::CTokenizer(const CTokenizer& tokenizer) :
		mTokens(tokenizer.mTokens), mCurrPos(tokenizer.mCurrPos)
	{
	}

	CTokenizer::CTokenizer(CTokenizer&& tokenizer) :
		mTokens(std::move(tokenizer.mTokens)), mCurrPos(tokenizer.mCurrPos)
	{
	}

	CTokenizer::~CTokenizer()
	{
		mTokens.clear();
	}

	const std::string& CTokenizer::GetCurrToken() const
	{
		return mTokens[mCurrPos];
	}

	const std::string& CTokenizer::Peek(U32 offset) const
	{
		U32 pos = mCurrPos + offset;

		if (pos >= mTokens.size())
		{
			return Wrench::StringUtils::GetEmptyStr();
		}

		return mTokens[mCurrPos + offset];
	}

	const std::string& CTokenizer::GetNextToken()
	{
		if (mCurrPos + 1 >= mTokens.size())
		{
			return Wrench::StringUtils::GetEmptyStr();
		}

		return mTokens[++mCurrPos];
	}

	bool CTokenizer::HasNext() const
	{
		return mCurrPos + 1 < mTokens.size();
	}

	void CTokenizer::RemoveCurrentToken()
	{
		if (mTokens.empty())
		{
			return;
		}

		mTokens.erase(mTokens.begin() + mCurrPos);
	}

	E_RESULT_CODE CTokenizer::RemoveRange(U32 count)
	{
		if (mCurrPos + count >= mTokens.size())
		{
			return RC_INVALID_ARGS;
		}

		mTokens.erase(mTokens.begin() + mCurrPos, mTokens.begin() + mCurrPos + count);

		return RC_OK;
	}

	void CTokenizer::ParseAndPaste(const std::string& source, const std::string& delims, const std::string& specDelims)
	{
		if (source.empty())
		{
			return;
		}

		std::vector<std::string> tokens = _tokenize(source, delims, specDelims);

		mTokens.insert(mTokens.begin() + mCurrPos, tokens.begin(), tokens.end());
	}

	void CTokenizer::Reset()
	{
		mCurrPos = 0;
	}

	const std::string& CTokenizer::SeekByOffset(U32 offset)
	{
		U32 pos = mCurrPos + offset;

		if (pos >= mTokens.size())
		{
			return Wrench::StringUtils::GetEmptyStr();
		}

		mCurrPos = pos;

		return mTokens[mCurrPos];
	}

	std::string CTokenizer::GetSourceStr() const
	{
		std::string source;

		for (auto iter = mTokens.cbegin(); iter != mTokens.cend(); ++iter)
		{
			source += (*iter);
		}

		return source;
	}

	U32 CTokenizer::GetCurrPos() const
	{
		return mCurrPos;
	}

	std::vector<std::string> CTokenizer::_tokenize(const std::string& str, const std::string& delims, const std::string& specDelims)
	{
		const std::string& allDelims = delims + specDelims;

		std::string::size_type pos     = 0;
		std::string::size_type currPos = 0;

		std::string currToken;

		std::vector<std::string> tokens;

		while ((currPos < str.length()) && ((pos = str.find_first_of(allDelims, currPos)) != std::string::npos))
		{
			currToken = std::move(str.substr(currPos, pos - currPos));

			if (!currToken.empty())
			{
				tokens.emplace_back(std::move(currToken));
			}

			if (specDelims.find_first_of(str[pos]) != std::string::npos)
			{
				tokens.emplace_back(std::move(str.substr(pos, 1)));
			}

			currPos = pos + 1;
		}

		/// insert last token if it wasn't pushed back before
		if (currPos != str.length())
		{
			tokens.emplace_back(std::move(str.substr(currPos, str.length() - currPos)));
		}

		return std::move(tokens);
	}


	TResult<CShaderPreprocessor::TPreprocessorResult> CShaderPreprocessor::PreprocessSource(IFileSystem* pFileSystem, const std::string& source)
	{
		if (!pFileSystem)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		std::vector<std::unique_ptr<tcpp::IInputStream>> inputStreams;
		inputStreams.emplace_back(new tcpp::StringInputStream{ source });
		tcpp::Lexer lexer { *inputStreams.front() };
		
		tcpp::Preprocessor preprocessor { lexer, 
		[](const tcpp::TErrorInfo& errorInfo)
		{
			LOG_ERROR(tcpp::ErrorTypeToString(errorInfo.mType));
			assert(false);
		}, 
		[&pFileSystem, &inputStreams](const std::string& path, bool isSystemPath) -> tcpp::IInputStream*
		{
			std::string filename = isSystemPath ? ("Shaders/" + path) : path; // \note for built-in files change the directory to search

			TResult<TFileEntryId> includeFileId = pFileSystem->Open<ITextFileReader>(filename);
			if (includeFileId.HasError())
			{
				return nullptr;
			}

			auto pCurrIncludeFile = pFileSystem->Get<ITextFileReader>(includeFileId.Get());

			inputStreams.emplace_back(new tcpp::StringInputStream{ pCurrIncludeFile->ReadToEnd() });
			pCurrIncludeFile->Close();

			return inputStreams.back().get();
		} };
		
		// \note below we define our custom preprocessor's commands like #version, #program/#endprogram, etc
		// \todo Reimplement error handling for custom directives
		preprocessor.AddCustomDirectiveHandler("version", [](auto&&, tcpp::Lexer& lexer, auto&&)
		{
			tcpp::TMacroDesc targetMacroDesc;
			targetMacroDesc.mName = "TARGET";

			std::string versionStr = "#version ";

			auto currToken = lexer.GetNextToken();

			currToken = lexer.GetNextToken();
			versionStr.append(currToken.mRawView);
			targetMacroDesc.mValue.push_back(currToken);

			currToken = lexer.GetNextToken();

			if ((currToken = lexer.GetNextToken()).mType != tcpp::E_TOKEN_TYPE::NEWLINE)
			{
				versionStr.append(" ").append(currToken.mRawView);
				targetMacroDesc.mValue.push_back(currToken);
			}

			return versionStr.append("\n");
		});

		E_SHADER_STAGE_TYPE lastVisitedRegion = E_SHADER_STAGE_TYPE::SST_NONE;
		TShaderStagesRegionsMap stagesRegionsInfo;

		preprocessor.AddCustomDirectiveHandler("program", [&stagesRegionsInfo, &lastVisitedRegion](auto&&, tcpp::Lexer& lexer, const std::string& str)
		{
			auto currToken = lexer.GetNextToken();
			currToken = lexer.GetNextToken();

			if (lastVisitedRegion != E_SHADER_STAGE_TYPE::SST_NONE)
			{
				LOG_ERROR("[Shader Preprocessor] A new region start is found before the previous one has finished");
				return "";
			}

			stagesRegionsInfo[lastVisitedRegion = ShaderStageStringToEnum(currToken.mRawView)] = { static_cast<U32>(str.length()), 0 };
			return "";
		});

		preprocessor.AddCustomDirectiveHandler("endprogram", [&stagesRegionsInfo, &lastVisitedRegion](auto&&, tcpp::Lexer& lexer, const std::string& str)
		{
			auto currToken = lexer.GetNextToken();
			auto currInfo = stagesRegionsInfo[lastVisitedRegion];
			
			stagesRegionsInfo[lastVisitedRegion] = { std::get<0>(currInfo), static_cast<U32>(str.length()) };
			lastVisitedRegion = E_SHADER_STAGE_TYPE::SST_NONE;

			return "";
		});

		std::string processedSource = preprocessor.Process();
		
		TDefinesMap definesTable;
		for (const auto& currMacroDef : preprocessor.GetSymbolsTable())
		{
			std::string valueStr = "";

			for (const auto& currToken : currMacroDef.mValue)
			{
				valueStr.append(currToken.mRawView);
			}

			definesTable[currMacroDef.mName] = { currMacroDef.mArgsNames, valueStr };
		}

		return Wrench::TOkValue<TPreprocessorResult>({ processedSource, definesTable, stagesRegionsInfo });
	}

	std::string CShaderPreprocessor::ShaderStageToString(const E_SHADER_STAGE_TYPE& stageType)
	{
		switch (stageType)
		{
			case E_SHADER_STAGE_TYPE::SST_VERTEX:
				return "vertex";
			case E_SHADER_STAGE_TYPE::SST_PIXEL:
				return "pixel";
			case E_SHADER_STAGE_TYPE::SST_GEOMETRY:
				return "geometry";
		}

		return "unknown";
	}

	E_SHADER_STAGE_TYPE CShaderPreprocessor::ShaderStageStringToEnum(const std::string& stageType)
	{
		static const std::unordered_map<std::string, E_SHADER_STAGE_TYPE> shadersStages
		{
			{ "vertex", E_SHADER_STAGE_TYPE::SST_VERTEX },
			{ "pixel", E_SHADER_STAGE_TYPE::SST_PIXEL },
			{ "geometry", E_SHADER_STAGE_TYPE::SST_GEOMETRY },
		};

		auto iter = shadersStages.find(stageType);
		return iter != shadersStages.cend() ? iter->second : E_SHADER_STAGE_TYPE::SST_NONE;
	}


	U32 CBaseShaderCompiler::mMaxStepsCount = 1000;

	const C8* CBaseShaderCompiler::mEntryPointsDefineNames[3] = { "VERTEX_ENTRY", "PIXEL_ENTRY", "GEOMETRY_ENTRY" };

	const C8* CBaseShaderCompiler::mTargetVersionDefineName = "TARGET";		

	CBaseShaderCompiler::CBaseShaderCompiler() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseShaderCompiler::Init(IFileSystem* pFileSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpFileSystem = pFileSystem;

		mIsInitialized = true;

		return RC_OK;
	}

	const C8* CBaseShaderCompiler::_getShaderStageDefineName(E_SHADER_STAGE_TYPE shaderStage) const
	{
		switch (shaderStage)
		{
			case SST_VERTEX:
				return "VERTEX";
			case SST_PIXEL:
				return "PIXEL";
			case SST_GEOMETRY:
				return "GEOMETRY";
		}

		return nullptr;
	}

	CBaseShaderCompiler::TShaderMetadata CBaseShaderCompiler::_parseShader(CTokenizer& tokenizer, const TDefinesMap& definesTable, const TStagesRegionsMap& stagesRegionsInfo) const
	{
		TShaderMetadata extractedMetadata {};
		
		extractedMetadata.mDefines = definesTable;

		extractedMetadata.mShaderStagesRegionsInfo = stagesRegionsInfo;

		extractedMetadata.mStructDeclsMap = _processStructDecls(tokenizer);
		
		extractedMetadata.mUniformBuffers = _processUniformBuffersDecls(extractedMetadata.mStructDeclsMap, tokenizer);

		extractedMetadata.mShaderResources = _processShaderResourcesDecls(tokenizer);

		extractedMetadata.mVertexShaderEntrypointName   = extractedMetadata.mDefines[mEntryPointsDefineNames[SST_VERTEX]].mValue;
		extractedMetadata.mPixelShaderEntrypointName    = extractedMetadata.mDefines[mEntryPointsDefineNames[SST_PIXEL]].mValue;
		extractedMetadata.mGeometryShaderEntrypointName = extractedMetadata.mDefines[mEntryPointsDefineNames[SST_GEOMETRY]].mValue;

		///\todo implement convertation of a version string into E_SHADER_TARGET_VERSION enum's value
		extractedMetadata.mFeatureLevel = _getTargetVersionFromStr(extractedMetadata.mDefines[mTargetVersionDefineName].mValue);

		return extractedMetadata;
	}

	bool CBaseShaderCompiler::_isShaderStageEnabled(E_SHADER_STAGE_TYPE shaderStage, const CBaseShaderCompiler::TShaderMetadata& shaderMeta) const
	{
		return shaderMeta.mShaderStagesRegionsInfo.find(shaderStage) != shaderMeta.mShaderStagesRegionsInfo.cend();
	}

	const C8* CBaseShaderCompiler::_getTargetVersionDefineName() const
	{
		return mTargetVersionDefineName;
	}

	CBaseShaderCompiler::TStructDeclsMap CBaseShaderCompiler::_processStructDecls(CTokenizer& tokenizer) const
	{
		TStructDeclsMap structDeclsMap;

		std::string currToken;
		std::string nextToken;

		std::string structName;
		std::string structBody;

		U8 nestedBlocks = 0;

		while (tokenizer.HasNext())
		{
			currToken = tokenizer.GetCurrToken();

			if (currToken != "struct")
			{
				currToken = tokenizer.GetNextToken();

				continue;
			}

			structName = tokenizer.GetNextToken();

			if ((nextToken = tokenizer.GetNextToken()) == ";") /// forward declaration
			{
				structDeclsMap[currToken] = 0;

				continue;
			}

			if (nextToken == "{")
			{
				structDeclsMap[structName] = _getPaddedStructSize(structDeclsMap, tokenizer);
			}
		}
		
		tokenizer.Reset();

		return structDeclsMap;
	}


	static std::string ExtractIdentifier(const std::string& idStr)
	{
		std::string outputIdentifier;
		
		for (auto&& ch : idStr)
		{
			if (!std::isalnum(ch) && ch != '_')
			{
				return outputIdentifier;
			}

			outputIdentifier.push_back(ch);
		}

		return outputIdentifier;
	}


	USIZE CBaseShaderCompiler::_getPaddedStructSize(const TStructDeclsMap& structsMap, CTokenizer& tokenizer,
													const TUniformVariableFunctor& uniformProcessor) const
	{
		USIZE totalStructSize = 0;

		std::string currToken;

		TStructDeclsMap::const_iterator existingTypeIter;
		
		while (tokenizer.HasNext())
		{
			currToken = tokenizer.GetNextToken();

			if (currToken == "}")
			{
				while (currToken != ";" && tokenizer.HasNext())
				{
					currToken = tokenizer.GetNextToken(); 
				}
				
				break;
			}

			/// if the custom type already exists just add its size
			if ((existingTypeIter = structsMap.find(currToken)) != structsMap.cend())
			{
				totalStructSize += (*existingTypeIter).second;

				continue;
			}

			if (currToken == "struct")
			{
				do
				{
					currToken = tokenizer.GetNextToken();
				} 
				while (currToken != "{" && tokenizer.HasNext());

				totalStructSize += _getPaddedStructSize(structsMap, tokenizer);

				continue;
			}

			/// extract type of a member and compute its padded size
			USIZE currMemberSize = _getBuiltinTypeSize(currToken);
			totalStructSize += currMemberSize;

			do
			{
				currToken = tokenizer.GetNextToken();

				if (currToken != ";")
				{
					uniformProcessor({ std::move(ExtractIdentifier(currToken)), currMemberSize });
				}
			} 
			while (currToken != ";" && tokenizer.HasNext());
		}

		return (totalStructSize + 15) & -16; /// compute padding
	}

	CBaseShaderCompiler::TUniformBuffersMap CBaseShaderCompiler::_processUniformBuffersDecls(const TStructDeclsMap& structsMap, CTokenizer& tokenizer) const
	{
		/// declare internal engine's buffers;

		return
			{ 
				{ "TDEngine2PerFrame", { IUBR_PER_FRAME, sizeof(TPerFrameShaderData), E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL } },
				{ "TDEngine2PerObject", { IUBR_PER_OBJECT, sizeof(TPerObjectShaderData), E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL} },
				{ "TDEngine2RareUpdate",{ IUBR_RARE_UDATED, sizeof(TRareUpdateShaderData), E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL} },
				{ "TDEngine2Constants", { IUBR_CONSTANTS, sizeof(TConstantShaderData), E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL} }
			};
	}

	std::string CBaseShaderCompiler::_enableShaderStage(E_SHADER_STAGE_TYPE shaderStage, const TStagesRegionsMap& stagesRegionsInfo, const std::string& source) const
	{
		std::string processedSource{ source };

		auto vertexRegions   = (stagesRegionsInfo.find(SST_VERTEX) != stagesRegionsInfo.cend()) ? stagesRegionsInfo.at(SST_VERTEX) : std::tuple<U32, U32>{ 0, 0 };
		auto pixelRegions    = (stagesRegionsInfo.find(SST_PIXEL) != stagesRegionsInfo.cend()) ? stagesRegionsInfo.at(SST_PIXEL) : std::tuple<U32, U32>{ 0, 0 };
		auto geometryRegions = (stagesRegionsInfo.find(SST_GEOMETRY) != stagesRegionsInfo.cend()) ? stagesRegionsInfo.at(SST_GEOMETRY) : std::tuple<U32, U32>{ 0, 0 };

		switch (shaderStage)
		{
			case E_SHADER_STAGE_TYPE::SST_VERTEX:
				processedSource.erase(std::get<0>(pixelRegions), std::get<1>(pixelRegions) - std::get<0>(pixelRegions));
				processedSource.erase(std::get<0>(geometryRegions), std::get<1>(geometryRegions) - std::get<0>(geometryRegions));
				break;
			case E_SHADER_STAGE_TYPE::SST_PIXEL:
				processedSource.erase(std::get<0>(vertexRegions), std::get<1>(vertexRegions) - std::get<0>(vertexRegions));
				processedSource.erase(std::get<0>(geometryRegions), std::get<1>(geometryRegions) - std::get<0>(geometryRegions));
				break;
			case E_SHADER_STAGE_TYPE::SST_GEOMETRY:
				processedSource.erase(std::get<0>(vertexRegions), std::get<1>(vertexRegions) - std::get<0>(vertexRegions));
				processedSource.erase(std::get<0>(pixelRegions), std::get<1>(pixelRegions) - std::get<0>(pixelRegions));
				break;
		}

		return processedSource;
	}
}