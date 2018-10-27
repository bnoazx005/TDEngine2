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

	const C8* CD3D11ShaderCompiler::mEntryPointsDefineNames[3] = { "VERTEX_ENTRY", "PIXEL_ENTRY", "GEOMETRY_ENTRY" };

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
		
		if (_isShaderStageEnabled(SST_VERTEX, shaderMetadata))
		{
			/// try to compile a vertex shader
			TCompileShaderStageResult vertexShaderOutput = _compileShaderStage(SST_VERTEX, source, shaderMetadata.mVertexShaderEntrypointName,
																			   shaderMetadata.mDefines, shaderMetadata.mTargetVersion);

			if (vertexShaderOutput.mResultCode != RC_OK)
			{
				return TShaderCompilerResult(vertexShaderOutput.mResultCode);
			}

			result.mVSByteCode = std::move(vertexShaderOutput.mByteCode);
		}

		if (_isShaderStageEnabled(SST_PIXEL, shaderMetadata))
		{
			/// try to compile a pixel shader
			TCompileShaderStageResult pixelShaderOutput = _compileShaderStage(SST_PIXEL, source, shaderMetadata.mPixelShaderEntrypointName,
																			  shaderMetadata.mDefines, shaderMetadata.mTargetVersion);

			if (pixelShaderOutput.mResultCode != RC_OK)
			{
				return TShaderCompilerResult(pixelShaderOutput.mResultCode);
			}

			result.mPSByteCode = std::move(pixelShaderOutput.mByteCode);
		}
		
		if (_isShaderStageEnabled(SST_GEOMETRY, shaderMetadata))
		{
			/// try to compile a geometry shader
			TCompileShaderStageResult geometryShaderOutput = _compileShaderStage(SST_GEOMETRY, source, shaderMetadata.mGeometryShaderEntrypointName,
																				 shaderMetadata.mDefines, shaderMetadata.mTargetVersion);

			if (geometryShaderOutput.mResultCode != RC_OK)
			{
				return TShaderCompilerResult(geometryShaderOutput.mResultCode);
			}

			result.mPSByteCode = std::move(geometryShaderOutput.mByteCode);
		}

		return result;
	}

	CD3D11ShaderCompiler::TCompileShaderStageResult CD3D11ShaderCompiler::_compileShaderStage(E_SHADER_STAGE_TYPE shaderStage, const std::string& source, 
																							  const std::string& entryPointName, 
																							  const CD3D11ShaderCompiler::TDefinesMap& shaderDefinesMap,
																							  E_SHADER_TARGET_VERSION targetVersion) const
	{
		TCompileShaderStageResult result;

		std::vector<U8> byteCodeArray;

		ID3DBlob* pBytecodeBuffer = nullptr;
		ID3DBlob* pErrorBuffer    = nullptr;

		//enable only needed stage within the source code with a preprocessor
		D3D_SHADER_MACRO defines[] =
		{
			{ _getShaderStageDefineName(SST_VERTEX), shaderStage == SST_VERTEX ? "1" : "0" },
			{ _getShaderStageDefineName(SST_PIXEL), shaderStage == SST_PIXEL ? "1" : "0" },
			{ _getShaderStageDefineName(SST_GEOMETRY), shaderStage == SST_GEOMETRY ? "1" : "0" },
			{ nullptr, nullptr }
		};

		U32 flags = D3DCOMPILE_DEBUG;

		if (FAILED(D3DCompile(source.c_str(), source.length(), nullptr, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
							  entryPointName.c_str(), CD3D11Mappings::GetShaderTargetVerStr(shaderStage, targetVersion).c_str(),
							  flags, 0x0, &pBytecodeBuffer, &pErrorBuffer)))
		{
			result.mResultCode = RC_FAIL;

			return result;
		}

		U32 size = pBytecodeBuffer->GetBufferSize();

		U8* pBuffer = static_cast<U8*>(pBytecodeBuffer->GetBufferPointer());

		result.mByteCode.reserve(size);

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

		extractedMetadata.mDefines = _processDefines(processedSourceCode);

		extractedMetadata.mVertexShaderEntrypointName   = extractedMetadata.mDefines[mEntryPointsDefineNames[SST_VERTEX]];
		extractedMetadata.mPixelShaderEntrypointName    = extractedMetadata.mDefines[mEntryPointsDefineNames[SST_PIXEL]];
		extractedMetadata.mGeometryShaderEntrypointName = extractedMetadata.mDefines[mEntryPointsDefineNames[SST_GEOMETRY]];

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

		U32 firstPos  = 0;
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
				thirdPos  = processedSourceCode.find("/*", seekPos);

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
			} 
			while (seekPos < processedSourceCode.length() && numOfNestedCommentsBlocks > 0);
			
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

	CD3D11ShaderCompiler::TDefinesMap CD3D11ShaderCompiler::_processDefines(const std::string& sourceCode) const
	{		
		TDefinesMap defines;

		U32 firstPos  = 0;
		U32 secondPos = 0;
		U32 nextPos   = 0;

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

	bool CD3D11ShaderCompiler::_isShaderStageEnabled(E_SHADER_STAGE_TYPE shaderStage, const CD3D11ShaderCompiler::TShaderMetadata& shaderMeta) const
	{
		TDefinesMap::const_iterator defineIter = shaderMeta.mDefines.find(mEntryPointsDefineNames[shaderStage]);

		if (defineIter == shaderMeta.mDefines.cend() || 
			defineIter->second.empty())
		{
			return false;
		}

		return true;
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