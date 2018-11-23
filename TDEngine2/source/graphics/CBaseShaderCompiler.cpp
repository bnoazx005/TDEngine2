#include "./../../include/graphics/CBaseShaderCompiler.h"
#include "./../../include/graphics/IShader.h"
#include "./../../include/core/IFileSystem.h"
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

	CBaseShaderCompiler::TShaderMetadata CBaseShaderCompiler::_parseShader(const std::string& sourceCode) const
	{
		TShaderMetadata extractedMetadata {};
		
		extractedMetadata.mDefines = _processDefines(sourceCode);

		extractedMetadata.mUniformBuffers = _processUniformBuffersDecls(sourceCode);

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
}