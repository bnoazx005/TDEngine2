#include "./../../../include/graphics/d3d11/CD3D11Mappings.h"


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	U32 CD3D11Mappings::GetAccessFlags(E_BUFFER_USAGE_TYPE type)
	{
		U32 accessFlags = 0x0;

		switch (type)
		{
			case BUT_DYNAMIC:
				accessFlags |= D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
				break;
		}

		return accessFlags;
	}
	
	D3D11_USAGE CD3D11Mappings::GetUsageType(E_BUFFER_USAGE_TYPE type)
	{
		switch (type)
		{
			case BUT_DYNAMIC:
				return D3D11_USAGE_DYNAMIC;
			case BUT_STATIC:
				return D3D11_USAGE_IMMUTABLE;
		}

		return D3D11_USAGE_DEFAULT;
	}
}

#endif