#include "./../include/COGLMappings.h"


namespace TDEngine2
{
	GLenum COGLMappings::GetUsageType(E_BUFFER_USAGE_TYPE type)
	{
		if (type == BUT_DYNAMIC)
		{
			return GL_DYNAMIC_DRAW;
		}

		return GL_STATIC_DRAW;
	}

	GLenum COGLMappings::GetBufferMapAccessType(E_BUFFER_MAP_TYPE type)
	{
		switch (type)
		{
			case BMT_READ:
				return GL_READ_ONLY;
			case BMT_READ_WRITE:
				return GL_READ_WRITE;
			case BMT_WRITE:
				return GL_WRITE_ONLY;
		}

		return GL_WRITE_ONLY;
	}
}