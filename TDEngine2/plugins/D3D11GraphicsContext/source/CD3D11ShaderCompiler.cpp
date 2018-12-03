#include "./../include/CD3D11ShaderCompiler.h"
#include "./../include/CD3D11Mappings.h"
#include <algorithm>
#include <cctype>
#include <vector>
#include <sstream>
#include <iterator>
#include <graphics/IShader.h>
#include <core/IFileSystem.h>
#include <unordered_set>


#if defined (TDE2_USE_WIN32PLATFORM)

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
			return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}
		
		std::string processedSourceCode = _removeComments(source);
		
		CTokenizer tokenizer(processedSourceCode);

		/// parse source code to get a meta information about it
		TShaderMetadata shaderMetadata = _parseShader(tokenizer);

		TD3D11ShaderCompilerOutput* pResult = new TD3D11ShaderCompilerOutput();

		if (_isShaderStageEnabled(SST_VERTEX, shaderMetadata))
		{
			/// try to compile a vertex shader
			TResult<std::vector<U8>> vertexShaderOutput = _compileShaderStage(SST_VERTEX, processedSourceCode, shaderMetadata.mVertexShaderEntrypointName,
				shaderMetadata.mDefines, shaderMetadata.mFeatureLevel);

			if (vertexShaderOutput.HasError())
			{
				return TErrorValue<E_RESULT_CODE>(vertexShaderOutput.GetError());
			}

			pResult->mVSByteCode = std::move(vertexShaderOutput.Get());
		}

		if (_isShaderStageEnabled(SST_PIXEL, shaderMetadata))
		{
			/// try to compile a pixel shader
			TResult<std::vector<U8>> pixelShaderOutput = _compileShaderStage(SST_PIXEL, processedSourceCode, shaderMetadata.mPixelShaderEntrypointName,
				shaderMetadata.mDefines, shaderMetadata.mFeatureLevel);

			if (pixelShaderOutput.HasError())
			{
				return TErrorValue<E_RESULT_CODE>(pixelShaderOutput.GetError());
			}

			pResult->mPSByteCode = std::move(pixelShaderOutput.Get());
		}

		if (_isShaderStageEnabled(SST_GEOMETRY, shaderMetadata))
		{
			/// try to compile a geometry shader
			TResult<std::vector<U8>> geometryShaderOutput = _compileShaderStage(SST_GEOMETRY, processedSourceCode, shaderMetadata.mGeometryShaderEntrypointName,
				shaderMetadata.mDefines, shaderMetadata.mFeatureLevel);

			if (geometryShaderOutput.HasError())
			{
				return TErrorValue<E_RESULT_CODE>(geometryShaderOutput.GetError());
			}

			pResult->mPSByteCode = std::move(geometryShaderOutput.Get());
		}

		pResult->mUniformBuffersInfo = std::move(shaderMetadata.mUniformBuffers);
		
		return TOkValue<TShaderCompilerOutput*>(pResult);
	}

	TResult<std::vector<U8>> CD3D11ShaderCompiler::_compileShaderStage(E_SHADER_STAGE_TYPE shaderStage, const std::string& source, 
																	   const std::string& entryPointName, 
																	   const CD3D11ShaderCompiler::TDefinesMap& shaderDefinesMap,
																	   E_SHADER_FEATURE_LEVEL targetVersion) const
	{
		std::vector<U8> byteCodeArray;

		ID3DBlob* pBytecodeBuffer = nullptr;
		ID3DBlob* pErrorBuffer    = nullptr;

		//enable only needed stage within the source code with a preprocessor
		D3D_SHADER_MACRO defines[] =
		{
			{ "TDE2_HLSL_SHADER", "1" },
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
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		U32 size = pBytecodeBuffer->GetBufferSize();

		U8* pBuffer = static_cast<U8*>(pBytecodeBuffer->GetBufferPointer());

		byteCodeArray.reserve(size);

		std::copy(pBuffer, pBuffer + size, std::back_inserter(byteCodeArray));

		return TOkValue<std::vector<U8>>(byteCodeArray);
	}

	CD3D11ShaderCompiler::TUniformBuffersMap CD3D11ShaderCompiler::_processUniformBuffersDecls(const TStructDeclsMap& structsMap, CTokenizer& tokenizer) const
	{
		TUniformBuffersMap uniformBuffersDecls = CBaseShaderCompiler::_processUniformBuffersDecls(structsMap, tokenizer);

		std::string currToken;
		std::string nextToken;

		std::string bufferName;
				
		std::unordered_set<U8> usedBufferSlots { IUBR_PER_FRAME, IUBR_PER_OBJECT, IUBR_RARE_UDATED, IUBR_CONSTANTS };

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
				uniformBuffersDecls.insert({ bufferName, { getCurrentOrNextFreeBufferSlot(currSlotIndex), _getPaddedStructSize(structsMap, tokenizer) }});
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

	U32 CD3D11ShaderCompiler::_getBuiltinTypeSize(const std::string& type) const
	{
		U32 pos = type.find_first_of("1234");
		
		U32 size = 1;

		std::string baseType = type.substr(0, pos);
		
		U32 currPos = 0;

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