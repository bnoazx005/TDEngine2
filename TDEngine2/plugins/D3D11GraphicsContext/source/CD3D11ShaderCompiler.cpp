#include "./../include/CD3D11ShaderCompiler.h"
#include "./../include/CD3D11Mappings.h"
#include <algorithm>
#include <cctype>
#include <vector>
#include <sstream>
#include <iterator>


#if defined (TDE2_USE_WIN32PLATFORM)

#include <d3dcompiler.h>

#pragma comment(lib, "D3Dcompiler.lib")


namespace TDEngine2
{
	U32 CD3D11ShaderCompiler::mMaxStepsCount = 1000;

	const C8* CD3D11ShaderCompiler::mVSEntryPointDefineName  = "VERTEX_ENTRY";

	const C8* CD3D11ShaderCompiler::mPSEntryPointDefineName  = "PIXEL_ENTRY";

	const C8* CD3D11ShaderCompiler::mGSEntryPointDefineName  = "GEOMETRY_ENTRY";

	const C8* CD3D11ShaderCompiler::mTargetVersionDefineName = "TARGET";


	CD3D11ShaderCompiler::CD3D11ShaderCompiler():
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CD3D11ShaderCompiler::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D11ShaderCompiler::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	TShaderCompilerResult CD3D11ShaderCompiler::Compile(const std::string& source) const
	{
		if (source.empty())
		{
			return TShaderCompilerResult(RC_INVALID_ARGS);
		}

		TShaderCompilerResult result;
		
		/// parse source code to get a meta information about it
		TShaderMetadata shaderMetadata = _parseShader(source);
		
		/// try to compile a vertex shader
		TCompileShaderStageResult vertexShaderOutput = _compileShaderStage(SST_VERTEX, source, shaderMetadata.mVertexShaderEntrypointName, 
																		   shaderMetadata.mTargetVersion);

		if (vertexShaderOutput.mResultCode != RC_OK)
		{
			return TShaderCompilerResult(vertexShaderOutput.mResultCode);
		}

		result.mVSByteCode = std::move(vertexShaderOutput.mByteCode);

		/// try to compile a pixel shader
		TCompileShaderStageResult pixelShaderOutput = _compileShaderStage(SST_PIXEL, source, shaderMetadata.mPixelShaderEntrypointName,
																		  shaderMetadata.mTargetVersion);
		
		if (pixelShaderOutput.mResultCode != RC_OK)
		{
			return TShaderCompilerResult(pixelShaderOutput.mResultCode);
		}

		result.mPSByteCode = std::move(pixelShaderOutput.mByteCode);
		
		/// try to compile a geometry shader
		TCompileShaderStageResult geometryShaderOutput = _compileShaderStage(SST_GEOMETRY, source, shaderMetadata.mGeometryShaderEntrypointName,
																			 shaderMetadata.mTargetVersion);

		if (geometryShaderOutput.mResultCode != RC_OK)
		{
			return TShaderCompilerResult(geometryShaderOutput.mResultCode);
		}

		result.mPSByteCode = std::move(geometryShaderOutput.mByteCode);

		return result;
	}

	CD3D11ShaderCompiler::TCompileShaderStageResult CD3D11ShaderCompiler::_compileShaderStage(E_SHADER_STAGE_TYPE shaderStage, const std::string& source, 
																							  const std::string& entryPointName, E_SHADER_TARGET_VERSION targetVersion) const
	{
		TCompileShaderStageResult result;

		std::vector<U8> byteCodeArray;

		ID3DBlob* pBytecodeBuffer = nullptr;
		ID3DBlob* pErrorBuffer    = nullptr;

		//enable only needed stage within the source code with a preprocessor
		D3D_SHADER_MACRO defines[] =
		{
			{ _getShaderStageDefineName(shaderStage), "1" }
		};

		if (FAILED(D3DCompile(source.c_str(), source.length(), nullptr, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
							  entryPointName.c_str(), CD3D11Mappings::GetShaderTargetVerStr(shaderStage, targetVersion).c_str(),
							  0x0, 0x0, &pBytecodeBuffer, &pErrorBuffer)))
		{
			result.mResultCode = RC_FAIL;

			return result;
		}

		U32 size = pBytecodeBuffer->GetBufferSize();

		const U8* pBuffer = static_cast<const U8*>(pBytecodeBuffer->GetBufferPointer());

		result.mByteCode.resize(size);
		std::copy(pBuffer, pBuffer + size, std::back_inserter(result.mByteCode));

		result.mResultCode = RC_OK;

		return result;
	}

	const C8* CD3D11ShaderCompiler::_getShaderStageDefineName(E_SHADER_STAGE_TYPE shaderStage) const
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

	CD3D11ShaderCompiler::TShaderMetadata CD3D11ShaderCompiler::_parseShader(const std::string& sourceCode) const
	{
		TShaderMetadata extractedMetadata {};

		std::string processedSourceCode = _removeComments(sourceCode);

		std::istringstream sourceCodeStream(sourceCode);

		/// split source code into tokens
		std::vector<std::string> tokens(std::istream_iterator<std::string>{ sourceCodeStream }, std::istream_iterator<std::string>());
		
		U32 tokensCount = tokens.size();

		std::string currToken;
		
		auto iter = tokens.cbegin();

		while (iter != tokens.cend())
		{
			currToken = *iter;

			if (currToken == "#define")
			{
				extractedMetadata.mDefines.emplace(_processDefineStmt(tokens, ++iter));
			}

			++iter;
		}

		extractedMetadata.mVertexShaderEntrypointName   = extractedMetadata.mDefines[mVSEntryPointDefineName];
		extractedMetadata.mPixelShaderEntrypointName    = extractedMetadata.mDefines[mPSEntryPointDefineName];
		extractedMetadata.mGeometryShaderEntrypointName = extractedMetadata.mDefines[mGSEntryPointDefineName];

		///\todo implement convertation of a version string into E_SHADER_TARGET_VERSION enum's value
		extractedMetadata.mTargetVersion = _getTargetVersionFromStr(extractedMetadata.mDefines[mTargetVersionDefineName]);

		return extractedMetadata;
	}

	std::string CD3D11ShaderCompiler::_removeComments(const std::string& sourceCode) const
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

		/// remove spaces between # and preprocessor directives
		bool isPrevChHash = false;

		processedSourceCode.erase(std::remove_if(processedSourceCode.begin(), processedSourceCode.end(), [&isPrevChHash](C8 ch) 
		{
			bool isCurrChHash = (ch == '#');

			isPrevChHash = isCurrChHash;

			return std::isspace(ch) && isPrevChHash;
		}), processedSourceCode.end());

		U32 firstPos  = 0;
		U32 secondPos = 0;
		
		/// remove single-line comments
		while ((firstPos = processedSourceCode.find_first_of("//")) != std::string::npos)
		{
			secondPos = (processedSourceCode.find_first_of('\n', firstPos) == std::string::npos) ? processedSourceCode.find_first_of('\r', firstPos) : std::string::npos;

			if (secondPos == std::string::npos)
			{
				continue;
			}

			processedSourceCode = processedSourceCode.substr(0, firstPos) + processedSourceCode.substr(secondPos + 1, processedSourceCode.length() - secondPos);
		}

		/// remove multi-line C style comments (use a finite state machine for this)

		U8 state        = 0; // could be 0 is an open comment state, 1 is any char within the block or a new comment block, 2 is nested comment block state
		U8 stepsCounter = 0;

		while ((stepsCounter++ <= mMaxStepsCount) && ((state == 0) && (firstPos = processedSourceCode.find_first_of("/*") != std::string::npos)))
		{
			state = 1;

			if ((secondPos = processedSourceCode.find_first_of("*/", firstPos + 1)) != std::string::npos)
			{
				state = 0;

				/// cut current string
				processedSourceCode = processedSourceCode.substr(0, firstPos) + processedSourceCode.substr(secondPos + 1, processedSourceCode.length() - secondPos);

				break;
			}

			if ((secondPos = processedSourceCode.find_first_of("/*", firstPos + 1)) != std::string::npos)
			{
				state = 2;

				if (processedSourceCode.find_first_of("*/", secondPos + 1) != std::string::npos)
				{
					state = 1;

					continue;
				}
			}
		}

		return processedSourceCode;
	}

	CD3D11ShaderCompiler::TShaderDefineDesc CD3D11ShaderCompiler::_processDefineStmt(const std::vector<std::string>& tokens,
																					 const std::vector<std::string>::const_iterator& tokenIter) const
	{		
		std::string defineName = *tokenIter;

		auto iter = tokenIter + 1;

		std::string defineValue;

		while ((iter != tokens.cend()) && (*iter != "\n"))
		{
			defineValue.append(*iter);
		}

		return std::make_pair(defineName, defineValue);
	}

	E_SHADER_TARGET_VERSION CD3D11ShaderCompiler::_getTargetVersionFromStr(const std::string& ver) const
	{
		if (ver == "3_0")
		{
			return STV_3_0;
		}

		if (ver == "4_0")
		{
			return STV_4_0;
		}

		return STV_5_0;
	}
		

	TDE2_API IShaderCompiler* CreateD3D11ShaderCompiler(E_RESULT_CODE& result)
	{
		CD3D11ShaderCompiler* pShaderCompilerInstance = new (std::nothrow) CD3D11ShaderCompiler();

		if (!pShaderCompilerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pShaderCompilerInstance->Init();

		if (result != RC_OK)
		{
			delete pShaderCompilerInstance;

			pShaderCompilerInstance = nullptr;
		}

		return pShaderCompilerInstance;
	}
}

#endif