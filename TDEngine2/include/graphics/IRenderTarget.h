/*!
	\file IRenderTarget.h
	\date 09.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "ITexture.h"
#include "./../core/IResourceLoader.h"
#include "./../core/IResourceFactory.h"
#include <string>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IFileSystem;


	/*!
		interface IRenderTarget

		\brief The interface describes a common functionality of a render target (frame buffer)
	*/

	class IRenderTarget : public ITexture
	{
	public:
		/*!
			\brief The method initializes an internal state of a render target

			\param[in, out] pResourceManager A pointer to IResourceManager's implementation

			\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

			\param[in] name A resource's name

			\param[in] id An identifier of a resource

			\param[in] width Texture's width

			\param[in] height Texture's height

			\param[in] format Texture's format

			\param[in] mipLevelsCount An amount of levels, which represents a texture with different quality

			\param[in] samplesCount A number of multisamples per pixel

			\param[in] samplingQuality An image quality level

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, TResourceId id,
											U32 width, U32 height, E_FORMAT_TYPE format, U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality) = 0;
	protected:
		DECLARE_INTERFACE_PROTECTED_MEMBERS(IRenderTarget)
	};


	/*!
		struct TRenderTargetParameters

		\brief The stucture contains fields for creation IRenderTarget objects
	*/

	typedef struct TRenderTargetParameters : TBaseResourceParameters
	{
		IGraphicsContext* mpGraphicsContext;

		U32               mWidth;

		U32               mHeight;

		E_FORMAT_TYPE     mFormat;

		U32               mNumOfMipLevels;

		U32               mNumOfSamples;

		U32               mSamplingQuality;
	} TRenderTargetParameters, *TRenderTargetParametersPtr;


	/*!
		interface IRenderTargetFactory

		\brief The interface describes a functionality of a render target factory
	*/

	class IRenderTargetFactory : public IResourceFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IRenderTargetFactory)
	};
}
