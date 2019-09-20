/*!
	\file IFont.h
	\date 19.09.2019
	\author Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../core/IResourceLoader.h"
#include "./../core/IResourceFactory.h"
#include "./../math/TVector2.h"
#include "./../math/TVector4.h"
#include <string>
#include <vector>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IFileSystem;
	class ITexture2D;
	class CU8String;


	/*!
		struct TFontParameters

		\brief The stucture contains fields for creation IFont objects
	*/

	typedef struct TFontParameters : TBaseResourceParameters
	{
	} TFontParameters, *TFontParametersPtr;


	/*!
		interface IFont

		\brief The interface describes a functionality of a font resource
	*/

	class IFont
	{
		public:
			typedef std::vector<TVector4> TTextVertices;
		public:
			/*!
				\brief The method initializes an internal state of a font

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation				
				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, const std::string& name) = 0;

			/*!
				\brief The method restores state of the texture atlas based on information from a given file

				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in] filename A name of a file into which the data will be written

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Deserialize(IFileSystem* pFileSystem, const std::string& filename) = 0;
			
			/*!
				\brief The method generates 2D mesh for a given text based on font's settings

				\param[on] position A position of a top left corner of a text line
				\param[on] text An input text that should be rendered

				\return An array of vertices positions, each 4 forms a quad which is a single glyph
			*/

			TDE2_API virtual TTextVertices GenerateMesh(const TVector2& position, const CU8String& text) const = 0;

			/*!
				\brief The method returns a pointer to texture atlas that is linked with the font

				\return The method returns a pointer to texture atlas that is linked with the font
			*/

			TDE2_API virtual ITexture2D* GetTexture() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IFont)
	};


	/*!
		interface IFontLoader

		\brief The interface describes a functionality of a font loader
	*/

	class IFontLoader : public IResourceLoader
	{
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IFontLoader)
	};


	/*!
		interface IFontFactory

		\brief The interface describes a functionality of a font factory
	*/

	class IFontFactory : public IResourceFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a font factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IFontFactory)
	};
}
