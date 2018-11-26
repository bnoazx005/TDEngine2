/*!
	\file COGLMappings.h
	\date 06.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <utils/Types.h>
#include <graphics/IBuffer.h>
#include <GL/glew.h>


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

			/*!
				\brief The function maps an internal format type into OpenGL internal format

				\param[in] format An internal format

				\return A value of internal GL format
			*/

			TDE2_API static GLint GetInternalFormat(E_FORMAT_TYPE format);

			/*!
				\brief The function maps an internal format type into pixel data's format

				\param[in] format An internal format

				\return A value of pixel data's format
			*/

			TDE2_API static GLenum GetPixelDataFormat(E_FORMAT_TYPE format);

			/*!
				\brief The function maps an internal shader stage's type into GL defined one

				\param[in] shaderStageType An internal type which represents a shader stage type

				\return GL type which describes a shader stage's type
			*/

			TDE2_API static GLenum GetShaderStageType(E_SHADER_STAGE_TYPE shaderStageType);
			
			/*!
				\brief The function returns a number of channels for a given format type

				\param[in] format A specific format of a texture

				\return The functiona returns a number of channels for a given format type
			*/

			TDE2_API static U32 GetNumOfChannelsOfFormat(E_FORMAT_TYPE format);

			/*!
				\brief The function returns a base type, which lies in format's specification

				\param[in] format A specific format of a texture

				\return The function returns a base type, which lies in format's specification
			*/

			TDE2_API static GLenum GetBaseTypeOfFormat(E_FORMAT_TYPE format);

			/*!
				\brief The function returns a size of a specified format

				\param[in] format An internal format

				\return A size in bytes of a specified format
			*/

			TDE2_API static GLsizei GetFormatSize(E_FORMAT_TYPE format);

			/*!
				\brief The function is a predicate that returns true if a given format is normalized,
				false in other cases

				\param[in] format An internal format

				\return The function is a predicate that returns true if a given format is normalized,
				false in other cases
			*/

			TDE2_API static bool IsFormatNormalized(E_FORMAT_TYPE format);
	};
}