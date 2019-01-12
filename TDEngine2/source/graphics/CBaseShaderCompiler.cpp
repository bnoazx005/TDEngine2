#include "./../../include/graphics/CBaseShaderCompiler.h"
#include "./../../include/graphics/IShader.h"
#include "./../../include/core/IFileSystem.h"
#include "./../../include/graphics/InternalShaderData.h"
#include <algorithm>
#include <cctype>
#include <vector>
#include <sstream>
#include <iterator>
#include <unordered_set>


namespace TDEngine2
{
	U32 CBaseShaderCompiler::mMaxStepsCount = 1000;

	const C8* CBaseShaderCompiler::mEntryPointsDefineNames[3] = { "VERTEX_ENTRY", "PIXEL_ENTRY", "GEOMETRY_ENTRY" };

	const C8* CBaseShaderCompiler::mTargetVersionDefineName = "TARGET";


	std::string CBaseShaderCompiler::CTokenizer::mEmptyStr = "";

	CBaseShaderCompiler::CTokenizer::CTokenizer(const std::string& str):
		mCurrPos(0), mSourceStr(str)
	{
		const std::string& delims     = "\n\t ";
		const std::string& specDelims = "{}(),;:";
		const std::string& allDelims  = delims + specDelims;

		U32 pos     = 0;
		U32 currPos = 0;

		std::string currToken;

		while ((currPos < str.length()) && ((pos = str.find_first_of(allDelims, currPos)) != std::string::npos))
		{
			currToken = std::move(str.substr(currPos, pos - currPos));

			if (!currToken.empty())
			{
				mTokens.emplace_back(std::move(currToken));
			}

			if (specDelims.find_first_of(str[pos]) != std::string::npos)
			{
				mTokens.emplace_back(std::move(str.substr(pos, 1)));
			}

			currPos = pos + 1;
		}
	}
	
	CBaseShaderCompiler::CTokenizer::CTokenizer(const CBaseShaderCompiler::CTokenizer& tokenizer):
		mTokens(tokenizer.mTokens), mCurrPos(tokenizer.mCurrPos), mSourceStr(tokenizer.mSourceStr)
	{
	}

	CBaseShaderCompiler::CTokenizer::CTokenizer(CBaseShaderCompiler::CTokenizer&& tokenizer):
		mTokens(std::move(tokenizer.mTokens)), mCurrPos(tokenizer.mCurrPos), mSourceStr(std::move(tokenizer.mSourceStr))
	{
	}

	CBaseShaderCompiler::CTokenizer::~CTokenizer()
	{
		mTokens.clear();
	}

	const std::string& CBaseShaderCompiler::CTokenizer::GetCurrToken() const
	{
		return mTokens[mCurrPos];
	}

	const std::string& CBaseShaderCompiler::CTokenizer::Peek(U32 offset) const
	{
		U32 pos = mCurrPos + offset;

		if (pos >= mTokens.size())
		{
			return mEmptyStr;
		}

		return mTokens[mCurrPos + offset];
	}

	const std::string& CBaseShaderCompiler::CTokenizer::GetNextToken()
	{
		if (mCurrPos + 1 >= mTokens.size())
		{
			return mEmptyStr;
		}

		return mTokens[++mCurrPos];
	}

	bool CBaseShaderCompiler::CTokenizer::HasNext() const
	{
		return mCurrPos + 1 < mTokens.size();
	}

	void CBaseShaderCompiler::CTokenizer::Reset()
	{
		mCurrPos = 0;
	}

	const std::string& CBaseShaderCompiler::CTokenizer::SeekByOffset(U32 offset)
	{
		U32 pos = mCurrPos + offset;

		if (pos >= mTokens.size())
		{
			return mEmptyStr;
		}

		mCurrPos = pos;

		return mTokens[mCurrPos];
	}

	const std::string& CBaseShaderCompiler::CTokenizer::GetSourceStr() const
	{
		return mSourceStr;
	}
		

	CBaseShaderCompiler::CBaseShaderCompiler() :
		mIsInitialized(false)
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

	E_RESULT_CODE CBaseShaderCompiler::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

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

	CBaseShaderCompiler::TShaderMetadata CBaseShaderCompiler::_parseShader(CTokenizer& tokenizer) const
	{
		TShaderMetadata extractedMetadata {};
		
		extractedMetadata.mDefines = _processDefines(tokenizer.GetSourceStr());

		extractedMetadata.mStructDeclsMap = _processStructDecls(tokenizer);
		
		extractedMetadata.mUniformBuffers = _processUniformBuffersDecls(extractedMetadata.mStructDeclsMap, tokenizer);

		extractedMetadata.mShaderResources = _processShaderResourcesDecls(tokenizer);

		extractedMetadata.mVertexShaderEntrypointName   = extractedMetadata.mDefines[mEntryPointsDefineNames[SST_VERTEX]];
		extractedMetadata.mPixelShaderEntrypointName    = extractedMetadata.mDefines[mEntryPointsDefineNames[SST_PIXEL]];
		extractedMetadata.mGeometryShaderEntrypointName = extractedMetadata.mDefines[mEntryPointsDefineNames[SST_GEOMETRY]];

		///\todo implement convertation of a version string into E_SHADER_TARGET_VERSION enum's value
		extractedMetadata.mFeatureLevel = _getTargetVersionFromStr(extractedMetadata.mDefines[mTargetVersionDefineName]);

		return extractedMetadata;
	}

	std::string CBaseShaderCompiler::_removeComments(const std::string& sourceCode) const
	{
		std::string processedSourceCode = sourceCode;

		/// replace \t with ' '
		std::replace_if(processedSourceCode.begin(), processedSourceCode.end(), [](C8 ch) { return ch == '\t'; }, ' ');

		/// remove extra whitespaces
		bool isPrevChSpace = false;

		processedSourceCode.erase(std::remove_if(processedSourceCode.begin(), processedSourceCode.end(), [&isPrevChSpace](C8 ch)
		{
			bool isCurrChSpace = std::isspace(ch);

			bool result = isCurrChSpace && isPrevChSpace;

			isPrevChSpace = isCurrChSpace;

			return result;
		}), processedSourceCode.end());

		U32 firstPos = 0;
		U32 secondPos = 0;

		/// remove single-line comments
		while ((firstPos = processedSourceCode.find("//")) != std::string::npos)
		{
			secondPos = processedSourceCode.find_first_of("\n\r", firstPos);

			if (secondPos == std::string::npos)
			{
				processedSourceCode = processedSourceCode.substr(0, firstPos);

				break;
			}

			processedSourceCode = processedSourceCode.substr(0, firstPos) + processedSourceCode.substr(secondPos + 1, processedSourceCode.length() - secondPos);
		}

		/// remove multi-line C style comments

		U16 stepsCounter = 0;

		U8 numOfNestedCommentsBlocks = 0;

		U32 thirdPos;
		U32 seekPos;

		std::string::const_iterator iter = processedSourceCode.cbegin();

		while ((stepsCounter++ <= mMaxStepsCount) &&
			((firstPos = processedSourceCode.find("/*")) != std::string::npos))
		{
			++numOfNestedCommentsBlocks;

			seekPos = firstPos + 2;

			do
			{
				secondPos = processedSourceCode.find("*/", seekPos);
				thirdPos = processedSourceCode.find("/*", seekPos);

				if (secondPos < thirdPos)
				{
					--numOfNestedCommentsBlocks;

					seekPos = secondPos + 2;

					continue;
				}

				if (thirdPos < secondPos)
				{
					++numOfNestedCommentsBlocks;

					seekPos = thirdPos + 2;

					continue;
				}
			} while (seekPos < processedSourceCode.length() && numOfNestedCommentsBlocks > 0);

			if (secondPos != std::string::npos)
			{
				processedSourceCode = processedSourceCode.substr(0, firstPos) + processedSourceCode.substr(secondPos + 2, processedSourceCode.length() - secondPos - 2);
			}
			else
			{
				processedSourceCode = processedSourceCode.substr(0, firstPos);
			}
		}

		return processedSourceCode;
	}

	CBaseShaderCompiler::TDefinesMap CBaseShaderCompiler::_processDefines(const std::string& sourceCode) const
	{
		TDefinesMap defines;

		U32 firstPos = 0;
		U32 secondPos = 0;
		U32 nextPos = 0;

		std::string currLine;
		std::string currDefineName;
		std::string currDefineValue;

		/// By now it supports single line only macro
		/// \todo Multi-line macro support should be implemented

		while ((firstPos = sourceCode.find_first_of("#", nextPos)) != std::string::npos)
		{
			nextPos = firstPos + 1;

			/// if it's a define directive try to parse
			firstPos = sourceCode.find("define", nextPos);

			if (firstPos == std::string::npos)
			{
				continue;
			}

			/// extract the line with the definition
			secondPos = sourceCode.find_first_of('\n', nextPos);

			nextPos = secondPos + 1;

			currLine = sourceCode.substr(firstPos + 6, secondPos - firstPos - 6); /// 6 is a length of "define" keyword

																				  /// extract name 
			firstPos = currLine.find_first_not_of(' ');

			if (firstPos == std::string::npos)
			{
				continue;
			}

			secondPos = currLine.find_first_of(' ', firstPos);

			if (secondPos == std::string::npos)
			{
				secondPos = currLine.find_first_of(')', firstPos);

				if (secondPos == std::string::npos)
				{
					currDefineName = currLine.substr(firstPos, currLine.length() - firstPos);

					defines.emplace(currDefineName, "1"); // assign a default value

					continue;
				}
			}

			++secondPos;

			currDefineName = currLine.substr(firstPos, secondPos - firstPos - 1);

			/// extract value, if it doesn't exists the value is set to 1 by default
			currDefineValue = currLine.substr(secondPos, currLine.length() - secondPos);

			defines.emplace(currDefineName, currDefineValue);
		}

		return defines;
	}

	bool CBaseShaderCompiler::_isShaderStageEnabled(E_SHADER_STAGE_TYPE shaderStage, const CBaseShaderCompiler::TShaderMetadata& shaderMeta) const
	{
		TDefinesMap::const_iterator defineIter = shaderMeta.mDefines.find(mEntryPointsDefineNames[shaderStage]);

		if (defineIter == shaderMeta.mDefines.cend() ||
			defineIter->second.empty())
		{
			return false;
		}

		return true;
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

	U32 CBaseShaderCompiler::_getPaddedStructSize(const TStructDeclsMap& structsMap, CTokenizer& tokenizer) const
	{
		U32 totalStructSize = 0;

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
			totalStructSize += _getBuiltinTypeSize(currToken);

			do
			{
				currToken = tokenizer.GetNextToken();
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
}