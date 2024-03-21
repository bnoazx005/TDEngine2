/*!
	\file COGLMappings.h
	\date 06.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <utils/Types.h>
#include <graphics/IBuffer.h>
#include <core/IGraphicsContext.h>
#include <graphics/ICubemapTexture.h>
#include <GL/glew.h>
#include <string>


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
				\brief The function maps an internal format type into pixel data's format

				\param[in] format An internal format

				\return A value of pixel data's format
			*/

			TDE2_API static GLenum GetWritableTextureFormat(E_FORMAT_TYPE format);

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
				\brief The method returns a size of a type that can be retrieved from glGetActiveUniform
			*/

			TDE2_API static GLint GetTypeSize(GLenum type);

			/*!
				\brief The function is a predicate that returns true if a given format is normalized,
				false in other cases

				\param[in] format An internal format

				\return The function is a predicate that returns true if a given format is normalized,
				false in other cases
			*/

			TDE2_API static bool IsFormatNormalized(E_FORMAT_TYPE format);

			/*!
				\brief The function maps internal engine topology's type into OpenGL specific value

				\param[in] topologyType An internal engine representation of a primitive topology
				
				\return A primitive topology's value which is OpenGL specific
			*/

			TDE2_API static GLenum GetPrimitiveTopology(E_PRIMITIVE_TOPOLOGY_TYPE topologyType);

			/*!
				\brief The function maps internal index format into OpenGL specific type

				\param[in] indexFormatType An internal type that represents a format of an index

				\return An OpenGL format of an index
			*/

			TDE2_API static GLenum GetIndexFormat(E_INDEX_FORMAT_TYPE indexFormatType);

			/*!
				\brief The function maps an internal minification filter's value into OpenGL specific one

				\param[in] filterValue An internal value which represents a filter's type
				\param[in] useMipMaps The flag determines whether mip-mapping is used or when filtering happens

				\return A OpenGL specific filter's type
			*/

			TDE2_API static GLint GetMinFilterType(E_TEXTURE_FILTER_TYPE filterValue, bool useMipMaps = true);

			/*!
				\brief The function maps an internal magnification filter's value into OpenGL specific one

				\param[in] filterValue An internal value which represents a filter's type

				\return A OpenGL specific filter's type
			*/

			TDE2_API static GLint GetMagFilterType(E_TEXTURE_FILTER_TYPE filterValue);

			/*!
				\brief The function maps an internal address mode's type into OpenGL specific one

				\param[in] addressMode An internal type which represents an addressing mode

				\return A value of OpenGL type which is equivalent of a given one
			*/

			TDE2_API static GLint GetTextureAddressMode(E_ADDRESS_MODE_TYPE addressMode);

			/*!
				\brief The function maps GLenum error code into internal E_RESULT_CODE type

				\param[in] error An error code of GLenum type

				\return An error code of E_RESULT_CODE type which corresponds to a given one
			*/

			TDE2_API static E_RESULT_CODE GetErrorCode(GLenum error);

			/*!
				\brief The function converts internal OpenGL error's value into string representation

				\param[in] error An error code of GLenum type

				\return A string representation of OGL error
			*/

			TDE2_API static std::string ErrorCodeToString(GLenum error);

			/*!
				\brief The function maps engine's value into internal OpenGL's value which defines a value
				of a factor within the blending equation

				\param[in] factor A factor's value which is the engine's specific one

				\return An internal OpenGL's value which is equivalent for a given one
			*/

			TDE2_API static GLenum GetBlendFactorValue(E_BLEND_FACTOR_VALUE factor);

			/*!
				\brief The function maps engine's value into internal OpenGL's value which defines 
				a type of blending operation

				\param[in] factor A type's value which is the engine's specific one

				\return An internal OpenGL's value which is equivalent for a given one
			*/

			TDE2_API static GLenum GetBlendOpType(E_BLEND_OP_TYPE opType);
			
			/*!
				\brief The function maps internal cubemap's face index into OpenGL specific one

				\param[in] faceType An index of cubemap's face
				
				\return An internal OpenGL's value which is equivalent for a given one
			*/

			TDE2_API static GLenum GetCubemapFace(E_CUBEMAP_FACE faceType);

			TDE2_API static GLenum GetTextureType(E_TEXTURE_IMPL_TYPE implType);

			/*!
				\brief The function maps internal comparison function's type into OGL specific one

				\param[in] func An internal type of a comparison function

				\return OGL comparison function's type based on the given argument
			*/

			TDE2_API static GLenum GetComparisonFunc(const E_COMPARISON_FUNC& func);

			/*!
				\brief The function maps internal stencil operation's type into OGL specific one

				\param[in] stencilOp An internal type of a stencil operation

				\return OGL stencil operation's type based on the given argument
			*/

			TDE2_API static GLenum GetStencilOpType(const E_STENCIL_OP& stencilOp);

			/*!
				\brief The method maps E_CULL_MODE into Glenum's value

				\param[in] cullMode An internal enumeration's value

				\return OpenGL 3.x specific value which represents a culling mode
			*/

			TDE2_API static GLenum GetCullMode(const E_CULL_MODE& cullMode);
	};
}