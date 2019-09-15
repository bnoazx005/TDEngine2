/*!
	\file CBaseRenderTarget.h
	\date 09.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IRenderTarget.h"
#include "./../core/CBaseResource.h"


namespace TDEngine2
{
	/*!
		class CBaseRenderTarget

		\brief The class implements platform independent functionality of a render target
	*/

	class CBaseRenderTarget : public IRenderTarget, public CBaseResource
	{
		public:
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
										const TRenderTargetParameters& params) override;

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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseRenderTarget)

			TDE2_API virtual E_RESULT_CODE _createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
																		 U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality) = 0;
		protected:
			IGraphicsContext * mpGraphicsContext;

			U32                mWidth;

			U32                mHeight;

			E_FORMAT_TYPE      mFormat;

			U32                mNumOfMipLevels;

			U32                mNumOfSamples;

			U32                mSamplingQuality;
	};
}
