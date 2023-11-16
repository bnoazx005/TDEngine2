/*!
	\file ITexture.h
	\date 20.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/IBaseObject.h"
#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../math/TRect.h"


namespace TDEngine2
{
	/*!
		interface ITexture

		\brief The interface describes a common functionality that
		all texture types should provide within the engine
	*/

	class ITexture: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method binds a texture object to a given slot

				\param[in] slot Slot's index
			*/

			TDE2_API virtual void Bind(U32 slot) = 0;

			/*!
				\brief The method sets up wrapping mode for U axis
			*/

			TDE2_API virtual void SetUWrapMode(const E_ADDRESS_MODE_TYPE& mode) = 0;
			
			/*!
				\brief The method sets up wrapping mode for Z axis
			*/
			
			TDE2_API virtual void SetVWrapMode(const E_ADDRESS_MODE_TYPE& mode) = 0;
			
			/*!
				\brief The method sets up wrapping mode for W axis
			*/

			TDE2_API virtual void SetWWrapMode(const E_ADDRESS_MODE_TYPE& mode) = 0;

			/*!
				\brief The method sets up a type of filtering read samples from the texture
			*/

			TDE2_API virtual void SetFilterType(const E_TEXTURE_FILTER_TYPE& type) = 0;

			/*!
				\brief The method returns a width of a texture

				\return The method returns a width of a texture
			*/

			TDE2_API virtual U32 GetWidth() const = 0;

			/*!
				\brief The method returns a height of a texture

				\return The method returns a height of a texture
			*/

			TDE2_API virtual U32 GetHeight() const = 0;

			/*!
				\brief The method returns current format of the texture

				\return The method returns current format of the texture
			*/

			TDE2_API virtual E_FORMAT_TYPE GetFormat() const = 0;

			/*!
				\return The method returns a rect of the texture in range [0.0f, 1.0f]
			*/

			TDE2_API virtual TRectF32 GetNormalizedTextureRect() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITexture)
	};


	class IGraphicsContext;


	enum class E_TEXTURE_IMPL_TYPE: U32
	{
		TEXTURE_2D,
		TEXTURE_2D_ARRAY,
		TEXTURE_3D,
		CUBEMAP,
	};


	enum class E_TEXTURE_IMPL_USAGE_TYPE : U8
	{
		STAGING,
		DYNAMIC,
		STATIC,
	};


	TDE2_DECLARE_BITMASK_OPERATORS_INTERNAL(E_BIND_GRAPHICS_TYPE);


	typedef struct TInitTextureImplParams
	{
		E_TEXTURE_IMPL_TYPE       mType = E_TEXTURE_IMPL_TYPE::TEXTURE_2D;
		E_TEXTURE_IMPL_USAGE_TYPE mUsageType = E_TEXTURE_IMPL_USAGE_TYPE::STATIC;

		U32                       mWidth  = 2;
		U32                       mHeight = 2;
		U32                       mDepth  = 1;

		E_FORMAT_TYPE             mFormat = E_FORMAT_TYPE::FT_NORM_UBYTE4;
		E_BIND_GRAPHICS_TYPE      mBindFlags = E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE;

		U32                       mNumOfMipLevels = 1;
		U32                       mArraySize = 1;

		U32                       mNumOfSamples = 1;
		U32                       mSamplingQuality = 0;

		TTextureSamplerDesc       mTexSamplerDesc;

		bool                      mIsWriteable = false; ///< The field is used to make a texture writeable in a compute shader

		std::string               mName;
	} TInitTextureParams, *TInitTextureParamsPtr;


	/*!
		interface ITextureImpl

		\brief The interface provides functionality of low-level texture concept. The difference between ITexture and ITextureImpl
		is that the first is a high-level type that presents textures as loadable game resource where ITextureImpl is abstraction around
		GAPI calls.

		There are a few interfaces for cubemaps/atlases/3D textures. ITextureImpl provides access to all functionality for all that types.
	*/

	class ITextureImpl: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an initial state of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params) = 0;

			/*!
				\brief The method allows to resize internal resources that corresponds to the given texture
			*/

			TDE2_API virtual E_RESULT_CODE Resize(U32 width, U32 height, U32 depth = 1) = 0;

			TDE2_API virtual E_RESULT_CODE SetSamplerDesc(const TTextureSamplerDesc& samplerDesc) = 0;

			TDE2_API virtual void* GetInternalHandle() = 0;

			TDE2_API virtual const TInitTextureParams& GetParams() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITextureImpl)
	};
}
