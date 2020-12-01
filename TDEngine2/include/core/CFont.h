/*!
	\file CFont.h
	\date 19.09.2019
	\author Kasimov Ildar
*/

#pragma once


#include "IFont.h"
#include "CBaseResource.h"
#include "CBaseObject.h"
#include <string>
#include <unordered_map>


namespace TDEngine2
{
	class IShader;
	class IBinaryFileReader;


	/*!
		\brief A factory function for creation objects of CFont's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation
		
		\param[in] name A resource's name

		\param[in] id An identifier of a resource

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFont's implementation
	*/

	TDE2_API IFont* CreateFontResource(IResourceManager* pResourceManager, const std::string& name, E_RESULT_CODE& result);


	/*!
		class CFont

		\brief The class represents a default implementation of a font resource, which
		is used to store infromation about glyphs
	*/

	class CFont : public CBaseResource, public IFont
	{
		public:
			friend 	TDE2_API IFont* CreateFontResource(IResourceManager* pResourceManager, const std::string& name, E_RESULT_CODE& result);
		public:
			typedef std::unordered_map<U8C, TFontGlyphInfo> TGlyphsMap;
		public:
			TDE2_REGISTER_RESOURCE_TYPE(CFont)
			TDE2_REGISTER_TYPE(CFont)

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
				\brief The method saves current state of the object into some representation

				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in] filename A name of a file into which the data will be written

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Serialize(IFileSystem* pFileSystem, const std::string& filename) override;

			/*!
				\brief The method restores state of the texture atlas based on information from a given file

				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in] filename A name of a file into which the data will be written

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Deserialize(IFileSystem* pFileSystem, const std::string& filename) override;

			/*!
				\brief The method adds information about a single glyph into the font's resource

				\param[in] codePoint A value of a glyph
				\param[in] info A structure that contains glyph's metrics (advance, width, height, ...)

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE AddGlyphInfo(U8C codePoint, const TFontGlyphInfo& info) override;

			/*!
				\brief The method generates 2D mesh for a given text based on font's settings

				\param[in] position A position of a top left corner of a text line
				\param[in] scale A scale of a text mesh
				\param[in] text An input text that should be rendered
				\param[on, out] pDebugUtility A pointer to IDebugUtility, which can be used as a debug facility

				\return An array of vertices positions, each 4 forms a quad which is a single glyph
			*/

			TDE2_API const TTextVertices& GenerateMesh(const TVector2& position, F32 scale, const CU8String& text, IDebugUtility* pDebugUtility = nullptr) override;
			/*!
				\brief The method returns a pointer to texture atlas that is linked with the font

				\return The method returns a pointer to texture atlas that is linked with the font
			*/

			TDE2_API ITexture2D* GetTexture() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFont)

			TDE2_API const IResourceLoader* _getResourceLoader() override;
		protected:
			TResourceId mFontTextureAtlasHandle;

			TTextVertices mLastGeneratedMesh;

			TGlyphsMap mGlyphsMap;
	};


	/*!
		\brief A factory function for creation objects of CFontLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pFileSystem A pointer to IFileSystem's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFontLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateFontLoader(IResourceManager* pResourceManager, IFileSystem* pFileSystem, E_RESULT_CODE& result);


	/*!
		class CFontLoader

		\brief The class implements a functionality of a base material loader
	*/

	class CFontLoader : public CBaseObject, public IFontLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateFontLoader(IResourceManager* pResourceManager, IFileSystem* pFileSystem, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				
				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem) override;

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

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFontLoader)
		protected:
			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;
	};


	/*!
		\brief A factory function for creation objects of CFontFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFontFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateFontFactory(IResourceManager* pResourceManager, E_RESULT_CODE& result);


	/*!
		class CFontFactory

		\brief The class is an abstract factory of CFont objects that
		is used by a resource manager
	*/

	class CFontFactory : public CBaseObject, public IFontFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateFontFactory(IResourceManager* pResourceManager, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a material factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager) override;

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

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFontFactory)
		protected:
			IResourceManager* mpResourceManager;
	};
}
