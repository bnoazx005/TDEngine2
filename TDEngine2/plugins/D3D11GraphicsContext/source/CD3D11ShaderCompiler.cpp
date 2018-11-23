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

		/// parse source code to get a meta information about it
		TShaderMetadata shaderMetadata = _parseShader(processedSourceCode);

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

	CD3D11ShaderCompiler::TUniformBuffersMap CD3D11ShaderCompiler::_processUniformBuffersDecls(const std::string& sourceCode) const
	{
		TUniformBuffersMap uniformBuffersDecls;

		U32 firstPos  = 0;
		U32 secondPos = 0;
		U32 seekPos   = 0;

		U8 currSlotIndex = IUBR_LAST_USED_SLOT + 1; // b0 - b3 for internal usage only

		std::string currLine;
		std::string currBufferName;

		/// extract different shader regions bounds (vertex, pixel, geometry)	
		U32 vertexShaderRegionStartPos   = sourceCode.find(std::string("#if ").append(_getShaderStageDefineName(SST_VERTEX)));
		U32 pixelShaderRegionStartPos    = sourceCode.find(std::string("#if ").append(_getShaderStageDefineName(SST_PIXEL)));
		U32 geometryShaderRegionStartPos = sourceCode.find(std::string("#if ").append(_getShaderStageDefineName(SST_GEOMETRY)));

		std::tuple<U32, U32> vertexShaderRegion   = std::make_tuple(vertexShaderRegionStartPos, sourceCode.find("#endif", vertexShaderRegionStartPos));
		std::tuple<U32, U32> pixelShaderRegion    = std::make_tuple(pixelShaderRegionStartPos, sourceCode.find("#endif", pixelShaderRegionStartPos));
		std::tuple<U32, U32> geometryShaderRegion = std::make_tuple(geometryShaderRegionStartPos, sourceCode.find("#endif", geometryShaderRegionStartPos));

		auto getShaderType = [&vertexShaderRegion, &pixelShaderRegion, &geometryShaderRegion](U32 bufferPos) -> U8
		{
			if (bufferPos > std::get<0>(vertexShaderRegion) && bufferPos < std::get<1>(vertexShaderRegion))
			{
				return SST_VERTEX;
			}

			if (bufferPos > std::get<0>(pixelShaderRegion) && bufferPos < std::get<1>(pixelShaderRegion))
			{
				return SST_PIXEL;
			}

			if (bufferPos > std::get<0>(geometryShaderRegion) && bufferPos < std::get<1>(geometryShaderRegion))
			{
				return SST_GEOMETRY;
			}

			return SST_GEOMETRY + 1;
		};
		
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

		U8 currShaderRegion = 0;

		while ((firstPos = sourceCode.find("cbuffer", seekPos)) != std::string::npos)
		{
			seekPos = firstPos + 7; // 7 is a length of "cbuffer" keyword

			secondPos = sourceCode.find_first_of("{\n", firstPos);

			if (secondPos == std::string::npos)
			{

				continue;
			}

			currLine = sourceCode.substr(firstPos + 7, secondPos - firstPos - 7); // 7 is a length of "cbuffer" keyword

			currLine.erase(std::remove_if(currLine.begin(), currLine.end(), [](U8 ch) { return std::isspace(ch); }), currLine.end()); // remove all whitespaces

			// find delimiter :
			firstPos = currLine.find_first_of(':');

			currBufferName = currLine.substr(0, firstPos);

			currShaderRegion = getShaderType(firstPos);

			if (firstPos == std::string::npos) // cbufer's declaration contains name only
			{
				currSlotIndex = 0; // if a user didn't specify particular register just find first free slot
			}
			else
			{
				firstPos  = currLine.find("(b");
				secondPos = currLine.find_first_of(",)", firstPos);

				currSlotIndex = std::stoi(currLine.substr(firstPos + 2, secondPos - firstPos - 2));
			}

			currSlotIndex = getCurrentOrNextFreeBufferSlot(currSlotIndex); // if slot is already in use find another free

			uniformBuffersDecls[currShaderRegion][currBufferName] = currSlotIndex;
		}

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