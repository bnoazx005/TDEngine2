#include "../../include/utils/Utils.h"
#include "../../include/utils/CFileLogger.h"
#include "backward.hpp"
#define STR_UTILS_IMPLEMENTATION
#include "stringUtils.hpp"
#define MEM_TRACKER_IMPLEMENTATION
#include "memTracker.hpp"
#include <algorithm>
#include <cctype>


namespace TDEngine2
{
	template <> TDE2_API F32 SwapBytes<F32>(F32 value)
	{
		return SwapObjectBytes(value);
	}

	template <> TDE2_API F64 SwapBytes<F64>(F64 value)
	{
		return SwapObjectBytes(value);
	}


	TDE2_API U8* SwapObjectBytes(U8* pPtr, U32 size)
	{
		std::reverse(pPtr, pPtr + size);
		return pPtr;
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
		
		const USIZE commentPrefixLength  = commentPrefixStr.length();
		const USIZE commentPostfixLength = commentPostfixStr.length();

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


	template <> TDE2_API U32 ComputeStateDescHash<TBlendStateDesc>(const TBlendStateDesc& object)
	{
		return static_cast<U32>(object.mIsEnabled) << 31 |
				static_cast<U32>(object.mAlphaOpType) << 15 |
				static_cast<U32>(object.mDestAlphaValue) << 12 |
				static_cast<U32>(object.mDestValue) << 9 |
				static_cast<U32>(object.mOpType) << 6 |
				static_cast<U32>(object.mScrAlphaValue) << 3 |
				static_cast<U32>(object.mScrValue);
	}

	template <> TDE2_API U32 ComputeStateDescHash<TDepthStencilStateDesc>(const TDepthStencilStateDesc& object)
	{
		return static_cast<U32>(object.mIsDepthTestEnabled) << 31 |
				static_cast<U32>(object.mIsDepthWritingEnabled) << 30 |
				static_cast<U32>(object.mIsStencilTestEnabled) << 29 |
				static_cast<U32>(object.mDepthCmpFunc) << 26 |
				static_cast<U32>(object.mStencilBackFaceOp.mDepthFailOp) << 23 |
				static_cast<U32>(object.mStencilBackFaceOp.mFailOp) << 21 |
				static_cast<U32>(object.mStencilBackFaceOp.mFunc) << 19 |
				static_cast<U32>(object.mStencilBackFaceOp.mPassOp) << 17 |
				static_cast<U32>(object.mStencilFrontFaceOp.mDepthFailOp) << 15 |
				static_cast<U32>(object.mStencilFrontFaceOp.mFailOp) << 13 |
				static_cast<U32>(object.mStencilFrontFaceOp.mFunc) << 11 |
				static_cast<U32>(object.mStencilFrontFaceOp.mPassOp) << 9 |
				static_cast<U32>(object.mStencilReadMaskValue) << 8 |
				static_cast<U32>(object.mStencilWriteMaskValue);
	}

	template <> TDE2_API U32 ComputeStateDescHash<TTextureSamplerDesc>(const TTextureSamplerDesc& object)
	{
		std::array<C8, sizeof(U32) * 4> data;
		memcpy(&data[0], &object.mFilteringType, sizeof(U32));
		memcpy(&data[4], &object.mUAddressMode, sizeof(U32));
		memcpy(&data[8], &object.mVAddressMode, sizeof(U32));
		memcpy(&data[12], &object.mWAddressMode, sizeof(U32));

		return ComputeHash(&data.front());
	}


	template <> TDE2_API U32 ComputeStateDescHash<TRasterizerStateDesc>(const TRasterizerStateDesc& object)
	{
		return static_cast<U32>(object.mCullMode) |
			(static_cast<U32>(object.mIsDepthClippingEnabled) << 1) |
			(static_cast<U32>(object.mIsFrontCCWEnabled) << 2) |
			(static_cast<U32>(object.mIsScissorTestEnabled) << 3) |
			(static_cast<U32>(object.mIsWireframeModeEnabled) << 4) |
			(static_cast<U16>(object.mDepthBias * 1000.0f) << 18) |
			(static_cast<U16>(object.mMaxDepthBias * 1000.0f) << 5);
	}


	static std::string GetStackTrace() {
		std::ostringstream ss;

		backward::StackTrace stackTrace;
		backward::TraceResolver resolver;
		stackTrace.load_here();
		resolver.load_stacktrace(stackTrace);

		for (std::size_t i = 0; i < stackTrace.size(); ++i) {
			const backward::ResolvedTrace trace = resolver.resolve(stackTrace[i]);

			ss << "#" << i << " at " << trace.object_function << "\n";
		}

		return ss.str();
	}


	TDE2_API void AssertImpl(const C8* message, const C8* file, I32 line)
	{
		if (message)
		{
			LOG_ERROR(Wrench::StringUtils::Format("{0}, {1}:{2}", message, file, line));
		}

		LOG_ERROR(GetStackTrace());
		debug_break();
	}
}