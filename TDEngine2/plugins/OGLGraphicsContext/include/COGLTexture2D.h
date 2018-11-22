/*!
	\file COGLTexture2D.h
	\date 20.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CBaseTexture2D.h>
#include <GL/glew.h>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of COGLTexture2D's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[in] id An identifier of a resource

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to COGLTexture2D's implementation
	*/

	TDE2_API ITexture2D* CreateOGLTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
											TResourceId id, E_RESULT_CODE& result);

	/*!
		\brief A factory function for creation objects of COGLTexture2D's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[in] id An identifier of a resource

		\param[in] width Texture's width

		\param[in] height Texture's height

		\param[in] format Texture's format

		\param[in] mipLevelsCount An amount of levels, which represents a texture with different quality

		\param[in] samplesCount A number of multisamples per pixel

		\param[in] samplingQuality An image quality level

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to COGLTexture2D's implementation
	*/

	TDE2_API ITexture2D* CreateOGLTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
											  TResourceId id, U32 width, U32 height, E_FORMAT_TYPE format, U32 mipLevelsCount,
											  U32 samplesCount, U32 samplingQuality, E_RESULT_CODE& result);


	/*!
		class COGLTexture2D

		\brief The class represents an implementation of a 2d texture for OGL 3.x GAPI
	*/

	class COGLTexture2D : public CBaseTexture2D
	{
		public:
			friend TDE2_API ITexture2D* CreateOGLTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
														   TResourceId id, E_RESULT_CODE& result);

			friend TDE2_API ITexture2D* CreateOGLTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
														   TResourceId id, U32 width, U32 height, E_FORMAT_TYPE format, U32 mipLevelsCount,
														   U32 samplesCount, U32 samplingQuality, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(COGLTexture2D)

			/*!
				\brief The method loads resource data into memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load() override;

			/*!
				\brief The method unloads resource data from memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Unload() override;

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;

			/*!
				\brief The method writes data into a specified texture's region

				\param[in] regionRect A region, which will be overwritten

				\param[in] pData Data that will be written into a given region

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE WriteData(const TRectI32& regionRect, const U8* pData) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLTexture2D)

			TDE2_API E_RESULT_CODE _createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
																 U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality) override;
		protected:
			GLuint mTextureHandler;
	};


	/*!
		\brief A factory function for creation objects of COGLTexture2DFactory's type

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to COGLTexture2DFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateOGLTexture2DFactory(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class COGLTexture2DFactory

		\brief The class is an abstract factory of COGLTexture2D objects that
		is used by a resource manager
	*/

	class COGLTexture2DFactory : public ITexture2DFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateOGLTexture2DFactory(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* Create(const TBaseResourceParameters* pParams) const override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* CreateDefault(const TBaseResourceParameters& params) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TDE2_API U32 GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLTexture2DFactory)
		protected:
			bool              mIsInitialized;

			IGraphicsContext* mpGraphicsContext;
	};
}