/*!
	\file CD3D11Mappings.h
	\date 03.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <utils/Types.h>
#include "CD3D11Buffer.h"
#include <string>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	/*!
		\brief The static class contains static helper methods, which represents
		mappings from D3D11 types and formates into in-engine used ones
	*/

	class CD3D11Mappings
	{
		public:
			/*!
				\brief The function encodes specified type into a 32 bits value,
				which contains access flags

				\param[in] type A usage type of a buffer

				\return A 32 bits value, which contains access flags
			*/

			TDE2_API static U32 GetAccessFlags(E_BUFFER_USAGE_TYPE type);

			/*!
				\brief The function maps internal buffer usage type into
				D3D11_USAGE type

				\param[in] type A usage type of a buffer

				\return A value of D3D11_USAGE type
			*/

			TDE2_API static D3D11_USAGE GetUsageType(E_BUFFER_USAGE_TYPE type);

			/*!
				\brief The function maps an internal format type into DXGI_FORMAT

				\param[in] format An internal format

				\return A value of DXGI_FORMAT type
			*/

			TDE2_API static DXGI_FORMAT GetDXGIFormat(E_FORMAT_TYPE format);

			/*!
				\brief The function returns a size of a specified format

				\param[in] format An internal format

				\return A size in bytes of a specified format
			*/

			TDE2_API static U32 GetFormatSize(E_FORMAT_TYPE format);

			/*!
				\brief The function returns a string that contains a target version and type of a shader specific
				for D3D11

				\param[in] stageType A type of a shader
				\param[in] version A version of a shader

				\return The function returns a string that contains a target version and type of a shader specific
				for D3D11
			*/

			TDE2_API static std::string GetShaderTargetVerStr(E_SHADER_STAGE_TYPE stageType, E_SHADER_TARGET_VERSION version);
	};
}

#endif