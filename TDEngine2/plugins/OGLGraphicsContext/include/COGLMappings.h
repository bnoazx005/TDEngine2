/*!
	\file COGLMappings.h
	\date 06.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <utils/Types.h>
#include <graphics/IBuffer.h>


namespace TDEngine2
{
	/*!
		\brief The static class contains static helper methods, which represents
		mappings from OGL types and formates into in-engine used ones
	*/

	class COGLMappings
	{
		public:
			/*!
				\brief The function maps internal buffer usage type into
				GLenum type

				\param[in] type A usage type of a buffer

				\return A value of Glenum type
			*/

			TDE2_API static GLenum GetUsageType(E_BUFFER_USAGE_TYPE type);

			/*!
				\brief The function converts the internal E_BUFFER_MAP_TYPE into
				corresponding GLenum type of OGL

				\param[in] type A type, which specifies an access type to a buffer

				\return A value of Glenum type
			*/

			TDE2_API static GLenum GetBufferMapAccessType(E_BUFFER_MAP_TYPE type);
	};
}