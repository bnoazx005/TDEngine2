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

		U32 startPos = 0;
		U32 endPos   = 0;

		std::string processedStr { source };

		while ((startPos = processedStr.find(commentPrefixStr)) != std::string::npos)
		{
			endPos = processedStr.find_first_of("\n\r", startPos);

			if (endPos == std::string::npos)
			{
				return source.substr(0, startPos); /// assume that the rest part is a comment
			}

			processedStr = processedStr.substr(0, startPos).append(processedStr.substr(endPos + 1, processedStr.length() - endPos));
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

		U32 firstPos  = 0;
		U32 secondPos = 0;
		U32 thirdPos  = 0;
		
		const U32 commentPrefixLength  = commentPrefixStr.length();
		const U32 commentPostfixLength = commentPostfixStr.length();

		U8 numOfNestedCommentsBlocks = 0;

		U32 seekPos;
		
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
				processedStr = processedStr.substr(0, firstPos).append(processedStr.substr(secondPos + commentPostfixLength, processedStr.length() - secondPos - commentPostfixLength));
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

	std::string CStringUtils::GetEmptyStr()
	{
		return mEmptyStr;
	}

	const std::string CStringUtils::mEmptyStr {};
}