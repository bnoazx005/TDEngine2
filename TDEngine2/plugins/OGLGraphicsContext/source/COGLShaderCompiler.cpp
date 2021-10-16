#include "./../include/COGLShaderCompiler.h"
#include "./../include/COGLMappings.h"
#include <core/IFileSystem.h>
#include <stringUtils.hpp>
#include <platform/CTextFileReader.h>
#include <utils/CFileLogger.h>
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
#include <cassert>


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
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}
		
		auto preprocessorResult = CShaderPreprocessor::PreprocessSource(mpFileSystem, "#define TDE2_GLSL_SHADER\n" + source).Get();

		std::string preprocessedSource = preprocessorResult.mPreprocessedSource;

		/// parse source code to get a meta information about it
		CTokenizer tokenizer(preprocessedSource);

		TShaderMetadata shaderMetadata = _parseShader(tokenizer, preprocessorResult.mDefinesTable, preprocessorResult.mStagesRegions);
		
		TOGLShaderCompilerOutput* pResult = new TOGLShaderCompilerOutput();

		if (_isShaderStageEnabled(SST_VERTEX, shaderMetadata))
		{
			/// try to compile a vertex shader
			TResult<GLuint> vertexShaderOutput = _compileShaderStage(SST_VERTEX, preprocessedSource, shaderMetadata);

			if (vertexShaderOutput.HasError())
			{
				return Wrench::TErrValue<E_RESULT_CODE>(vertexShaderOutput.GetError());
			}

			pResult->mVertexShaderHandler = vertexShaderOutput.Get();
		}

		if (_isShaderStageEnabled(SST_PIXEL, shaderMetadata))
		{
			/// try to compile a pixel shader
			TResult<GLuint> pixelShaderOutput = _compileShaderStage(SST_PIXEL, preprocessedSource, shaderMetadata);

			if (pixelShaderOutput.HasError())
			{
				return Wrench::TErrValue<E_RESULT_CODE>(pixelShaderOutput.GetError());
			}

			pResult->mFragmentShaderHandler = pixelShaderOutput.Get();
		}

		if (_isShaderStageEnabled(SST_GEOMETRY, shaderMetadata))
		{
			/// try to compile a geometry shader
			TResult<GLuint> geometryShaderOutput = _compileShaderStage(SST_GEOMETRY, preprocessedSource, shaderMetadata);

			if (geometryShaderOutput.HasError())
			{
				return Wrench::TErrValue<E_RESULT_CODE>(geometryShaderOutput.GetError());
			}

			pResult->mGeometryShaderHandler = geometryShaderOutput.Get();
		}

		pResult->mUniformBuffersInfo  = std::move(shaderMetadata.mUniformBuffers);
		pResult->mShaderResourcesInfo = std::move(shaderMetadata.mShaderResources);
		
		return Wrench::TOkValue<TShaderCompilerOutput*>(pResult);
	}
	
	TResult<GLuint> COGLShaderCompiler::_compileShaderStage(E_SHADER_STAGE_TYPE shaderStage, const std::string& source, const TShaderMetadata& shaderMetadata) const
	{
		GLuint shaderHandler = glCreateShader(COGLMappings::GetShaderStageType(shaderStage));

		if (glGetError() != GL_NO_ERROR)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}
		
		std::string processedShaderSource = _enableShaderStage(shaderStage, shaderMetadata.mShaderStagesRegionsInfo, source);

		const C8* shaderSource = processedShaderSource.c_str();

		const GLint shaderSourceLength = static_cast<const GLint>(strlen(shaderSource));

		glShaderSource(shaderHandler, 1, &shaderSource, &shaderSourceLength);

		if (glGetError() != GL_NO_ERROR)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		glCompileShader(shaderHandler);

		if (glGetError() != GL_NO_ERROR)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		GLint isCompiled = 0;

		glGetShaderiv(shaderHandler, GL_COMPILE_STATUS, &isCompiled);

		if (glGetError() != GL_NO_ERROR)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
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
			LOG_ERROR(Wrench::StringUtils::Format("[GL Shader Compiler] {0}", errorMessageStr));
			
			glDeleteShader(shaderHandler);

			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}
		
		return Wrench::TOkValue<GLuint>(shaderHandler);
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

		auto uniformVariableFunctor = [&uniformBuffersDecls, &bufferName](auto&& uniformDesc)
		{
			uniformBuffersDecls[bufferName].mVariables.emplace_back(uniformDesc);
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
					auto& currBuffer = uniformBuffersDecls[bufferName];
					currBuffer.mSlot = getCurrentOrNextFreeBufferSlot(0);
					currBuffer.mSize = _getPaddedStructSize(structsMap, tokenizer, uniformVariableFunctor);
				}
				else
				{
					uniformBuffersDecls[bufferName].mSize = _getPaddedStructSize(structsMap, tokenizer, uniformVariableFunctor);
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
	
	USIZE COGLShaderCompiler::_getBuiltinTypeSize(const std::string& type) const
	{
		USIZE pos = type.find_first_of("1234");

		USIZE size = 1;

		if (pos != std::string::npos)
		{
			size = (std::max)(1, static_cast<I32>(type[pos]) - static_cast<I32>('0'));
			assert(size >= 1 && size <= 4);
		}

		std::string baseType = type.substr(0, pos);

		static const std::unordered_map<std::string, std::function<USIZE(USIZE)>> sizesTable
		{
			{ "double", [](USIZE size) { return size * 8; } },
			{ "dvec", [](USIZE size) { return size * 8; } },
			{ "mat", [](USIZE size) { return size * size * 4; } },
			{ "dmat", [](USIZE size) { return size * size * 8; } },
		};

		auto iter = sizesTable.find(baseType);
		if (iter == sizesTable.cend())
		{
			return size * 4;
		}

		return (iter->second)(size);
	}

	COGLShaderCompiler::TShaderResourcesMap COGLShaderCompiler::_processShaderResourcesDecls(CTokenizer& tokenizer) const
	{
		TShaderResourcesMap shaderResources{};

		std::string currToken;

		E_SHADER_RESOURCE_TYPE currType = E_SHADER_RESOURCE_TYPE::SRT_UNKNOWN;

		U8 currSlotIndex = 0;

		while (tokenizer.HasNext())
		{
			currToken = tokenizer.GetCurrToken();

			if ((currType = _isShaderResourceType(currToken)) == E_SHADER_RESOURCE_TYPE::SRT_UNKNOWN)
			{
				tokenizer.GetNextToken();

				continue;
			}

			/// found a shader resource
			currToken = tokenizer.GetNextToken();

			shaderResources[currToken] = { currType, currSlotIndex++ };
		}

		tokenizer.Reset();

		return shaderResources;
	}
	
	E_SHADER_RESOURCE_TYPE COGLShaderCompiler::_isShaderResourceType(const std::string& token) const
	{
		static const std::unordered_map<std::string, E_SHADER_RESOURCE_TYPE> shaderResourcesMap
		{
			{ "sampler2D", E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D },
			{ "isampler2D", E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D },
			{ "usampler2D", E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D },
			{ "sampler2DArray", E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D_ARRAY },
			{ "isampler2DArray", E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D_ARRAY },
			{ "usampler2DArray", E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D_ARRAY },
			{ "sampler3D", E_SHADER_RESOURCE_TYPE::SRT_TEXTURE3D },
			{ "isampler3D", E_SHADER_RESOURCE_TYPE::SRT_TEXTURE3D },
			{ "usampler3D", E_SHADER_RESOURCE_TYPE::SRT_TEXTURE3D },
			{ "samplerCube", E_SHADER_RESOURCE_TYPE::SRT_TEXTURECUBE },
			{ "isamplerCube", E_SHADER_RESOURCE_TYPE::SRT_TEXTURECUBE },
			{ "usamplerCube", E_SHADER_RESOURCE_TYPE::SRT_TEXTURECUBE },
		};

		auto result = shaderResourcesMap.find(token);

		return (result == shaderResourcesMap.cend()) ? E_SHADER_RESOURCE_TYPE::SRT_UNKNOWN : result->second;
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