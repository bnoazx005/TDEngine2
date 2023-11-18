/*!
	\file ITexture2D.h
	\date 20.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "ITexture.h"
#include "./../core/IResourceLoader.h"
#include "./../core/IResourceFactory.h"
#include "./../math/TRect.h"
#include <string>
#include <memory>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IFileSystem;


	/*!
		struct TTexture2DParameters

		\brief The stucture contains fields for creation ITexture2D objects
	*/

	typedef struct TTexture2DParameters : TBaseResourceParameters
	{
		TDE2_API TTexture2DParameters() = default;

		/*!
			\brief The main constructor of the struct

			\note TTexture2DParameters's definition is placed in CBaseTexture2D.cpp file
		*/

		TDE2_API TTexture2DParameters(U32 width, U32 height, E_FORMAT_TYPE format, U32 mipLevelsCount = 1, U32 samplesCount = 1, U32 samplingQuality = 0);
		
		/*!
			\brief The method deserializes object's state from given reader

			\param[in, out] pReader An input stream of data that contains information about the object

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

		/*!
			\brief The method serializes object's state into given stream

			\param[in, out] pWriter An output stream of data that writes information about the object

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

		U32                 mWidth;

		U32                 mHeight;

		E_FORMAT_TYPE       mFormat;

		U32                 mNumOfMipLevels;

		U32                 mNumOfSamples;

		U32                 mSamplingQuality;

		TTextureSamplerDesc mTexSamplerDesc;

		bool                mIsWriteable = false; ///< The field is used to make a texture writeable in a compute shader
	} TTexture2DParameters, *TTexture2DParametersPtr;


	/*!
		interface ITexture2D

		\brief The interface describes a common functionality of 2D textures
	*/

	class ITexture2D: public ITexture
	{
		public:
			TDE2_REGISTER_TYPE(ITexture2D);

			/*!
				\brief The method initializes an internal state of a 2d texture. The overloaded version of
				Init method creates a new texture object within memory with given width, height and format

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\param[in] params Additional parameters of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, 
												const TTexture2DParameters& params) = 0;

			/*!
				\brief The method writes data into a specified texture's region

				\param[in] regionRect A region, which will be overwritten

				\param[in] pData Data that will be written into a given region

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE WriteData(const TRectI32& regionRect, const U8* pData) = 0;

			/*!
				\brief The method returns an internal data that the texture stores. (The returned data is allocated
				on heap so should be manually deleted later) For now we use std::unique_ptr instead

				\return A pointer to texture's memory, which size equals to width * height * bytes_per_channel
			*/

			TDE2_API virtual std::vector<U8> GetInternalData() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITexture2D)
	};


	TDE2_DECLARE_SCOPED_PTR(ITexture2D)


	class ITexture2DLoader : public IGenericResourceLoader<IResourceManager*, IGraphicsContext*, IFileSystem*> {};
	class ITexture2DFactory : public IGenericResourceFactory<IResourceManager*, IGraphicsContext*> {};
}
