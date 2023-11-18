/*!
	\file ICubemapTexture.h
	\date 20.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/IResourceLoader.h"
#include "../core/IResourceFactory.h"
#include "../graphics/ITexture2D.h"
#include <string>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;


	enum class E_CUBEMAP_FACE: U8
	{
		POSITIVE_X,
		NEGATIVE_X,
		POSITIVE_Y,
		NEGATIVE_Y,
		POSITIVE_Z,
		NEGATIVE_Z
	};


	/*!
		interface ICubemapTexture

		\brief The interface describes a common functionality of cubemaps
	*/

	class ICubemapTexture : public ITexture
	{
		public:
			TDE2_REGISTER_TYPE(ICubemapTexture);

			/*!
				\brief The method initializes an internal state of a cubemap texture. The overloaded version of
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

				\param[in] face An index of cubemap's face
				\param[in] regionRect A region, which will be overwritten
				\param[in] pData Data that will be written into a given region

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE WriteData(E_CUBEMAP_FACE face, const TRectI32& regionRect, const U8* pData) = 0;

			/*!
				\brief The method allows to mark one side of a cubemap as loaded one. When all the sides are loaded the resource
				is marked as loaded.
			*/

			TDE2_API virtual void MarkFaceAsLoaded(E_CUBEMAP_FACE face) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ICubemapTexture)
	};


	class ICubemapTextureLoader : public IGenericResourceLoader<IResourceManager*, IGraphicsContext*, IFileSystem*> {};
	class ICubemapTextureFactory: public IGenericResourceFactory<IResourceManager*, IGraphicsContext*> {};
}
