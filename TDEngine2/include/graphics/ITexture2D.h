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


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IFileSystem;


	/*!
		interface ITexture2D

		\brief The interface describes a common functionality of 2D textures
	*/

	class ITexture2D: public ITexture
	{
		public:
			/*!
				\brief The method initializes an internal state of a 2d texture. The method 
				is used when we want just to load texture's data from some storage. In this 
				case all the parameters will be executed automatically. To create a new blank
				texture object use overloaded version of Init()
				
				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\param[in] id An identifier of a resource
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, TResourceId id) = 0;

			/*!
				\brief The method initializes an internal state of a 2d texture. The overloaded version of
				Init method creates a new texture object within memory with given width, height and format

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

			/*!
				\brief The method writes data into a specified texture's region

				\param[in] regionRect A region, which will be overwritten

				\param[in] pData Data that will be written into a given region

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE WriteData(const TRectI32& regionRect, const U8* pData) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITexture2D)
	};


	/*!
		interface ITexture2DLoader

		\brief The interface describes a functionality of a 2d texture loader
	*/

	class ITexture2DLoader : public IResourceLoader
	{
		public:
			/*!
				\brief The method initializes an inner state of an object
				
				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in, out] pFileSystem A pointer to IFileSystem's implementation
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITexture2DLoader)
	};


	/*!
		struct TTexture2DParameters

		\brief The stucture contains fields for creation ITexture2D objects
	*/

	typedef struct TTexture2DParameters : TBaseResourceParameters
	{
		IGraphicsContext* mpGraphicsContext;

		U32               mWidth;
			              
		U32               mHeight;

		E_FORMAT_TYPE     mFormat;

		U32               mNumOfMipLevels;
			              
		U32               mNumOfSamples;
			              
		U32               mSamplingQuality;
	} TTexture2DParameters, *TTexture2DParametersPtr;


	/*!
		interface ITexture2DFactory

		\brief The interface describes a functionality of a 2d texture factory
	*/

	class ITexture2DFactory : public IResourceFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITexture2DFactory)
	};
}
