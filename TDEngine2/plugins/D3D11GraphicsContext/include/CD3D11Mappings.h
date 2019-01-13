/*!
	\file CD3D11Mappings.h
	\date 03.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <utils/Types.h>
#include "CD3D11Buffer.h"
#include <core/IGraphicsContext.h>
#include <graphics/IIndexBuffer.h>
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

			TDE2_API static std::string GetShaderTargetVerStr(E_SHADER_STAGE_TYPE stageType, E_SHADER_FEATURE_LEVEL version);

			/*!
				\brief The function returns a number of channels for a given format type

				\param[in] format A specific format of a texture

				\return The function returns a number of channels for a given format type
			*/

			TDE2_API static U32 GetNumOfChannelsOfFormat(E_FORMAT_TYPE format);

			/*!
				\brief The function returns a name of a given semantic type

				\param[in] semanticType A semantic type of a single vertex

				\return The function returns a name of a given semantic type
			*/

			TDE2_API static const C8* GetSemanticTypeName(E_VERTEX_ELEMENT_SEMANTIC_TYPE semanticType);

			/*!
				\brief The function maps internal engine topology's type into Direct3D11 specific value

				\param[in] topologyType An internal engine representation of a primitive topology

				\return A primitive topology's value which is D3D11 specific
			*/

			TDE2_API static D3D11_PRIMITIVE_TOPOLOGY GetPrimitiveTopology(E_PRIMITIVE_TOPOLOGY_TYPE topologyType);
			
			/*!
				\brief The function maps internal index format into Direct3D11 specific type

				\param[in] indexFormatType An internal type that represents a format of an index

				\return A Direct3D11 format of an index
			*/

			TDE2_API static DXGI_FORMAT GetIndexFormat(E_INDEX_FORMAT_TYPE indexFormatType);

			/*!
				\brief The function maps an internal filter's value into Direct3D11 specific one

				\param[in] filterValue An internal value which represents a filter's type

				\return A D3D11 specific filter's type
			*/

			TDE2_API static D3D11_FILTER GetFilterType(U32 filterValue);

			/*!
				\brief The function maps an internal address mode's type into Direct3D11 specific one

				\param[in] addressMode An internal type which represents an addressing mode

				\return A value of D3D11_TEXTURE_ADDRESS_MODE type which is equivalent of a given one
			*/

			TDE2_API static D3D11_TEXTURE_ADDRESS_MODE GetTextureAddressMode(E_ADDRESS_MODE_TYPE addressMode);
	};
}

#endif