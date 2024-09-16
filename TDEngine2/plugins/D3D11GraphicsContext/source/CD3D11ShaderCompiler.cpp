#include "./../include/CD3D11ShaderCompiler.h"
#include "./../include/CD3D11Mappings.h"
#include <stringUtils.hpp>
#include <utils/CFileLogger.h>
#include <algorithm>
#include <cctype>
#include <vector>
#include <sstream>
#include <iterator>
#include <graphics/IShader.h>
#include <core/IFileSystem.h>
#include <editor/CPerfProfiler.h>
#include <unordered_set>


#if defined (TDE2_USE_WINPLATFORM)

#include <d3d11shader.h>
#include <d3dcompiler.h>

#pragma comment(lib, "D3Dcompiler.lib")


namespace TDEngine2
{
	/*!
		class CD3D11ShaderCompiler

		\brief The class represents main compiler of shaders for D3D11 GAPI
	*/

	class CD3D11ShaderCompiler : public CBaseShaderCompiler
	{
		public:
			friend IShaderCompiler* CreateD3D11ShaderCompiler(IFileSystem* pFileSystem, E_RESULT_CODE& result);

			/*!
				\brief The method compiles specified source code into the bytecode representation.
				Note that the method allocates memory for TShaderCompilerOutput object on heap so it should be
				released manually

				\param[in] source A string that contains a source code of a shader

				\return An object that contains either bytecode or some error code. Note that the
				method allocates memory for TShaderCompilerOutput object on heap so it should be
				released manually
			*/

			TDE2_API TResult<TShaderCompilerOutput*> Compile(const std::string& shaderId, const std::string& source) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11ShaderCompiler)

				TDE2_API TResult<std::vector<U8>> _compileShaderStage(E_SHADER_STAGE_TYPE shaderStage, const std::string& source, TShaderMetadata& shaderMetadata,
					E_SHADER_FEATURE_LEVEL targetVersion) const;

			TDE2_API TUniformBuffersMap _processUniformBuffersDecls(const TStructDeclsMap& structsMap, CTokenizer& tokenizer) const override;

			TDE2_API E_SHADER_FEATURE_LEVEL _getTargetVersionFromStr(const std::string& ver) const override;

			TDE2_API USIZE _getBuiltinTypeSize(const std::string& type, const std::function<void(const std::string&)> typeProcessor = nullptr) const override;

			TDE2_API TShaderResourcesMap _processShaderResourcesDecls(CTokenizer& tokenizer) const override;

			TDE2_API E_SHADER_RESOURCE_TYPE _isShaderResourceType(const std::string& token) const;
	};


	CD3D11ShaderCompiler::CD3D11ShaderCompiler():
		CBaseShaderCompiler()
	{
	}

	TResult<TShaderCompilerOutput*> CD3D11ShaderCompiler::Compile(const std::string& shaderId, const std::string& source) const
	{
		TDE2_PROFILER_SCOPE("CD3D11ShaderCompiler::Compile");

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

		TD3D11ShaderCompilerOutput* pResult = new TD3D11ShaderCompilerOutput();

		for (U32 shaderStage = static_cast<U32>(SST_VERTEX); shaderStage != static_cast<U32>(SST_NONE); shaderStage++)
		{
			if (!_isShaderStageEnabled(static_cast<E_SHADER_STAGE_TYPE>(shaderStage), shaderMetadata))
			{
				continue;
			}

			/// try to compile a shader
			TResult<std::vector<U8>> shaderOutput = _compileShaderStage(static_cast<E_SHADER_STAGE_TYPE>(shaderStage), preprocessedSource, shaderMetadata, shaderMetadata.mFeatureLevel);
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

	TResult<std::vector<U8>> CD3D11ShaderCompiler::_compileShaderStage(E_SHADER_STAGE_TYPE shaderStage, const std::string& source,
																	   TShaderMetadata& shaderMetadata,
																	   E_SHADER_FEATURE_LEVEL targetVersion) const
	{
		TDE2_PROFILER_SCOPE("CD3D11ShaderCompiler::_compileShaderStage");

		std::vector<U8> byteCodeArray;

		ID3DBlob* pBytecodeBuffer = nullptr;
		ID3DBlob* pErrorBuffer    = nullptr;

		std::string processedSource = _enableShaderStage(shaderStage, shaderMetadata.mShaderStagesRegionsInfo, source);
		std::string entryPointName  = shaderMetadata.mEntrypointsTable[shaderStage];

		U32 flags = D3DCOMPILE_DEBUG | D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;

		{
			TDE2_PROFILER_SCOPE("D3DCompile");

			if (FAILED(D3DCompile(processedSource.c_str(), processedSource.length(), nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
				entryPointName.c_str(), CD3D11Mappings::GetShaderTargetVerStr(shaderStage, targetVersion).c_str(),
				flags, 0x0, &pBytecodeBuffer, &pErrorBuffer)))
			{
				LOG_ERROR(Wrench::StringUtils::Format("[D3D11 Shader Compiler] {0}", static_cast<const C8*>(pErrorBuffer->GetBufferPointer())));

				return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
			}
		}

		// Check up constant buffer sizes, extract their accurate sizes using the reflector
		ID3D11ShaderReflection* pReflector = nullptr;
		
		{
			TDE2_PROFILER_SCOPE("D3DReflect");

			if (FAILED(D3DReflect(pBytecodeBuffer->GetBufferPointer(), pBytecodeBuffer->GetBufferSize(), __uuidof(ID3D11ShaderReflection), (void**)&pReflector)))
			{
				return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
			}
		}

		D3D11_SHADER_BUFFER_DESC constantBufferInfoDesc;
		D3D11_SHADER_VARIABLE_DESC shaderVariableInfoDesc;
		D3D11_SHADER_TYPE_DESC shaderVariableTypeDesc;

		auto&& constantBuffers = shaderMetadata.mUniformBuffers;
		for (auto&& currConstantBuffer : constantBuffers)
		{
			// \note System constant buffers don't need any recomputation
			if (E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL == (currConstantBuffer.second.mFlags & E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL))
			{
				continue;
			}

			if (auto pConstantBufferData = pReflector->GetConstantBufferByName(currConstantBuffer.first.c_str()))
			{
				if (FAILED(pConstantBufferData->GetDesc(&constantBufferInfoDesc)))
				{
					continue;
				}

				currConstantBuffer.second.mSize = std::max<USIZE>(currConstantBuffer.second.mSize, static_cast<USIZE>(constantBufferInfoDesc.Size));
				
				for (TShaderUniformDesc& variableDesc : currConstantBuffer.second.mVariables)
				{
					if (ID3D11ShaderReflectionVariable* pVariableInfo = pConstantBufferData->GetVariableByName(variableDesc.mName.c_str()))
					{
						if (FAILED(pVariableInfo->GetDesc(&shaderVariableInfoDesc)))
						{
							continue;
						}
						
						if (auto pUniformType = pVariableInfo->GetType())
						{
							pUniformType->GetDesc(&shaderVariableTypeDesc);
							variableDesc.mTypeId = CBaseShaderCompiler::GetShaderBuiltInTypeId(shaderVariableTypeDesc.Name);
							variableDesc.mIsArray = static_cast<bool>(shaderVariableTypeDesc.Elements);
						}

						variableDesc.mSize = std::max<USIZE>(variableDesc.mSize, static_cast<USIZE>(shaderVariableInfoDesc.Size));
						variableDesc.mOffset = shaderVariableInfoDesc.StartOffset;

					}
				}
			}
		}

#if 0
		D3D11_SHADER_INPUT_BIND_DESC bindingInfo{};

		auto&& shaderResources = shaderMetadata.mShaderResources;
		for (auto&& currShaderResource : shaderResources)
		{
			if (FAILED(pReflector->GetResourceBindingDescByName(currShaderResource.first.c_str(), &bindingInfo)))
			{
				continue;
			}

			currShaderResource.second.mSlot = static_cast<U8>(bindingInfo.BindPoint);
		}
#endif

		USIZE size = pBytecodeBuffer->GetBufferSize();

		U8* pBuffer = static_cast<U8*>(pBytecodeBuffer->GetBufferPointer());

		byteCodeArray.reserve(size);

		std::copy(pBuffer, pBuffer + size, std::back_inserter(byteCodeArray));

		return Wrench::TOkValue<std::vector<U8>>(byteCodeArray);
	}

	CD3D11ShaderCompiler::TUniformBuffersMap CD3D11ShaderCompiler::_processUniformBuffersDecls(const TStructDeclsMap& structsMap, CTokenizer& tokenizer) const
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

	E_SHADER_FEATURE_LEVEL CD3D11ShaderCompiler::_getTargetVersionFromStr(const std::string& ver) const
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

	USIZE CD3D11ShaderCompiler::_getBuiltinTypeSize(const std::string& type, const std::function<void(const std::string&)> typeProcessor) const
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


	static U8 ExtractRegisterSlot(const std::string& registerInfo)
	{
		U8 slot = 0;

		const USIZE delimiterPos = registerInfo.find(',');
		if (std::string::npos != delimiterPos) // case of register of the following view ( [shader_profile], Type#[subcomponent] )
		{
			TDE2_UNIMPLEMENTED(); // unsupported for now
			return slot;
		}

		TDE2_ASSERT(registerInfo[0] == 'b' || registerInfo[0] == 't' || registerInfo[0] == 'c' || registerInfo[0] == 's' || registerInfo[0] == 'u');

		return static_cast<I8>(std::stoi(registerInfo.substr(1)));
	}


	CD3D11ShaderCompiler::TShaderResourcesMap CD3D11ShaderCompiler::_processShaderResourcesDecls(CTokenizer& tokenizer) const
	{
		TShaderResourcesMap shaderResources {};

		std::string currToken;
		std::string registerInfo;

		E_SHADER_RESOURCE_TYPE currType = E_SHADER_RESOURCE_TYPE::SRT_UNKNOWN;

		U8 currSlotIndex = 0;
		U8 currScopeIndex = 0; // \note 0th is a global scope

		std::unordered_set<U8> usedSlots;

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
			if (E_SHADER_RESOURCE_TYPE::SRT_STRUCTURED_BUFFER == currType || E_SHADER_RESOURCE_TYPE::SRT_RW_STRUCTURED_BUFFER == currType ||
				E_SHADER_RESOURCE_TYPE::SRT_RAW_BUFFER == currType || E_SHADER_RESOURCE_TYPE::SRT_RW_RAW_BUFFER == currType ||
				E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE2D == currType || E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE3D == currType)
			{
				while (currToken != ">")
				{
					currToken = tokenizer.GetNextToken();
				}
			}

			/// found a shader resource
			const std::string& resourceId = tokenizer.GetNextToken();

			const bool isWriteableResource = 
				E_SHADER_RESOURCE_TYPE::SRT_RW_STRUCTURED_BUFFER == currType ||
				E_SHADER_RESOURCE_TYPE::SRT_RW_RAW_BUFFER == currType ||
				E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE2D == currType ||
				E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE3D == currType;
			
			registerInfo.clear();

			currToken = tokenizer.GetNextToken();
			if (currToken == ":" && tokenizer.Peek(1) == "register")
			{
				tokenizer.GetNextToken(); // take register
				tokenizer.GetNextToken(); // take (

				while ((currToken = tokenizer.GetNextToken()) != ")")
				{
					registerInfo.append(currToken);
				}

				tokenizer.GetNextToken(); // take )
				tokenizer.GetNextToken(); // take ;
			}

			U8 currSlot = 0;
			
			if (registerInfo.empty())
			{
				do
				{
					currSlot = currSlotIndex++;
				} while (usedSlots.find(currSlot) != usedSlots.cend());
			}
			else
			{
				currSlot = ExtractRegisterSlot(registerInfo);
			}

			/// \note At this point only global scope's declarations should be passed
			shaderResources[resourceId] = { currType, currSlot, isWriteableResource};
			usedSlots.emplace(currSlot);
		}

		tokenizer.Reset();

		return shaderResources;
	}

	E_SHADER_RESOURCE_TYPE CD3D11ShaderCompiler::_isShaderResourceType(const std::string& token) const
	{
		static const std::unordered_map<std::string, E_SHADER_RESOURCE_TYPE> shaderResourcesMap
		{
			{ "Texture2D", E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D },
			{ "Texture2DArray", E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D_ARRAY },
			{ "Texture3D", E_SHADER_RESOURCE_TYPE::SRT_TEXTURE3D },
			{ "TextureCube", E_SHADER_RESOURCE_TYPE::SRT_TEXTURECUBE },
			{ "RWTexture2D", E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE2D },
			{ "RWTexture3D", E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE3D },
			{ "StructuredBuffer", E_SHADER_RESOURCE_TYPE::SRT_STRUCTURED_BUFFER },
			{ "AppendStructuredBuffer", E_SHADER_RESOURCE_TYPE::SRT_RW_STRUCTURED_BUFFER },
			{ "ConsumeStructuredBuffer", E_SHADER_RESOURCE_TYPE::SRT_RW_STRUCTURED_BUFFER },
			{ "RWStructuredBuffer", E_SHADER_RESOURCE_TYPE::SRT_RW_STRUCTURED_BUFFER },
			{ "RWBuffer", E_SHADER_RESOURCE_TYPE::SRT_RW_RAW_BUFFER },
			{ "Buffer", E_SHADER_RESOURCE_TYPE::SRT_RAW_BUFFER },
			{ "TextureCube", E_SHADER_RESOURCE_TYPE::SRT_TEXTURECUBE },
		};

		auto result = shaderResourcesMap.find(token);

		return (result == shaderResourcesMap.cend()) ? E_SHADER_RESOURCE_TYPE::SRT_UNKNOWN : result->second;
	}
	

	IShaderCompiler* CreateD3D11ShaderCompiler(IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IShaderCompiler, CD3D11ShaderCompiler, result, pFileSystem);
	}
}

#endif