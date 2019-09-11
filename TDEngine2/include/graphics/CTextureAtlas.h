/*!
	\brief CTextureAtlas.h
	\date 10.09.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "ITextureAtlas.h"
#include "./../core/CBaseResource.h"
#include "./../math/TRect.h"
#include <string>
#include <unordered_map>


namespace TDEngine2
{
	class IFileSystem;
	class IGraphicsContext;
	class IResourceHandler;


	/*!
		\brief A factory function for creation objects of CTextureAtlas's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[in] params A parameters of created material

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTextureAtlas's implementation
	*/

	TDE2_API ITextureAtlas* CreateTextureAtlas(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
											   const TTexture2DParameters& params, E_RESULT_CODE& result);




	/*!
		interface CTextureAtlas

		\brief The implementation represents a basic functionality of a texture atlases
	*/

	class CTextureAtlas : public ITextureAtlas, public CBaseResource
	{
		public:
			friend TDE2_API ITextureAtlas* CreateTextureAtlas(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
															  const TTexture2DParameters& params, E_RESULT_CODE& result);
		public:
			typedef std::unordered_map<std::string, TRectF32> TAtlasRegistry;
		public:
			TDE2_REGISTER_TYPE(CTextureAtlas)

			/*!
				\brief The method initializes an internal state of a texture atlas.

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) override;

			/*!
				\brief The method initializes a state of a brand new texture atlas.

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\param[in] params Additional parameters of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
										const TTexture2DParameters& params) override;

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
				\brief The method binds a material to a rendering pipeline
			*/

			TDE2_API void Bind(U32 slot) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTextureAtlas)
		protected:
			IGraphicsContext* mpGraphicsContext;

			IResourceHandler* mpTextureResource;

			TAtlasRegistry    mAtlasEntities;
	};


	/*!
		\brief A factory function for creation objects of CTextureAtlasLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in, out] pFileSystem A pointer to IFileSystem's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTextureAtlasLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateTextureAtlasLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
													   E_RESULT_CODE& result);


	/*!
		class CTextureAtlasLoader

		\brief The class is an implementation of a loader of texture atlases
	*/

	class CTextureAtlasLoader : public ITextureAtlasLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateTextureAtlasLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
																	  E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method loads data into the specified resource based on its
				internal information

				\param[in, out] pResource A pointer to an allocated resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE LoadResource(IResource* pResource) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the loader serves

				\return The method returns an identifier of a resource's type, which
				the loader serves
			*/

			TDE2_API U32 GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTextureAtlasLoader)
		protected:
			bool              mIsInitialized;

			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;

			IGraphicsContext* mpGraphicsContext;
	};


	/*!
		\brief A factory function for creation objects of CTextureAtlasFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTextureAtlasFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateTextureAtlasFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CTextureAtlasFactory

		\brief The class is an implementation of a factory of texture atlases
	*/

	class CTextureAtlasFactory : public ITextureAtlasFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateTextureAtlasFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* Create(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* CreateDefault(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TDE2_API U32 GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTextureAtlasFactory)
		protected:
			bool              mIsInitialized;

			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}
