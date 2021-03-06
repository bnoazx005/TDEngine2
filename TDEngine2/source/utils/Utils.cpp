#include "./../../include/utils/Utils.h"
#include <algorithm>
#include <cctype>


namespace TDEngine2
{
	TDE2_API E_ENDIAN_TYPE GetHostEndianType()
	{
		U32 checker = 0x1;

		return (*((U8*)&checker) == 0x1) ? E_ENDIAN_TYPE::ET_LITTLE_ENDIAN : E_ENDIAN_TYPE::ET_BIG_ENDIAN;
	}

	TDE2_API U16 Swap2Bytes(U16 value)
	{
		return (value & 0x00FF) << 8 | (value & 0xFF00) >> 8;
	}

	TDE2_API U32 Swap4Bytes(U32 value)
	{
		return (value & 0x000000FF) << 24 | (value & 0x0000FF00) << 8 | (value & 0x00FF0000) >> 8 | (value & 0xFF000000) >> 24;
	}

	TDE2_API U64 Swap8Bytes(U64 value)
	{
		return (value & 0x00000000000000FF) << 56 |
			   (value & 0x000000000000FF00) << 40 | 
			   (value & 0x0000000000FF0000) << 24 | 
			   (value & 0x00000000FF000000) << 8  | 
			   (value & 0x000000FF00000000) >> 8  | 
			   (value & 0x0000FF0000000000) >> 24 |
			   (value & 0x00FF000000000000) >> 40 |
			   (value & 0xFF00000000000000) >> 56;
	}


	std::string CStringUtils::RemoveSingleLineComments(const std::string& source, const std::string& commentPrefixStr)
	{
		if (source.empty() || commentPrefixStr.empty())
		{
			return source;
		}

		std::string::size_type startPos = 0;
		std::string::size_type endPos   = 0;

		std::string processedStr { source };

		while ((startPos = processedStr.find(commentPrefixStr)) != std::string::npos)
		{
			endPos = processedStr.find_first_of("\n\r", startPos);

			if (endPos == std::string::npos)
			{
				return source.substr(0, startPos); /// assume that the rest part is a comment
			}

			processedStr = processedStr.substr(0, startPos) + processedStr.substr(endPos + 1, processedStr.length() - endPos);
		}

		return processedStr;
	}

	std::string CStringUtils::RemoveMultiLineComments(const std::string& source, const std::string& commentPrefixStr,
										const std::string& commentPostfixStr)
	{
		if (source.empty() || commentPrefixStr.empty() || commentPostfixStr.empty())
		{
			return source;
		}

		std::string::size_type firstPos  = 0;
		std::string::size_type secondPos = 0;
		std::string::size_type thirdPos  = 0;
		
		const U32 commentPrefixLength  = commentPrefixStr.length();
		const U32 commentPostfixLength = commentPostfixStr.length();

		U8 numOfNestedCommentsBlocks = 0;

		std::string::size_type seekPos;
		
		std::string processedStr { source };

		while (((firstPos = processedStr.find(commentPrefixStr)) != std::string::npos))
		{
			++numOfNestedCommentsBlocks;

			seekPos = firstPos + commentPrefixLength;

			do
			{
				secondPos = processedStr.find(commentPostfixStr, seekPos);
				thirdPos = processedStr.find(commentPrefixStr, seekPos);

				if ((secondPos == thirdPos) && (secondPos == std::string::npos))
				{
					break;
				}

				if (secondPos < thirdPos)
				{
					--numOfNestedCommentsBlocks;

					seekPos = secondPos + commentPostfixLength;

					continue;
				}

				if (thirdPos < secondPos)
				{
					++numOfNestedCommentsBlocks;

					seekPos = thirdPos + commentPrefixLength;

					continue;
				}
			} while (seekPos < processedStr.length() && (numOfNestedCommentsBlocks > 0));

			if (numOfNestedCommentsBlocks == 0)
			{
				processedStr = processedStr.substr(0, firstPos) + processedStr.substr(secondPos + commentPostfixLength, processedStr.length() - secondPos - commentPostfixLength);
			}
			else
			{
				processedStr = processedStr.substr(0, firstPos);
			}
		}

		return processedStr;
	}

	std::string CStringUtils::RemoveExtraWhitespaces(const std::string& str)
	{
		bool isPrevChSpace = false;

		std::string processedStr { str };

		processedStr.erase(std::remove_if(processedStr.begin(), processedStr.end(), [&isPrevChSpace](C8 ch)
		{
			bool isCurrChSpace = std::isspace(ch);

			bool result = isCurrChSpace && isPrevChSpace;

			isPrevChSpace = isCurrChSpace;

			return result;
		}), processedStr.end());

		return processedStr;
	}

	std::string CStringUtils::RemoveWhitespaces(const std::string& str)
	{
		std::string processedStr{ str };

		processedStr.erase(std::remove_if(processedStr.begin(), processedStr.end(), [](C8 ch)
		{
			return std::isspace(ch);
		}), processedStr.end());

		return processedStr;
	}
	
	std::vector<std::string> CStringUtils::Split(const std::string& str, const std::string& delims)
	{
		std::string::size_type pos     = 0;
		std::string::size_type currPos = 0;

		std::string currToken;

		std::vector<std::string> tokens;

		while ((currPos < str.length()) && ((pos = str.find_first_of(delims, currPos)) != std::string::npos))
		{
			currToken = std::move(str.substr(currPos, pos - currPos));

			if (!currToken.empty())
			{
				tokens.emplace_back(std::move(currToken));
			}
			
			currPos = pos + 1;
		}

		/// insert last token if it wasn't pushed back before
		if (currPos != str.length())
		{
			tokens.emplace_back(std::move(str.substr(currPos, str.length() - currPos)));
		}

		return std::move(tokens);
	}

	std::string CStringUtils::GetEmptyStr()
	{
		return mEmptyStr;
	}

	bool CStringUtils::StartsWith(const std::string& str, const std::string& prefix)
	{
		return str.rfind(prefix, 0) == 0;
	}

	const std::string CStringUtils::mEmptyStr {};


	TDE2_API std::string GraphicsContextTypeToString(E_GRAPHICS_CONTEXT_GAPI_TYPE graphicsContextType)
	{
		switch (graphicsContextType)
		{
			case GCGT_DIRECT3D11:
				return "d3d11";
			case GCGT_OPENGL3X:
				return "gl3x";
		}

		return "unknown";
	}

	TDE2_API E_GRAPHICS_CONTEXT_GAPI_TYPE StringToGraphicsContextType(const std::string& value)
	{
		if (value.find("d3d11") != std::string::npos)
		{
			return GCGT_DIRECT3D11;
		}
		else if (value.find("gl3x") != std::string::npos)
		{
			return GCGT_OPENGL3X;
		}

		return GCGT_UNKNOWN;
	}


	/*!
		\brief CFormatUtils's definition
	*/

	U8 CFormatUtils::GetNumOfChannelsOfFormat(E_FORMAT_TYPE format)
	{
		switch (format)
		{
			case FT_BYTE1:
			case FT_NORM_BYTE1:
			case FT_UBYTE1:
			case FT_NORM_UBYTE1:
			case FT_FLOAT1:
			case FT_D32:
			case FT_SHORT1:
			case FT_NORM_SHORT1:
			case FT_USHORT1:
			case FT_NORM_USHORT1:
			case FT_UINT1:
			case FT_NORM_UINT1:
			case FT_SINT1:
			case FT_NORM_SINT1:
				return 1;
			case FT_BYTE2:
			case FT_NORM_BYTE2:
			case FT_UBYTE2:
			case FT_NORM_UBYTE2:
			case FT_FLOAT2:
			case FT_FLOAT2_TYPELESS:
			case FT_SHORT2:
			case FT_NORM_SHORT2:
			case FT_USHORT2:
			case FT_NORM_USHORT2:
			case FT_UINT2:
			case FT_NORM_UINT2:
			case FT_SINT2:
			case FT_NORM_SINT2:
				return 2;
			case FT_BYTE3:
			case FT_NORM_BYTE3:
			case FT_UBYTE3:
			case FT_NORM_UBYTE3:
			case FT_FLOAT3:
			case FT_FLOAT3_TYPELESS:
			case FT_SHORT3:
			case FT_NORM_SHORT3:
			case FT_USHORT3:
			case FT_NORM_USHORT3:
			case FT_UINT3:
			case FT_NORM_UINT3:
			case FT_SINT3:
			case FT_NORM_SINT3:
				return 3;
			case FT_BYTE4:
			case FT_NORM_BYTE4:
			case FT_UBYTE4:
			case FT_NORM_UBYTE4:
			case FT_NORM_BYTE4_SRGB:
			case FT_UBYTE4_BGRA_UNORM:
			case FT_SHORT4:
			case FT_NORM_SHORT4:
			case FT_SINT4:
			case FT_NORM_SINT4:
			case FT_UINT4:
			case FT_NORM_UINT4:
			case FT_USHORT4:
			case FT_NORM_USHORT4:
			case FT_FLOAT4:
			case FT_FLOAT4_TYPELESS:
				return 4;
		}

		return 0;
	}
	
	U32 CFormatUtils::GetFormatSize(E_FORMAT_TYPE format)
	{
		switch (format)
		{
			case FT_BYTE1:
			case FT_NORM_BYTE1:
				return sizeof(char);
			case FT_BYTE2:
			case FT_NORM_BYTE2:
				return 2 * sizeof(char);
			case FT_BYTE3:
			case FT_NORM_BYTE3:
				return 3 * sizeof(char);
			case FT_BYTE4:
			case FT_NORM_BYTE4:
				return 4 * sizeof(char);
			case FT_UBYTE1:
			case FT_NORM_UBYTE1:
				return sizeof(unsigned char);
			case FT_UBYTE2:
			case FT_NORM_UBYTE2:
				return 2 * sizeof(unsigned char);
			case FT_UBYTE3:
			case FT_NORM_UBYTE3:
				return 3 * sizeof(unsigned char);
			case FT_UBYTE4:
			case FT_NORM_UBYTE4:
			case FT_NORM_BYTE4_SRGB:
			case FT_UBYTE4_BGRA_UNORM:
				return 4 * sizeof(unsigned char);
			case FT_FLOAT1:
			case FT_D32:
				return sizeof(float);
			case FT_FLOAT2:
			case FT_FLOAT2_TYPELESS:
				return 2 * sizeof(float);
			case FT_FLOAT3:
			case FT_FLOAT3_TYPELESS:
				return 3 * sizeof(float);
			case FT_FLOAT4:
			case FT_FLOAT4_TYPELESS:
				return 4 * sizeof(float);
			case FT_SHORT1:
			case FT_NORM_SHORT1:
				return 1 * sizeof(short);
			case FT_SHORT2:
			case FT_NORM_SHORT2:
				return 2 * sizeof(short);
			case FT_SHORT3:
			case FT_NORM_SHORT3:
				return 3 * sizeof(short);
			case FT_SHORT4:
			case FT_NORM_SHORT4:
				return 4 * sizeof(short);
			case FT_USHORT1:
			case FT_NORM_USHORT1:
				return 1 * sizeof(short);
			case FT_USHORT2:
			case FT_NORM_USHORT2:
				return 2 * sizeof(short);
			case FT_USHORT3:
			case FT_NORM_USHORT3:
				return 3 * sizeof(short);
			case FT_USHORT4:
			case FT_NORM_USHORT4:
				return 4 * sizeof(short);
			case FT_UINT1:
			case FT_NORM_UINT1:
				return 1 * sizeof(int);
			case FT_UINT2:
			case FT_NORM_UINT2:
				return 2 * sizeof(int);
			case FT_UINT3:
			case FT_NORM_UINT3:
				return 3 * sizeof(int);
			case FT_UINT4:
			case FT_NORM_UINT4:
				return 4 * sizeof(int);
			case FT_SINT1:
			case FT_NORM_SINT1:
				return 1 * sizeof(int);
			case FT_SINT2:
			case FT_NORM_SINT2:
				return 2 * sizeof(int);
			case FT_SINT3:
			case FT_NORM_SINT3:
				return 3 * sizeof(int);
			case FT_SINT4:
			case FT_NORM_SINT4:
				return 4 * sizeof(int);
		}

		return 0;
	}

	E_FORMAT_TYPE CFormatUtils::GetFormatFromString(const std::string& str)
	{
		static std::unordered_map<std::string, E_FORMAT_TYPE> formatsMap
		{
			{ "R8_UNORM" , FT_NORM_UBYTE1 },
			{ "R8G8_UNORM" , FT_NORM_UBYTE2 },
			{ "R8G8B8_UNORM" , FT_NORM_UBYTE3 },
			{ "R8G8B8A8_UNORM" , FT_NORM_UBYTE4 },
		};

		if (formatsMap.find(str) == formatsMap.cend())
		{
			/// \todo not all values of E_FORMAT_TYPE are represented here

			TDE2_UNIMPLEMENTED();

			return FT_UNKNOWN;
		}

		return formatsMap.at(str);
	}


	/*!
		\brief CDeferOperation's definition
	*/

	CDeferOperation::CDeferOperation(const TCallbackType& callback) :
		mCallback(callback)
	{
	}

	CDeferOperation::~CDeferOperation()
	{
		mCallback();
	}


	std::string EngineSubsystemTypeToString(E_ENGINE_SUBSYSTEM_TYPE type)
	{
		switch (type)
		{
			case EST_WINDOW:
				return "Window system";
			case EST_GRAPHICS_CONTEXT:
				return "Graphics Context";
			case EST_FILE_SYSTEM:
				return "File Manager";
			case EST_RESOURCE_MANAGER:
				return "Resource Manager";
			case EST_JOB_MANAGER:
				return "Job Manager";
			case EST_PLUGIN_MANAGER:
				return "Plugin Manager";
			case EST_EVENT_MANAGER:
				return "Events Manager";
			case EST_MEMORY_MANAGER:
				return "Memory Manager";
			case EST_RENDERER:
				return "Renderer system";
			case EST_INPUT_CONTEXT:
				return "Input Context";
		}
		
		return "";
	}
}