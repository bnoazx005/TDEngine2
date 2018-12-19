#include "./../../include/utils/Utils.h"


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
}