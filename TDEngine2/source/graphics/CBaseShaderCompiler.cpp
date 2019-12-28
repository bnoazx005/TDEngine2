#include "./../../include/graphics/CBaseShaderCompiler.h"
#include "./../../include/graphics/IShader.h"
#include "./../../include/core/IFileSystem.h"
#include "./../../include/graphics/InternalShaderData.h"
#include "./../../include/platform/CTextFileReader.h"
#include <algorithm>
#include <cctype>
#include <vector>
#include <sstream>
#include <iterator>
#include <unordered_set>
#include <stack>
#include <iostream>


namespace TDEngine2
{
	std::string CTokenizer::mEmptyStr = "";

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
			return mEmptyStr;
		}

		return mTokens[mCurrPos + offset];
	}

	const std::string& CTokenizer::GetNextToken()
	{
		if (mCurrPos + 1 >= mTokens.size())
		{
			return mEmptyStr;
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
			return mEmptyStr;
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


	std::regex CShaderPreprocessor::mIncludePattern { "#\\s*include\\s+(<|\")(.*?)(>|\")" };

	std::regex CShaderPreprocessor::mDefinePattern { "#\\s*define\\s+(.*?)\\s+(.*?)\n" };

	TResult<CShaderPreprocessor::TPreprocessorResult> CShaderPreprocessor::PreprocessSource(IFileSystem* pFileSystem, const std::string& source)
	{
		if (!pFileSystem)
		{
			return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		if (source.empty())
		{
			return TOkValue<CShaderPreprocessor::TPreprocessorResult>({});
		}

		TPreprocessorResult processingResult = _expandMacros(_expandInclusions(pFileSystem, _removeComments(source)));

		return TOkValue<TPreprocessorResult>(processingResult);
	}

	std::string CShaderPreprocessor::_removeComments(const std::string& source)
	{
		return CStringUtils::RemoveMultiLineComments(CStringUtils::RemoveSingleLineComments(CStringUtils::RemoveExtraWhitespaces(source)));
	}

	std::string CShaderPreprocessor::_expandInclusions(IFileSystem* pFileSystem, const std::string& source)
	{
		std::string includingFilename;	

		std::smatch matches;

		E_RESULT_CODE result = RC_OK;

		ITextFileReader* pCurrIncludeFile = nullptr;

		std::string processingSource { source };

		std::string::size_type pos = 0;

		while (std::regex_search(processingSource, matches, mIncludePattern))
		{
			if (includingFilename == matches[2]) /// stop search if we get a same substring more that two times
			{
				break;
			}

			includingFilename = matches[2];
			includingFilename = (matches[1] == '<') ? ("vfs://Shaders/" + includingFilename) : includingFilename; // \note for built-in files change the directory to search

			TResult<TFileEntryId> includeFileId = pFileSystem->Open<ITextFileReader>(pFileSystem->ResolveVirtualPath(includingFilename, false));

			if (includeFileId.HasError())
			{
				continue;
			}

			pCurrIncludeFile = pFileSystem->Get<ITextFileReader>(includeFileId.Get());

			if (result != RC_OK)
			{
				/// can't find file just leave this directive without changes

				continue;
			}

			pos = matches.position();

			pCurrIncludeFile->ReadToEnd();

			processingSource = processingSource.substr(0, pos) +
			_removeComments(pCurrIncludeFile->ReadToEnd()) +
			processingSource.substr(pos + matches.length(), processingSource.length() - pos - matches.length());

			pCurrIncludeFile->Close();
		}

		return processingSource;
	}

	CShaderPreprocessor::TPreprocessorResult CShaderPreprocessor::_expandMacros(const std::string& source)
	{
		std::string::size_type pos = 0;
		std::string::size_type pos2 = 0;

		auto getNextChar = [&source, &pos]() -> C8
		{
			return (++pos < source.length()) ? source[pos] : '\0';
		};

		std::string processedSource;
		std::string currFrame;	/// contains either directive name or a current read substring
		std::string macroName;
		std::string macroValue;

		C8 currCh;

		TDefinesOrderedArray defines{};

		auto isDefined = [&defines](const std::string& name) -> bool
		{
			return defines.cend() != std::find_if(defines.cbegin(), defines.cend(), [&name](const TMacroDeclaration& decl)
			{
				return std::get<std::string>(decl) == name;
			});
		};

		std::stack<bool> frameStack; /// contains information about current group's inclusion via result of #if #ifndef and #ifdef

		frameStack.push(true); /// default field of view

		bool wasIfConditionaryBlock = false; /// \note we don't skip #if block so, the flag is used to add #endif if the corresponding #if has appeared

		while (pos < source.length())
		{
			currCh = source[pos];

			if (currCh != '#')
			{
				pos2 = source.find_first_of("\n\r", pos); /// read line

				currFrame = std::move(source.substr(pos, pos2 - pos + 1));

				if (frameStack.top())
				{
					processedSource += _expandMacro(currFrame, defines);
				}

				pos = pos2 + 1;

				currFrame.clear();

				continue;
			}

			currFrame.push_back(currCh); /// push '#'

			while (++pos < source.length() && std::isspace(currCh = source[pos]))
			{
			}

			while (pos < source.length() && !std::isspace(currCh))
			{
				currFrame.push_back(currCh);

				currCh = source[++pos];
			}

			if (currFrame == "#define" && frameStack.top())
			{
				currFrame.clear();

				bool isMultilineMacro = false;
				do
				{
					pos2 = source.find_first_of("\n\r", pos + 1);
					isMultilineMacro = source.find_first_of('\\', pos + 1) < pos2;

					currFrame.append(source.substr(pos + 1, pos2 - pos - 1)); /// contains identifier and replacement-list

					if (isMultilineMacro)
					{
						std::replace_if(currFrame.begin(), currFrame.end(), std::function<bool(C8)>([](C8 ch) { return ch == '\\'; }), '\n');
					}

					pos = pos2;
				}
				while (isMultilineMacro);

				defines.emplace_back(_parseMacroDeclaration(currFrame));
				
				currFrame.clear();

				pos = pos2 + 1;

				continue;
			}

			/// especial directive for OpenGL version of a preprocessor
			if (currFrame == "#version")
			{
				pos2 = source.find_first_of("\n\r", pos + 1);

				currFrame.clear();

				currFrame.append(source.substr(pos + 1, pos2 - pos - 1)); /// contains identifier and replacement-list

				defines.push_back({ "TARGET", { {}, currFrame }});

				processedSource += "#version " + currFrame + "\n";

				currFrame.clear();

				pos = pos2 + 1;

				continue;
			}

			if (currFrame == "#undef" && frameStack.top())
			{
				pos2 = source.find_first_of("\n\r", pos + 1);
				
				currFrame.clear();

				currFrame.append(source.substr(pos + 1, pos2 - pos - 1)); /// contains identifier

				defines.erase(std::find_if(defines.cbegin(), defines.cend(), [&currFrame](const TMacroDeclaration& decl)
				{
					return std::get<std::string>(decl) == currFrame;
				}));

				currFrame.clear();

				pos = pos2;

				continue;
			}

			if (currFrame == "#ifndef" && frameStack.top())
			{
				pos2 = source.find_first_of("\n\r", pos + 1);

				currFrame.clear();

				currFrame.append(source.substr(pos + 1, pos2 - pos - 1)); /// contains identifier

				frameStack.push(!isDefined(currFrame));
				
				currFrame.clear();

				pos = pos2 + 1;

				continue;
			}

			if (currFrame == "#ifdef" && frameStack.top())
			{
				pos2 = source.find_first_of("\n\r", pos + 1);

				currFrame.clear();

				currFrame.append(source.substr(pos + 1, pos2 - pos - 1)); /// contains identifier

				frameStack.push(isDefined(currFrame));
				
				currFrame.clear();

				pos = pos2 + 1;

				continue;
			}

			if (currFrame == "#if")
			{
				pos2 = source.find_first_of("\n\r", pos + 1);

				currFrame.clear();

				currFrame.append(source.substr(pos + 1, pos2 - pos)); /// contains identifier

				processedSource += "#if " + currFrame;

				frameStack.push(true); /// \todo now condition's result of #if is always true

				wasIfConditionaryBlock = true;

				currFrame.clear();

				pos = pos2 + 1;

				continue;
			}

			if (currFrame == "#endif")
			{
				if (wasIfConditionaryBlock)
				{
					processedSource += "#endif\n";

					wasIfConditionaryBlock = false;
				}

				frameStack.pop();

				currFrame.clear();

				++pos;

				continue;
			}
			
			if (!frameStack.top()) /// just skip text which lies within a block with false condition within #if
			{
				++pos;

				continue;
			}
			
			processedSource += currFrame + ' ';

			currFrame.clear();

			++pos;
		}

		return { processedSource, CShaderPreprocessor::_buildDefinesTable(defines) };
	}

	std::string CShaderPreprocessor::_expandMacro(const std::string& source, const TDefinesOrderedArray& definesTable)
	{
		if (definesTable.empty())
		{
			return source;
		}

		std::string::size_type pos          = 0;
		std::string::size_type startArgsPos = 0;
		std::string::size_type endArgsPos   = 0;

		std::string processedSource { source };
		std::string currMacroIdentifier;
		std::string evaluatedStr;

		TDefineInfoDesc currMacroDesc;
		
		for (auto currMacroEntity : definesTable)
		{
			currMacroIdentifier = std::get<std::string>(currMacroEntity);

			currMacroDesc = std::get<TDefineInfoDesc>(currMacroEntity);

			/// simple identifier
			while (currMacroDesc.mArgs.empty() && (pos = processedSource.find(currMacroIdentifier)) != std::string::npos)
			{
				processedSource = processedSource.substr(0, pos) + 
								  currMacroDesc.mValue + 
								  processedSource.substr(pos + currMacroIdentifier.length(), processedSource.length() - pos - currMacroIdentifier.length());
			}

			/// function-like macro
			while (!currMacroDesc.mArgs.empty() && (pos = processedSource.find(currMacroIdentifier)) != std::string::npos)
			{
				startArgsPos = processedSource.find_first_of('(', pos);
				endArgsPos   = processedSource.find_first_of(')', startArgsPos);

				evaluatedStr = _evalFuncMacro(currMacroEntity, processedSource.substr(startArgsPos + 1, endArgsPos - 1 - startArgsPos));

				processedSource = processedSource.substr(0, pos) + evaluatedStr + processedSource.substr(endArgsPos + 1, processedSource.length() - endArgsPos);
			}
		}

		return processedSource;
	}

	CShaderPreprocessor::TMacroDeclaration CShaderPreprocessor::_parseMacroDeclaration(const std::string& declarationStr)
	{
		std::string::size_type pos  = 0;
		std::string::size_type pos2 = 0;

		/// \note try to parse function like macro firstly
		if ((pos = declarationStr.find_first_of('(')) != std::string::npos)
		{
			pos2 = declarationStr.find_first_of(')', pos + 1);

			std::string identifierPart = declarationStr.substr(0, pos);
			std::string argsPart       = CStringUtils::RemoveWhitespaces(declarationStr.substr(pos + 1, pos2 - pos - 1));
			std::string valuePart      = declarationStr.substr(pos2 + 2, declarationStr.length() - pos2 - 2);

			return { identifierPart , { CStringUtils::Split(argsPart, ","), valuePart } };
		}

		/// simple identifier with a replacement value
		if ((pos = declarationStr.find_first_of(" \t")) != std::string::npos)
		{
			return { declarationStr.substr(0, pos), { {}, declarationStr.substr(pos + 1, declarationStr.length() - pos - 1) } };
		}

		/// declaration contains only identifier
		return { declarationStr, { {}, "1" } };
	}

	std::string CShaderPreprocessor::_evalFuncMacro(const TMacroDeclaration& macro, const std::string& args)
	{
		std::vector<std::string> separateArgs = CStringUtils::Split(CStringUtils::RemoveWhitespaces(args), ",");
		std::vector<std::string> argsNames    = std::get<TDefineInfoDesc>(macro).mArgs;

		std::string resultStr = std::get<TDefineInfoDesc>(macro).mValue;
		
		std::string::size_type pos = 0;

		for (U32 i = 0; i < separateArgs.size(); ++i)
		{
			while ((pos = resultStr.find(argsNames[i])) != std::string::npos)
			{
				resultStr = resultStr.substr(0, pos) + separateArgs[i] + resultStr.substr(pos + argsNames[i].length(), resultStr.length() - pos - argsNames[i].length());
			}
		}

		return resultStr;
	}

	CShaderPreprocessor::TDefinesMap CShaderPreprocessor::_buildDefinesTable(const TDefinesOrderedArray& definesArray)
	{
		TDefinesMap definesTable {};

		if (definesArray.empty())
		{
			return definesTable;
		}

		std::string currMacroIdentifier;

		TDefineInfoDesc currMacroDesc;
		
		for (auto currMacroEntity : definesArray)
		{
			currMacroIdentifier = std::get<std::string>(currMacroEntity);

			currMacroDesc = std::get<TDefineInfoDesc>(currMacroEntity);

			definesTable.emplace(currMacroIdentifier, currMacroDesc);
		}

		return definesTable;
	}


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

	CBaseShaderCompiler::TShaderMetadata CBaseShaderCompiler::_parseShader(CTokenizer& tokenizer, const TDefinesMap& definesTable) const
	{
		TShaderMetadata extractedMetadata {};
		
		extractedMetadata.mDefines = definesTable;

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
		TDefinesMap::const_iterator defineIter = shaderMeta.mDefines.find(mEntryPointsDefineNames[shaderStage]);

		if (defineIter == shaderMeta.mDefines.cend() ||
			defineIter->second.mValue.empty())
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

	U32 CBaseShaderCompiler::_getPaddedStructSize(const TStructDeclsMap& structsMap, CTokenizer& tokenizer,
												  const TUniformVariableFunctor& uniformProcessor) const
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
			U32 currMemberSize = _getBuiltinTypeSize(currToken);
			totalStructSize += currMemberSize;

			do
			{
				currToken = tokenizer.GetNextToken();

				if (currToken != ";")
				{
					uniformProcessor({ currToken, currMemberSize });
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
}