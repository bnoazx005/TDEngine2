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
#include <unordered_set>


#if defined (TDE2_USE_WINPLATFORM)

#include <d3d11shader.h>
#include <d3dcompiler.h>

#pragma comment(lib, "D3Dcompiler.lib")


namespace TDEngine2
{
	CD3D11ShaderCompiler::CD3D11ShaderCompiler():
		CBaseShaderCompiler()
	{
	}

	TResult<TShaderCompilerOutput*> CD3D11ShaderCompiler::Compile(const std::string& source) const
	{
		if (source.empty())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}
		
		auto preprocessorResult = CShaderPreprocessor::PreprocessSource(mpFileSystem, "#define TDE2_HLSL_SHADER\n" + source).Get();

		std::string preprocessedSource = preprocessorResult.mPreprocessedSource;

		CTokenizer tokenizer(preprocessedSource);

		/// parse source code to get a meta information about it
		TShaderMetadata shaderMetadata = _parseShader(tokenizer, preprocessorResult.mDefinesTable, preprocessorResult.mStagesRegions);

		TD3D11ShaderCompilerOutput* pResult = new TD3D11ShaderCompilerOutput();

		if (_isShaderStageEnabled(SST_VERTEX, shaderMetadata))
		{
			/// try to compile a vertex shader
			TResult<std::vector<U8>> vertexShaderOutput = _compileShaderStage(SST_VERTEX, preprocessedSource, shaderMetadata, shaderMetadata.mFeatureLevel);

			if (vertexShaderOutput.HasError())
			{
				return Wrench::TErrValue<E_RESULT_CODE>(vertexShaderOutput.GetError());
			}

			pResult->mVSByteCode = std::move(vertexShaderOutput.Get());
		}

		if (_isShaderStageEnabled(SST_PIXEL, shaderMetadata))
		{
			/// try to compile a pixel shader
			TResult<std::vector<U8>> pixelShaderOutput = _compileShaderStage(SST_PIXEL, preprocessedSource, shaderMetadata, shaderMetadata.mFeatureLevel);

			if (pixelShaderOutput.HasError())
			{
				return Wrench::TErrValue<E_RESULT_CODE>(pixelShaderOutput.GetError());
			}

			pResult->mPSByteCode = std::move(pixelShaderOutput.Get());
		}

		if (_isShaderStageEnabled(SST_GEOMETRY, shaderMetadata))
		{
			/// try to compile a geometry shader
			TResult<std::vector<U8>> geometryShaderOutput = _compileShaderStage(SST_GEOMETRY, preprocessedSource, shaderMetadata, shaderMetadata.mFeatureLevel);

			if (geometryShaderOutput.HasError())
			{
				return Wrench::TErrValue<E_RESULT_CODE>(geometryShaderOutput.GetError());
			}

			pResult->mPSByteCode = std::move(geometryShaderOutput.Get());
		}

		pResult->mUniformBuffersInfo  = std::move(shaderMetadata.mUniformBuffers);
		pResult->mShaderResourcesInfo = std::move(shaderMetadata.mShaderResources);
		
		return Wrench::TOkValue<TShaderCompilerOutput*>(pResult);
	}

	TResult<std::vector<U8>> CD3D11ShaderCompiler::_compileShaderStage(E_SHADER_STAGE_TYPE shaderStage, const std::string& source,
																	   TShaderMetadata& shaderMetadata,
																	   E_SHADER_FEATURE_LEVEL targetVersion) const
	{
		std::vector<U8> byteCodeArray;

		ID3DBlob* pBytecodeBuffer = nullptr;
		ID3DBlob* pErrorBuffer    = nullptr;

		std::string processedSource = _enableShaderStage(shaderStage, shaderMetadata.mShaderStagesRegionsInfo, source);
		std::string entryPointName  = shaderMetadata.mVertexShaderEntrypointName;

		switch (shaderStage)
		{
			case E_SHADER_STAGE_TYPE::SST_PIXEL:
				entryPointName = shaderMetadata.mPixelShaderEntrypointName;
				break;
			case E_SHADER_STAGE_TYPE::SST_GEOMETRY:
				entryPointName = shaderMetadata.mGeometryShaderEntrypointName;
				break;
		}

		U32 flags = D3DCOMPILE_DEBUG | D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;

		if (FAILED(D3DCompile(processedSource.c_str(), processedSource.length(), nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
							  entryPointName.c_str(), CD3D11Mappings::GetShaderTargetVerStr(shaderStage, targetVersion).c_str(),
							  flags, 0x0, &pBytecodeBuffer, &pErrorBuffer)))
		{
			LOG_ERROR(Wrench::StringUtils::Format("[D3D11 Shader Compiler] {0}", static_cast<const C8*>(pErrorBuffer->GetBufferPointer())));

			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		// Check up constant buffer sizes, extract their accurate sizes using the reflector
		ID3D11ShaderReflection* pReflector = nullptr;
		
		if (FAILED(D3DReflect(pBytecodeBuffer->GetBufferPointer(), pBytecodeBuffer->GetBufferSize(), __uuidof(ID3D11ShaderReflection), (void**)&pReflector)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
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
						}

						variableDesc.mSize = std::max<USIZE>(variableDesc.mSize, static_cast<USIZE>(shaderVariableInfoDesc.Size));
						variableDesc.mOffset = shaderVariableInfoDesc.StartOffset;
					}
				}
			}
		}

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

	CD3D11ShaderCompiler::TShaderResourcesMap CD3D11ShaderCompiler::_processShaderResourcesDecls(CTokenizer& tokenizer) const
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

			/// found a shader resource
			currToken = tokenizer.GetNextToken();

			/// \note At this point only global scope's declarations should be passed
			shaderResources[currToken] = { currType, currSlotIndex++ };
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
		};

		auto result = shaderResourcesMap.find(token);

		return (result == shaderResourcesMap.cend()) ? E_SHADER_RESOURCE_TYPE::SRT_UNKNOWN : result->second;
	}
	

	TDE2_API IShaderCompiler* CreateD3D11ShaderCompiler(IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		CD3D11ShaderCompiler* pShaderCompilerInstance = new (std::nothrow) CD3D11ShaderCompiler();

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

#endif