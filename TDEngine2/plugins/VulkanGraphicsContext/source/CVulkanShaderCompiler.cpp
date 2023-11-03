#include "../include/CVulkanShaderCompiler.h"
#include "../include/CVulkanMappings.h"
#include <editor/CPerfProfiler.h>
#include <utils/CFileLogger.h>
#include <shaderc/shaderc.hpp>
#include "stringUtils.hpp"


namespace TDEngine2
{
	CVulkanShaderCompiler::CVulkanShaderCompiler():
		CBaseShaderCompiler()
	{
	}

	TResult<TShaderCompilerOutput*> CVulkanShaderCompiler::Compile(const std::string& shaderId, const std::string& source) const
	{
		TDE2_PROFILER_SCOPE("CVulkanShaderCompiler::Compile");

		if (source.empty())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}
		
		auto preprocessorResult = CShaderPreprocessor::PreprocessSource(mpFileSystem, "#define TDE2_HLSL_SHADER\n" + source).Get();

		std::string preprocessedSource = preprocessorResult.mPreprocessedSource;

		CTokenizer tokenizer(preprocessedSource);

		/// parse source code to get a meta information about it
		TShaderMetadata shaderMetadata = _parseShader(tokenizer, preprocessorResult.mDefinesTable, preprocessorResult.mStagesRegions);
		shaderMetadata.mColorDataUniforms = std::move(preprocessorResult.mColorDataUniforms);
		shaderMetadata.mShaderName = shaderId;

		TShaderCompilerOutput* pResult = new TShaderCompilerOutput();

		for (U32 shaderStage = static_cast<U32>(SST_VERTEX); shaderStage != static_cast<U32>(SST_NONE); shaderStage++)
		{
			if (!_isShaderStageEnabled(static_cast<E_SHADER_STAGE_TYPE>(shaderStage), shaderMetadata))
			{
				continue;
			}

			/// try to compile a shader
			TResult<std::vector<U8>> shaderOutput = _compileShaderStage(static_cast<E_SHADER_STAGE_TYPE>(shaderStage), preprocessedSource, shaderMetadata);
			if (shaderOutput.HasError())
			{
				return Wrench::TErrValue<E_RESULT_CODE>(shaderOutput.GetError());
			}

			TShaderStageInfoDesc stageInfo;
			stageInfo.mEntrypointName = shaderMetadata.mEntrypointsTable[static_cast<E_SHADER_STAGE_TYPE>(shaderStage)];
			stageInfo.mBytecode = std::move(shaderOutput.Get());

			pResult->mStagesInfo.emplace(static_cast<E_SHADER_STAGE_TYPE>(shaderStage), stageInfo);
		}

		MarkColorDataUniforms(shaderMetadata);

		pResult->mUniformBuffersInfo  = std::move(shaderMetadata.mUniformBuffers);
		pResult->mShaderResourcesInfo = std::move(shaderMetadata.mShaderResources);

		return Wrench::TOkValue<TShaderCompilerOutput*>(pResult);
	}


	static shaderc_shader_kind GetShadercInternalShaderStageType(E_SHADER_STAGE_TYPE shaderStage)
	{
		switch (shaderStage)
		{
			case E_SHADER_STAGE_TYPE::SST_VERTEX:
				return shaderc_vertex_shader;
			case E_SHADER_STAGE_TYPE::SST_GEOMETRY:
				return shaderc_geometry_shader;
			case E_SHADER_STAGE_TYPE::SST_PIXEL:
				return shaderc_fragment_shader;
			case E_SHADER_STAGE_TYPE::SST_COMPUTE:
				return shaderc_compute_shader;
		}

		TDE2_UNREACHABLE();
		return shaderc_vertex_shader;
	}


	TResult<std::vector<U8>> CVulkanShaderCompiler::_compileShaderStage(E_SHADER_STAGE_TYPE shaderStage, const std::string& source,
																	   TShaderMetadata& shaderMetadata) const
	{
		TDE2_PROFILER_SCOPE("CVulkanShaderCompiler::_compileShaderStage");

		std::string processedSource = _enableShaderStage(shaderStage, shaderMetadata.mShaderStagesRegionsInfo, source);
		std::string entryPointName  = shaderMetadata.mEntrypointsTable[shaderStage];

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetSourceLanguage(shaderc_source_language::shaderc_source_language_hlsl);

		shaderc::SpvCompilationResult module{};

		{
			TDE2_PROFILER_SCOPE("shaderc::CompileGlslToSpv");
			module = compiler.CompileGlslToSpv(source, GetShadercInternalShaderStageType(shaderStage), shaderMetadata.mShaderName.c_str(), options);
		}

		if (shaderc_compilation_status_success != module.GetCompilationStatus()) 
		{
			LOG_ERROR(Wrench::StringUtils::Format("[CVulkanShaderCompiler] Compilation error happened: {0}", module.GetErrorMessage()));
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		std::vector<U32> spirvBytecode{ module.cbegin(), module.cend() };
		std::vector<U8> byteCodeArray(spirvBytecode.size() * sizeof(U32));

		memcpy(byteCodeArray.data(), spirvBytecode.data(), byteCodeArray.size());

		return Wrench::TOkValue<std::vector<U8>>(byteCodeArray);
	}

	CVulkanShaderCompiler::TUniformBuffersMap CVulkanShaderCompiler::_processUniformBuffersDecls(const TStructDeclsMap& structsMap, CTokenizer& tokenizer) const
	{
		TUniformBuffersMap uniformBuffersDecls = CBaseShaderCompiler::_processUniformBuffersDecls(structsMap, tokenizer);

		std::string currToken;
		std::string nextToken;

		std::string bufferName;
				
		std::unordered_set<U8> usedBufferSlots { /*IUBR_PER_FRAME, IUBR_PER_OBJECT, IUBR_RARE_UDATED, IUBR_CONSTANTS*/ };

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

		U8 currSlotIndex = 0;

		while (tokenizer.HasNext())
		{
			currToken = tokenizer.GetCurrToken();

			if (currToken != "cbuffer")
			{
				currToken = tokenizer.GetNextToken();

				continue;
			}

			bufferName = tokenizer.GetNextToken();

			if ((nextToken = tokenizer.GetNextToken()) == ";") /// found cbuffer's declaration without a body
			{
				uniformBuffersDecls.insert({ bufferName, { getCurrentOrNextFreeBufferSlot(0), 0 } });

				continue;
			}

			if ((nextToken == ":") &&
				(tokenizer.Peek(1) == "register") &&
				(tokenizer.Peek(2) == "(")) /// found register(b#) decl
			{
				nextToken = tokenizer.SeekByOffset(3);

				if (nextToken[0] != 'b')
				{
					continue;
				}

				currSlotIndex = std::atoi(nextToken.substr(1, nextToken.length()).c_str());

				do
				{
					nextToken = tokenizer.GetNextToken();
				} 
				while (nextToken != "{" && tokenizer.HasNext());
			}
			
			if (nextToken == "{")
			{
				std::vector<TShaderUniformDesc> uniforms{};

				auto pushUniformInformation = [&uniforms](const TShaderUniformDesc& uniformInfo)
				{
					uniforms.push_back(uniformInfo);
				};

				uniformBuffersDecls.insert({ bufferName, { getCurrentOrNextFreeBufferSlot(currSlotIndex), _getPaddedStructSize(structsMap, tokenizer, pushUniformInformation) } });

				uniformBuffersDecls[bufferName].mVariables = std::move(uniforms);
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

	E_SHADER_FEATURE_LEVEL CVulkanShaderCompiler::_getTargetVersionFromStr(const std::string& ver) const
	{
		if (ver == "3_0")
		{
			return SFL_3_0;
		}

		if (ver == "4_0")
		{
			return SFL_4_0;
		}

		return SFL_5_0;
	}

	USIZE CVulkanShaderCompiler::_getBuiltinTypeSize(const std::string& type, const std::function<void(const std::string&)> typeProcessor) const
	{
		USIZE pos = type.find_first_of("1234");
		
		USIZE size = 1;

		std::string baseType = type.substr(0, pos);
		
		USIZE currPos = 0;

		if (typeProcessor)
		{
			typeProcessor(baseType);
		}

		while ((pos = type.find_first_of("1234", currPos)) != std::string::npos)
		{
			switch (type[pos])
			{
				case '2':
					size *= 2;
					break;
				case '3':
					size *= 3;
					break;
				case '4':
					size *= 4;
					break;
			}

			currPos = pos + 1;
		}
		
		if (baseType == "double")
		{
			return size * 8; // 8 is sizeof(double) in HLSL
		}
		else if (baseType == "half")
		{
			return size * 2; // 2 is sizeof(half) in HLSL
		}
			
		return size * 4; // other types sizes equal to 4 bytes
	}

	CVulkanShaderCompiler::TShaderResourcesMap CVulkanShaderCompiler::_processShaderResourcesDecls(CTokenizer& tokenizer) const
	{
		TShaderResourcesMap shaderResources {};

		std::string currToken;

		E_SHADER_RESOURCE_TYPE currType = E_SHADER_RESOURCE_TYPE::SRT_UNKNOWN;

		U8 currSlotIndex = 0;
		U8 currScopeIndex = 0; // \note 0th is a global scope

		while (tokenizer.HasNext())
		{
			currToken = tokenizer.GetCurrToken();

			if ((E_SHADER_RESOURCE_TYPE::SRT_UNKNOWN == (currType = _isShaderResourceType(currToken))) || (currScopeIndex > 0))
			{
				if (currToken == "{" || currToken == "(") {	++currScopeIndex; }
				if (currToken == ")" || currToken == "}") {	--currScopeIndex; }

				tokenizer.GetNextToken();

				continue;
			}

			/// skip template parameters
			if (E_SHADER_RESOURCE_TYPE::SRT_STRUCTURED_BUFFER == currType || E_SHADER_RESOURCE_TYPE::SRT_RW_STRUCTURED_BUFFER == currType)
			{
				while (currToken != ">")
				{
					currToken = tokenizer.GetNextToken();
				}
			}

			/// found a shader resource
			currToken = tokenizer.GetNextToken();

			/// \note At this point only global scope's declarations should be passed
			shaderResources[currToken] = { currType, currSlotIndex++ };
		}

		tokenizer.Reset();

		return shaderResources;
	}

	E_SHADER_RESOURCE_TYPE CVulkanShaderCompiler::_isShaderResourceType(const std::string& token) const
	{
		static const std::unordered_map<std::string, E_SHADER_RESOURCE_TYPE> shaderResourcesMap
		{
			{ "Texture2D", E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D },
			{ "Texture2DArray", E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D_ARRAY },
			{ "Texture3D", E_SHADER_RESOURCE_TYPE::SRT_TEXTURE3D },
			{ "TextureCube", E_SHADER_RESOURCE_TYPE::SRT_TEXTURECUBE },
			{ "RWTexture2D", E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE2D },
			{ "StructuredBuffer", E_SHADER_RESOURCE_TYPE::SRT_STRUCTURED_BUFFER },
			{ "RWStructuredBuffer", E_SHADER_RESOURCE_TYPE::SRT_RW_STRUCTURED_BUFFER },
			{ "TextureCube", E_SHADER_RESOURCE_TYPE::SRT_TEXTURECUBE },
		};

		auto result = shaderResourcesMap.find(token);

		return (result == shaderResourcesMap.cend()) ? E_SHADER_RESOURCE_TYPE::SRT_UNKNOWN : result->second;
	}
	

	TDE2_API IShaderCompiler* CreateVulkanShaderCompiler(IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IShaderCompiler, CVulkanShaderCompiler, result, pFileSystem);
	}
}