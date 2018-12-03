#include "./../include/COGLShaderCompiler.h"
#include "./../include/COGLMappings.h"
#include <core/IFileSystem.h>
#include <platform/CTextFileReader.h>
#include <algorithm>
#include <cctype>
#include <vector>
#include <sstream>
#include <iterator>
#include <graphics/IShader.h>
#include <unordered_set>
#include <string>
#include <GL/glew.h>
#include <cstring>


namespace TDEngine2
{
	COGLShaderCompiler::COGLShaderCompiler() :
		CBaseShaderCompiler()
	{
	}

	TResult<TShaderCompilerOutput*> COGLShaderCompiler::Compile(const std::string& source) const
	{
		if (source.empty())
		{
			return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		std::string processedSourceCode = _removeComments(_preprocessShaderSource(_removeComments(source)));
		
		/// parse source code to get a meta information about it
		CTokenizer tokenizer(processedSourceCode);

		TShaderMetadata shaderMetadata = _parseShader(tokenizer);

		TOGLShaderCompilerOutput* pResult = new TOGLShaderCompilerOutput();

		if (_isShaderStageEnabled(SST_VERTEX, shaderMetadata))
		{
			/// try to compile a vertex shader
			TResult<GLuint> vertexShaderOutput = _compileShaderStage(SST_VERTEX, processedSourceCode, shaderMetadata.mDefines);

			if (vertexShaderOutput.HasError())
			{
				return TErrorValue<E_RESULT_CODE>(vertexShaderOutput.GetError());
			}

			pResult->mVertexShaderHandler = vertexShaderOutput.Get();
		}

		if (_isShaderStageEnabled(SST_PIXEL, shaderMetadata))
		{
			/// try to compile a pixel shader
			TResult<GLuint> pixelShaderOutput = _compileShaderStage(SST_PIXEL, processedSourceCode, shaderMetadata.mDefines);

			if (pixelShaderOutput.HasError())
			{
				return TErrorValue<E_RESULT_CODE>(pixelShaderOutput.GetError());
			}

			pResult->mFragmentShaderHandler = pixelShaderOutput.Get();
		}

		if (_isShaderStageEnabled(SST_GEOMETRY, shaderMetadata))
		{
			/// try to compile a geometry shader
			TResult<GLuint> geometryShaderOutput = _compileShaderStage(SST_GEOMETRY, processedSourceCode, shaderMetadata.mDefines);

			if (geometryShaderOutput.HasError())
			{
				return TErrorValue<E_RESULT_CODE>(geometryShaderOutput.GetError());
			}

			pResult->mGeometryShaderHandler = geometryShaderOutput.Get();
		}

		pResult->mUniformBuffersInfo = std::move(shaderMetadata.mUniformBuffers);
		
		return TOkValue<TShaderCompilerOutput*>(pResult);
	}
	
	TResult<GLuint> COGLShaderCompiler::_compileShaderStage(E_SHADER_STAGE_TYPE shaderStage, const std::string& source, const TDefinesMap& shaderDefinesMap) const
	{
		GLuint shaderHandler = glCreateShader(COGLMappings::GetShaderStageType(shaderStage));

		if (glGetError() != GL_NO_ERROR)
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}
		
		std::string processedShaderSource = _injectInternalDefines(shaderStage, source);

		const C8* shaderSource = processedShaderSource.c_str();

		I32 shaderSourceLength = strlen(shaderSource);

		glShaderSource(shaderHandler, 1, &shaderSource, &shaderSourceLength);

		if (glGetError() != GL_NO_ERROR)
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		glCompileShader(shaderHandler);

		if (glGetError() != GL_NO_ERROR)
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		GLint isCompiled = 0;

		glGetShaderiv(shaderHandler, GL_COMPILE_STATUS, &isCompiled);

		if (glGetError() != GL_NO_ERROR)
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (!isCompiled)
		{
			/// \todo reimplement error handling in other way
			GLint messageLength = 0;

			glGetShaderiv(shaderHandler, GL_INFO_LOG_LENGTH, &messageLength);

			std::vector<GLchar> tmpErrorMsgBuffer(messageLength);

			/// retrieve error message
			glGetShaderInfoLog(shaderHandler, messageLength, &messageLength, &tmpErrorMsgBuffer[0]);
			
			std::string errorMessageStr(tmpErrorMsgBuffer.begin(), tmpErrorMsgBuffer.end());

			glDeleteShader(shaderHandler);

			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}
		
		return TOkValue<GLuint>(shaderHandler);
	}
	
	COGLShaderCompiler::TDefinesMap COGLShaderCompiler::_processDefines(const std::string& sourceCode) const
	{
		TDefinesMap shaderDefinesMap = CBaseShaderCompiler::_processDefines(sourceCode);

		U32 firstPos  = 0;
		U32 secondPos = 0;
		U32 currPos   = 0;

		std::string versionStr;

		while ((firstPos = sourceCode.find_first_of('#', currPos)) != std::string::npos)
		{
			currPos = firstPos + 1;

			if ((secondPos = sourceCode.find("version", firstPos)) != std::string::npos)
			{
				/// 8 is "version" length + 1 space char
				versionStr = sourceCode.substr(secondPos + 8, sourceCode.find_first_of('\n', secondPos) - secondPos - 8);

				break;
			}			
		}

		shaderDefinesMap[_getTargetVersionDefineName()] = versionStr;

		return shaderDefinesMap;
	}


	COGLShaderCompiler::TUniformBuffersMap COGLShaderCompiler::_processUniformBuffersDecls(const TStructDeclsMap& structsMap, CTokenizer& tokenizer) const
	{
		TUniformBuffersMap uniformBuffersDecls = CBaseShaderCompiler::_processUniformBuffersDecls(structsMap, tokenizer);

		std::string currToken;
		std::string nextToken;

		std::string bufferName;

		std::unordered_set<U8> usedBufferSlots{ IUBR_PER_FRAME, IUBR_PER_OBJECT, IUBR_RARE_UDATED, IUBR_CONSTANTS };

		/*!
			\brief The function returns current passed index if its slot is still unused,
			or next free slot's index
		*/

		auto getCurrentOrNextFreeBufferSlot = [&usedBufferSlots](U8 index) -> U8
		{
			U8 slotIndex = index;

			while (usedBufferSlots.find(slotIndex) != usedBufferSlots.cend())
			{
				++slotIndex;
			}

			usedBufferSlots.insert(slotIndex);

			return slotIndex;
		};

		while (tokenizer.HasNext())
		{
			currToken = tokenizer.GetCurrToken();

			if (currToken != "uniform")
			{
				currToken = tokenizer.GetNextToken();

				continue;
			}

			if (tokenizer.Peek(3) == ";") /// single uniform variable has found, we're interested only in buffers
			{
				tokenizer.SeekByOffset(4);

				continue;
			}

			bufferName = tokenizer.GetNextToken();
									
			if ((nextToken = tokenizer.GetNextToken()) == "{")
			{
				if (uniformBuffersDecls.find(bufferName) == uniformBuffersDecls.cend())
				{
					uniformBuffersDecls[bufferName] = { getCurrentOrNextFreeBufferSlot(0), _getPaddedStructSize(structsMap, tokenizer) };
				}
				else
				{
					uniformBuffersDecls[bufferName].mSize = _getPaddedStructSize(structsMap, tokenizer);
				}
			}

			while ((nextToken = tokenizer.GetCurrToken()) != ";" && tokenizer.HasNext())
			{
				nextToken = tokenizer.GetNextToken();
			}

			nextToken = tokenizer.GetNextToken();
		}

		tokenizer.Reset();

		return uniformBuffersDecls;
	}

	E_SHADER_FEATURE_LEVEL COGLShaderCompiler::_getTargetVersionFromStr(const std::string& ver) const
	{
		if (ver.find("150") != std::string::npos)
		{
			return SFL_3_0;
		}

		if (ver.find("330") != std::string::npos)
		{
			return SFL_4_0;
		}

		return SFL_5_0;
	}

	std::string COGLShaderCompiler::_preprocessShaderSource(const std::string& source) const
	{
		std::string preprocessedSource = source;
		
		U32 firstPos  = 0;
		U32 secondPos = 0;
		U32 currPos   = 0;

		std::string headerFilename;

		ITextFileReader* pCurrHeaderFile = nullptr;

		E_RESULT_CODE result = RC_OK;

		/// process #include directive
		while ((firstPos = preprocessedSource.find('#', currPos)) != std::string::npos)
		{
			currPos = firstPos + 1;

			if ((firstPos = preprocessedSource.find("include", firstPos)) == std::string::npos ||
				(secondPos = preprocessedSource.find_first_of("<\"", firstPos)) == std::string::npos)
			{
				continue;
			}

			currPos = preprocessedSource.find_first_of(">\"\n", secondPos + 1);

			headerFilename = preprocessedSource.substr(secondPos + 1, currPos - secondPos - 1); /// extract name without brackets

			/// \todo implement absolute and relative paths based on the current shader's file
			pCurrHeaderFile = mpFileSystem->Create<CTextFileReader>(headerFilename, result);

			if (result != RC_OK)
			{
				if (pCurrHeaderFile)
				{
					pCurrHeaderFile->Close();
				}

				/// remove #include directive, which can't be loaded
				preprocessedSource = preprocessedSource.substr(0, firstPos - 1) + 
									 preprocessedSource.substr(currPos + 1, preprocessedSource.length() - currPos - 1);

				continue;
			}
			
			preprocessedSource = preprocessedSource.substr(0, firstPos - 1) +
								 pCurrHeaderFile->ReadToEnd() +
								 preprocessedSource.substr(currPos + 1, preprocessedSource.length() - currPos - 1);
		}

		return preprocessedSource;
	}

	std::string COGLShaderCompiler::_injectInternalDefines(E_SHADER_STAGE_TYPE shaderStage, const std::string& source) const
	{
		std::string newSourceCode;

		/// insert internal defines such as VERTEX/PIXEL/GEOMETRY after #version directive

		U32 pos = source.find("#version");

		if (pos == std::string::npos)
		{
			return source; /// doesn't change anything
		}

		U32 endPos = source.find('\n', pos + 1);

		newSourceCode = source.substr(0, endPos);

		newSourceCode.append("\n#define ").append(_getShaderStageDefineName(shaderStage)).append(" 1\n")
					 .append("\n#define ").append("TDE2_GLSL_SHADER\n");

		newSourceCode.append(source.substr(endPos, source.length() - endPos));

		return newSourceCode;
	}
	
	U32 COGLShaderCompiler::_getBuiltinTypeSize(const std::string& type) const
	{
		U32 pos = type.find_first_of("1234");

		U32 size = 1;

		if (pos != std::string::npos)
		{
			switch (type[pos])
			{
				case '2':
					size = 2;
					break;
				case '3':
					size = 3;
					break;
				case '4':
					size = 4;
					break;
			}
		}

		std::string baseType = type.substr(0, pos);

		if (baseType == "double" || baseType == "dvec")
		{
			return size * 8; // 8 is sizeof(double) in HLSL
		}
		else if (baseType == "mat") /// matrix wtih single precision floating-point elements
		{
			return size * size * 4;
		}
		else if (baseType == "dmat") /// matrix wtih double precision floating-point elements
		{
			return size * size * 8;
		}

		return size * 4; // other types sizes equal to 4 bytes
	}


	TDE2_API IShaderCompiler* CreateOGLShaderCompiler(IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		COGLShaderCompiler* pShaderCompilerInstance = new (std::nothrow) COGLShaderCompiler();

		if (!pShaderCompilerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pShaderCompilerInstance->Init(pFileSystem);

		if (result != RC_OK)
		{
			delete pShaderCompilerInstance;

			pShaderCompilerInstance = nullptr;
		}

		return pShaderCompilerInstance;
	}
}