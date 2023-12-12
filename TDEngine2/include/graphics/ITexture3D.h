/*!
	\file ITexture3D.h
	\date 28.11.2023
	\authors Kasimov Ildar
*/

#pragma once


#include "ITexture2D.h"
#include "../core/IResourceLoader.h"
#include "../core/IResourceFactory.h"
#include "../math/TRect.h"
#include <string>
#include <memory>


namespace TDEngine2
{
	/*!
		struct TTexture3DParameters

		\brief The stucture contains fields for creation ITexture3D objects
	*/

	typedef struct TTexture3DParameters : TTexture2DParameters
	{
		TDE2_API TTexture3DParameters() = default;

		/*!
			\brief The main constructor of the struct

			\note TTexture3DParameters's definition is placed in CBaseTexture3D.cpp file
		*/

		TDE2_API TTexture3DParameters(U32 width, U32 height, U32 depth, E_FORMAT_TYPE format, U32 mipLevelsCount = 1, U32 samplesCount = 1, U32 samplingQuality = 0);
		
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

		U32 mDepth = 1;

		U32 mInputSheetColsCount = 1;
		U32 mInputSheetRowsCount = 1;
	} TTexture3DParameters, *TTexture3DParametersPtr;


	/*!
		interface ITexture3D

		\brief The interface describes a common functionality of volumetric textures
	*/

	class ITexture3D: public ITexture
	{
		public:
			TDE2_REGISTER_TYPE(ITexture3D);

			/*!
				\brief The method initializes an internal state of a 3D texture. The overloaded version of
				Init method creates a new texture object within memory with given width, height and format

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\param[in] params Additional parameters of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, 
												const TTexture3DParameters& params) = 0;

			/*!
				\brief The method writes data into a specified texture's region

				\param[in] regionRect A region, which will be overwritten

				\param[in] pData Data that will be written into a given region

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE WriteData(const TRectI32& regionRect, U32 depthFirstSlice, U32 depthLastSlice, const U8* pData) = 0;

			/*!
				\brief The method returns an internal data that the texture stores. (The returned data is allocated
				on heap so should be manually deleted later) For now we use std::unique_ptr instead

				\return A pointer to texture's memory, which size equals to width * height * bytes_per_channel
			*/

			TDE2_API virtual std::vector<U8> GetInternalData() = 0;

			TDE2_API virtual U32 GetDepth() const = 0;

			TDE2_API virtual TTextureHandleId GetHandle() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITexture3D)
	};


	TDE2_DECLARE_SCOPED_PTR(ITexture3D)


	class ITexture3DLoader : public IGenericResourceLoader<IResourceManager*, IGraphicsContext*, IFileSystem*> {};
	class ITexture3DFactory : public IGenericResourceFactory<IResourceManager*, IGraphicsContext*> {};
}
