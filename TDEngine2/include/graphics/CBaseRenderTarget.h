/*!
	\file CBaseRenderTarget.h
	\date 09.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IRenderTarget.h"
#include "../core/CBaseResource.h"


namespace TDEngine2
{
	/*!
		class CBaseRenderTarget

		\brief The class implements platform independent functionality of a render target
	*/

	class CBaseRenderTarget : public virtual IRenderTarget, public CBaseResource
	{
		public:
			TDE2_REGISTER_RESOURCE_TYPE(CBaseRenderTarget)
			TDE2_REGISTER_TYPE(CBaseRenderTarget)

			/*!
				\brief The method initializes an internal state of a render target

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\param[in] params Additional parameters of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, 
										const TTexture2DParameters& params) override;

			/*!
				\brief The method binds a texture object to a given slot

				\param[in] slot Slot's index
			*/

			TDE2_API void Bind(U32 slot) override;

			/*!
				\brief The method allows to resize internal resources that corresponds to the given render target
			*/

			TDE2_API E_RESULT_CODE Resize(U32 width, U32 height) override;

			/*!
				\brief The method sets up wrapping mode for U axis
			*/

			TDE2_API void SetUWrapMode(const E_ADDRESS_MODE_TYPE& mode) override;

			/*!
				\brief The method sets up wrapping mode for Z axis
			*/

			TDE2_API void SetVWrapMode(const E_ADDRESS_MODE_TYPE& mode) override;

			/*!
				\brief The method sets up wrapping mode for W axis
			*/

			TDE2_API void SetWWrapMode(const E_ADDRESS_MODE_TYPE& mode) override;

			/*!
				\brief The method sets up a type of filtering read samples from the texture
			*/

			TDE2_API void SetFilterType(const E_TEXTURE_FILTER_TYPE& type) override;

			/*!
				\brief The method returns a width of a texture

				\return The method returns a width of a texture
			*/

			TDE2_API U32 GetWidth() const override;

			/*!
				\brief The method returns a height of a texture

				\return The method returns a height of a texture
			*/

			TDE2_API U32 GetHeight() const override;

			/*!
				\brief The method returns current format of the texture

				\return The method returns current format of the texture
			*/

			TDE2_API E_FORMAT_TYPE GetFormat() const override;

			/*!
				\return The method returns a rect of the texture in range [0.0f, 1.0f]
			*/

			TDE2_API TRectF32 GetNormalizedTextureRect() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseRenderTarget)

			TDE2_API virtual E_RESULT_CODE _createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
																		 U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality, bool isWriteable) = 0;

			TDE2_API const TPtr<IResourceLoader> _getResourceLoader() override;
		protected:
			IGraphicsContext*   mpGraphicsContext;

			U32                 mWidth;

			U32                 mHeight;

			E_FORMAT_TYPE       mFormat;

			U32                 mNumOfMipLevels;

			U32                 mNumOfSamples;

			U32                 mSamplingQuality;

			TTextureSamplerDesc mTextureSamplerParams;

			TTextureSamplerId   mCurrTextureSamplerHandle = TTextureSamplerId::Invalid;

			bool                mIsRandomlyWriteable = false;
	};


	/*!
		class CBaseDepthBufferTarget

		\brief The class implements base functionlity of bindable depth buffer
	*/

	class CBaseDepthBufferTarget : public CBaseRenderTarget, public IDepthBufferTarget
	{
		public:
			TDE2_REGISTER_TYPE(CBaseDepthBufferTarget)
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseDepthBufferTarget)
	};
}
