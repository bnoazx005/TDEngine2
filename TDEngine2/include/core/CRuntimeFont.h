/*!
	\file CRuntimeFont.h
	\date 08.06.2021
	\author Kasimov Ildar
*/

#pragma once


#include "CFont.h"
#include <unordered_set>
#include <vector>
#include <memory>


struct stbtt_fontinfo;


namespace TDEngine2
{
	class IShader;
	class IBinaryFileReader;
	class ITextureAtlas;


	/*!
		\brief A factory function for creation objects of CRuntimeFont's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation		
		\param[in] name A resource's name
		\param[in] id An identifier of a resource
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CRuntimeFont's implementation
	*/

	TDE2_API IRuntimeFont* CreateRuntimeFontResource(IResourceManager* pResourceManager, const std::string& name, E_RESULT_CODE& result);


	/*!
		class CRuntimeFont

		\brief The class represents a default implementation of a font resource that differs from CFont with a feature of
		runtime generation font's atlase
	*/

	class CRuntimeFont : public CFont, public IRuntimeFont
	{
		public:
			friend 	TDE2_API IRuntimeFont* CreateRuntimeFontResource(IResourceManager* pResourceManager, const std::string& name, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_RESOURCE_TYPE(CRuntimeFont)
			TDE2_REGISTER_TYPE(CRuntimeFont)

			/*!
				\brief The method initializes an internal state of a material

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, const std::string& name) override;

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;

			/*!
				\brief The method generates 2D mesh for a given text based on font's settings

				\param[in] params A set of parameters to configure text
				\param[in] text An input text that should be rendered

				\return An array of vertices positions, each 4 forms a quad which is a single glyph
			*/

			TDE2_API TTextMeshData GenerateMesh(const TTextMeshBuildParams& params, const CU8String& text) override;

			/*!
				\brief The method loads information from truetype font's file

				\param[in] pFontFile A pointer to stream of bytes that represents a TTF file

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE LoadFontInfo(IBinaryFileReader* pFontFile) override;

			TDE2_API E_RESULT_CODE SetFontHeight(F32 value) override;

			TDE2_API F32 GetFontHeight() const override;

			/*!
				\brief The method returns a pointer to texture atlas that is linked with the font

				\return The method returns a pointer to texture atlas that is linked with the font
			*/

			TDE2_API ITexture2D* GetTexture() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS_NO_DCTR(CRuntimeFont)
			virtual ~CRuntimeFont(); /// \note declare own destructor because of incomplete type as argument of std::unique_ptr

			TDE2_API E_RESULT_CODE _updateFontTextureCache(ITextureAtlas* pFontCacheTexture);

			TDE2_API const TPtr<IResourceLoader> _getResourceLoader() override;
		protected:
			std::unordered_set<U8C> mCachedGlyphs;

			mutable bool            mIsDirty = true;

			std::unique_ptr<stbtt_fontinfo> mpInternalFontInfo;

			std::vector<U8>         mFontInfoBytes;

			F32                     mFontHeight;
			F32                     mFontInternalScale;
	};


	/*!
		\brief A factory function for creation objects of CRuntimeFontLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pFileSystem A pointer to IFileSystem's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CRuntimeFontLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateRuntimeFontLoader(IResourceManager* pResourceManager, IFileSystem* pFileSystem, E_RESULT_CODE& result);


	/*!
		class CRuntimeFontLoader

		\brief The class implements a functionality of a base material loader
	*/

	class CRuntimeFontLoader : public CBaseObject, public IRuntimeFontLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateRuntimeFontLoader(IResourceManager* pResourceManager, IFileSystem* pFileSystem, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				
				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem) override;

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

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CRuntimeFontLoader)
		protected:
			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;
	};


	/*!
		\brief A factory function for creation objects of CRuntimeFontFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CRuntimeFontFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateRuntimeFontFactory(IResourceManager* pResourceManager, IFileSystem* pFileSystem, E_RESULT_CODE& result);


	/*!
		class CRuntimeFontFactory

		\brief The class is an abstract factory of CRuntimeFont objects that
		is used by a resource manager
	*/

	class CRuntimeFontFactory : public CBaseObject, public IRuntimeFontFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateRuntimeFontFactory(IResourceManager*, IFileSystem*, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem) override;

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

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CRuntimeFontFactory)
		protected:
			static constexpr U32 mAtlasSize = 1024;

			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;
	};
}
