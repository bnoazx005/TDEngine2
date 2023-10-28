#include "./../include/COGLShaderCompiler.h"
#include "./../include/COGLMappings.h"
#include <glslang/Include/ShHandle.h>
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/SpvTools.h>
#include <SPIRV/GlslangToSpv.h>
#include <core/IFileSystem.h>
#include <stringUtils.hpp>
#include <platform/CTextFileReader.h>
#include <utils/CFileLogger.h>
#include <editor/CPerfProfiler.h>
#include <core/CProjectSettings.h>
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
		if (GLEW_ARB_spirv_extensions)
		{
			glslang::InitializeProcess();
		}
	}

	TResult<TShaderCompilerOutput*> COGLShaderCompiler::Compile(const std::string& source) const
	{
		TDE2_PROFILER_SCOPE("COGLShaderCompiler::Compile");

		if (source.empty())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}
		
		auto preprocessorResult = CShaderPreprocessor::PreprocessSource(mpFileSystem, "#define TDE2_GLSL_SHADER\n" + source).Get();

		std::string preprocessedSource = preprocessorResult.mPreprocessedSource;

		/// parse source code to get a meta information about it
		CTokenizer tokenizer(preprocessedSource);

		TShaderMetadata shaderMetadata = _parseShader(tokenizer, preprocessorResult.mDefinesTable, preprocessorResult.mStagesRegions);
		shaderMetadata.mColorDataUniforms = std::move(preprocessorResult.mColorDataUniforms);

		MarkColorDataUniforms(shaderMetadata);

		auto compileShaderStagesResult = 
			/*GLEW_ARB_spirv_extensions && CProjectSettings::Get()->mGraphicsSettings.mIsShaderCacheEnabled ?
			_compileAllStagesToSPIRV(preprocessedSource, shaderMetadata) :*/ _compileAllStagesInRuntime(preprocessedSource, shaderMetadata);

		if (compileShaderStagesResult.HasError())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(compileShaderStagesResult.GetError());
		}

		TOGLShaderCompilerOutput* pResult = compileShaderStagesResult.Get();

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

	TResult<TOGLShaderCompilerOutput*> COGLShaderCompiler::_compileAllStagesInRuntime(const std::string& source, const TShaderMetadata& shaderMetadata) const
	{
		TOGLShaderCompilerOutput* pResult = new TOGLShaderCompilerOutput();

		if (_isShaderStageEnabled(SST_VERTEX, shaderMetadata))
		{
			/// try to compile a vertex shader
			TResult<GLuint> vertexShaderOutput = _compileShaderStage(SST_VERTEX, source, shaderMetadata);

			if (vertexShaderOutput.HasError())
			{
				return Wrench::TErrValue<E_RESULT_CODE>(vertexShaderOutput.GetError());
			}

			pResult->mVertexShaderHandler = vertexShaderOutput.Get();
		}

		if (_isShaderStageEnabled(SST_PIXEL, shaderMetadata))
		{
			/// try to compile a pixel shader
			TResult<GLuint> pixelShaderOutput = _compileShaderStage(SST_PIXEL, source, shaderMetadata);

			if (pixelShaderOutput.HasError())
			{
				return Wrench::TErrValue<E_RESULT_CODE>(pixelShaderOutput.GetError());
			}

			pResult->mFragmentShaderHandler = pixelShaderOutput.Get();
		}

		if (_isShaderStageEnabled(SST_GEOMETRY, shaderMetadata))
		{
			/// try to compile a geometry shader
			TResult<GLuint> geometryShaderOutput = _compileShaderStage(SST_GEOMETRY, source, shaderMetadata);

			if (geometryShaderOutput.HasError())
			{
				return Wrench::TErrValue<E_RESULT_CODE>(geometryShaderOutput.GetError());
			}

			pResult->mGeometryShaderHandler = geometryShaderOutput.Get();
		}

		if (_isShaderStageEnabled(SST_COMPUTE, shaderMetadata))
		{
			/// \todo Add verification

			/// try to compile a compute shader
			TResult<GLuint> computeShaderOutput = _compileShaderStage(SST_COMPUTE, source, shaderMetadata);

			if (computeShaderOutput.HasError())
			{
				return Wrench::TErrValue<E_RESULT_CODE>(computeShaderOutput.GetError());
			}

			pResult->mComputeShaderHandler = computeShaderOutput.Get();
		}

		return Wrench::TOkValue<TOGLShaderCompilerOutput*>(pResult);
	}

	TResult<TOGLShaderCompilerOutput*> COGLShaderCompiler::_compileAllStagesToSPIRV(const std::string& source, const TShaderMetadata& shaderMetadata) const
	{
		TOGLShaderCompilerOutput* pResult = new TOGLShaderCompilerOutput();

		for (U32 shaderStage = static_cast<U32>(SST_VERTEX); shaderStage != static_cast<U32>(SST_NONE); shaderStage++)
		{
			if (!_isShaderStageEnabled(static_cast<E_SHADER_STAGE_TYPE>(shaderStage), shaderMetadata))
			{
				continue;
			}

			TResult<std::vector<U8>> shaderOutput = _compileSPIRVShaderStage(static_cast<E_SHADER_STAGE_TYPE>(shaderStage), source, shaderMetadata);

			if (shaderOutput.HasError())
			{
				return Wrench::TErrValue<E_RESULT_CODE>(shaderOutput.GetError());
			}

			TShaderStageInfoDesc stageInfo;
			stageInfo.mBytecode = std::move(shaderOutput.Get());
			stageInfo.mEntrypointName = shaderMetadata.mEntrypointsTable.at(static_cast<E_SHADER_STAGE_TYPE>(shaderStage));

			pResult->mStagesInfo.emplace(static_cast<E_SHADER_STAGE_TYPE>(shaderStage), stageInfo);
		}

		return Wrench::TOkValue<TOGLShaderCompilerOutput*>(pResult);
	}


	static EShLanguage GetShaderLanguageByStage(E_SHADER_STAGE_TYPE shaderStage)
	{
		switch (shaderStage)
		{
			case E_SHADER_STAGE_TYPE::SST_VERTEX:
				return EShLangVertex;
			case E_SHADER_STAGE_TYPE::SST_PIXEL:
				return EShLangFragment;
			case E_SHADER_STAGE_TYPE::SST_GEOMETRY:
				return EShLangGeometry;
			case E_SHADER_STAGE_TYPE::SST_COMPUTE:
				return EShLangCompute;
		}

		return EShLangVertex;
	}


	const TBuiltInResource DefaultSPIRVBuiltInResource = {
		/* .MaxLights = */ 32,
		/* .MaxClipPlanes = */ 6,
		/* .MaxTextureUnits = */ 32,
		/* .MaxTextureCoords = */ 32,
		/* .MaxVertexAttribs = */ 64,
		/* .MaxVertexUniformComponents = */ 4096,
		/* .MaxVaryingFloats = */ 64,
		/* .MaxVertexTextureImageUnits = */ 32,
		/* .MaxCombinedTextureImageUnits = */ 80,
		/* .MaxTextureImageUnits = */ 32,
		/* .MaxFragmentUniformComponents = */ 4096,
		/* .MaxDrawBuffers = */ 32,
		/* .MaxVertexUniformVectors = */ 128,
		/* .MaxVaryingVectors = */ 8,
		/* .MaxFragmentUniformVectors = */ 16,
		/* .MaxVertexOutputVectors = */ 16,
		/* .MaxFragmentInputVectors = */ 15,
		/* .MinProgramTexelOffset = */ -8,
		/* .MaxProgramTexelOffset = */ 7,
		/* .MaxClipDistances = */ 8,
		/* .MaxComputeWorkGroupCountX = */ 65535,
		/* .MaxComputeWorkGroupCountY = */ 65535,
		/* .MaxComputeWorkGroupCountZ = */ 65535,
		/* .MaxComputeWorkGroupSizeX = */ 1024,
		/* .MaxComputeWorkGroupSizeY = */ 1024,
		/* .MaxComputeWorkGroupSizeZ = */ 64,
		/* .MaxComputeUniformComponents = */ 1024,
		/* .MaxComputeTextureImageUnits = */ 16,
		/* .MaxComputeImageUniforms = */ 8,
		/* .MaxComputeAtomicCounters = */ 8,
		/* .MaxComputeAtomicCounterBuffers = */ 1,
		/* .MaxVaryingComponents = */ 60,
		/* .MaxVertexOutputComponents = */ 64,
		/* .MaxGeometryInputComponents = */ 64,
		/* .MaxGeometryOutputComponents = */ 128,
		/* .MaxFragmentInputComponents = */ 128,
		/* .MaxImageUnits = */ 8,
		/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
		/* .MaxCombinedShaderOutputResources = */ 8,
		/* .MaxImageSamples = */ 0,
		/* .MaxVertexImageUniforms = */ 0,
		/* .MaxTessControlImageUniforms = */ 0,
		/* .MaxTessEvaluationImageUniforms = */ 0,
		/* .MaxGeometryImageUniforms = */ 0,
		/* .MaxFragmentImageUniforms = */ 8,
		/* .MaxCombinedImageUniforms = */ 8,
		/* .MaxGeometryTextureImageUnits = */ 16,
		/* .MaxGeometryOutputVertices = */ 256,
		/* .MaxGeometryTotalOutputComponents = */ 1024,
		/* .MaxGeometryUniformComponents = */ 1024,
		/* .MaxGeometryVaryingComponents = */ 64,
		/* .MaxTessControlInputComponents = */ 128,
		/* .MaxTessControlOutputComponents = */ 128,
		/* .MaxTessControlTextureImageUnits = */ 16,
		/* .MaxTessControlUniformComponents = */ 1024,
		/* .MaxTessControlTotalOutputComponents = */ 4096,
		/* .MaxTessEvaluationInputComponents = */ 128,
		/* .MaxTessEvaluationOutputComponents = */ 128,
		/* .MaxTessEvaluationTextureImageUnits = */ 16,
		/* .MaxTessEvaluationUniformComponents = */ 1024,
		/* .MaxTessPatchComponents = */ 120,
		/* .MaxPatchVertices = */ 32,
		/* .MaxTessGenLevel = */ 64,
		/* .MaxViewports = */ 16,
		/* .MaxVertexAtomicCounters = */ 0,
		/* .MaxTessControlAtomicCounters = */ 0,
		/* .MaxTessEvaluationAtomicCounters = */ 0,
		/* .MaxGeometryAtomicCounters = */ 0,
		/* .MaxFragmentAtomicCounters = */ 8,
		/* .MaxCombinedAtomicCounters = */ 8,
		/* .MaxAtomicCounterBindings = */ 1,
		/* .MaxVertexAtomicCounterBuffers = */ 0,
		/* .MaxTessControlAtomicCounterBuffers = */ 0,
		/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
		/* .MaxGeometryAtomicCounterBuffers = */ 0,
		/* .MaxFragmentAtomicCounterBuffers = */ 1,
		/* .MaxCombinedAtomicCounterBuffers = */ 1,
		/* .MaxAtomicCounterBufferSize = */ 16384,
		/* .MaxTransformFeedbackBuffers = */ 4,
		/* .MaxTransformFeedbackInterleavedComponents = */ 64,
		/* .MaxCullDistances = */ 8,
		/* .MaxCombinedClipAndCullDistances = */ 8,
		/* .MaxSamples = */ 4,
		/* .maxMeshOutputVerticesNV = */ 256,
		/* .maxMeshOutputPrimitivesNV = */ 512,
		/* .maxMeshWorkGroupSizeX_NV = */ 32,
		/* .maxMeshWorkGroupSizeY_NV = */ 1,
		/* .maxMeshWorkGroupSizeZ_NV = */ 1,
		/* .maxTaskWorkGroupSizeX_NV = */ 32,
		/* .maxTaskWorkGroupSizeY_NV = */ 1,
		/* .maxTaskWorkGroupSizeZ_NV = */ 1,
		/* .maxMeshViewCountNV = */ 4,
		/* .maxMeshOutputVerticesEXT = */ 256,
		/* .maxMeshOutputPrimitivesEXT = */ 256,
		/* .maxMeshWorkGroupSizeX_EXT = */ 128,
		/* .maxMeshWorkGroupSizeY_EXT = */ 128,
		/* .maxMeshWorkGroupSizeZ_EXT = */ 128,
		/* .maxTaskWorkGroupSizeX_EXT = */ 128,
		/* .maxTaskWorkGroupSizeY_EXT = */ 128,
		/* .maxTaskWorkGroupSizeZ_EXT = */ 128,
		/* .maxMeshViewCountEXT = */ 4,
		/* .maxDualSourceDrawBuffersEXT = */ 1,

		/* .limits = */ {
			/* .nonInductiveForLoops = */ 1,
			/* .whileLoops = */ 1,
			/* .doWhileLoops = */ 1,
			/* .generalUniformIndexing = */ 1,
			/* .generalAttributeMatrixVectorIndexing = */ 1,
			/* .generalVaryingIndexing = */ 1,
			/* .generalSamplerIndexing = */ 1,
			/* .generalVariableIndexing = */ 1,
			/* .generalConstantMatrixVectorIndexing = */ 1,
		} };


	TResult<std::vector<U8>> COGLShaderCompiler::_compileSPIRVShaderStage(E_SHADER_STAGE_TYPE shaderStage, const std::string& source, const TShaderMetadata& shaderMetadata) const
	{
		std::string processedShaderSource = _enableShaderStage(shaderStage, shaderMetadata.mShaderStagesRegionsInfo, source);
		
		const C8* shaderSource = processedShaderSource.c_str();
		const int shaderSourceLength = static_cast<int>(strlen(shaderSource));

		const auto shaderLangType = GetShaderLanguageByStage(shaderStage);

		std::unique_ptr<glslang::TShader> pShader = std::make_unique<glslang::TShader>(shaderLangType);

		pShader->setStringsWithLengths(&shaderSource, &shaderSourceLength, 1);
		pShader->setSourceEntryPoint(shaderMetadata.mEntrypointsTable.at(shaderStage).c_str());
		pShader->setEnvInput(glslang::EShSourceGlsl, shaderLangType, glslang::EShClientOpenGL, 100);
		pShader->setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
		pShader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

		EShMessages messages = EShMsgSpvRules;

		glslang::TShader::ForbidIncluder includer; // \note We don't need includer here because we process source on our side

		if (!pShader->parse(&DefaultSPIRVBuiltInResource, 110, false, EShMsgSpvRules, includer))
		{
			LOG_ERROR(Wrench::StringUtils::Format("[GL Shader Compiler] {0}", pShader->getInfoLog()));
			LOG_ERROR(Wrench::StringUtils::Format("[GL Shader Compiler] {0}", pShader->getInfoDebugLog()));

			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		glslang::TIntermediate* pIRCode = pShader->getIntermediate();
		
		std::vector<U32> spirv;
		spv::SpvBuildLogger logger;
		glslang::SpvOptions spvOptions;
		
		spvOptions.validate = true;
		spvOptions.compileOnly = false;

		glslang::GlslangToSpv(*pIRCode, spirv, &logger, &spvOptions);

		std::vector<U8> bytecode;
		bytecode.resize(spirv.size() * sizeof(U32));

		memcpy(bytecode.data(), spirv.data(), bytecode.size());

		return Wrench::TOkValue<std::vector<U8>>(bytecode);
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
	
	USIZE COGLShaderCompiler::_getBuiltinTypeSize(const std::string& type, const std::function<void(const std::string&)> typeProcessor) const
	{
		USIZE pos = type.find_first_of("1234");

		USIZE size = 1;

		if (pos != std::string::npos)
		{
			size = (std::max)(1, static_cast<I32>(type[pos]) - static_cast<I32>('0'));
			assert(size >= 1 && size <= 4);
		}

		if (typeProcessor)
		{
			typeProcessor(type);
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
			{ "image2D", E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE2D },
			{ "buffer", E_SHADER_RESOURCE_TYPE::SRT_RW_STRUCTURED_BUFFER },
			{ "buffer", E_SHADER_RESOURCE_TYPE::SRT_STRUCTURED_BUFFER },
		};

		auto result = shaderResourcesMap.find(token);

		return (result == shaderResourcesMap.cend()) ? E_SHADER_RESOURCE_TYPE::SRT_UNKNOWN : result->second;
	}


	TDE2_API IShaderCompiler* CreateOGLShaderCompiler(IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IShaderCompiler, COGLShaderCompiler, result, pFileSystem);
	}
}