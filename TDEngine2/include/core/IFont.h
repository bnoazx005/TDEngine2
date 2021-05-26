/*!
	\file IFont.h
	\date 19.09.2019
	\author Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../core/IResourceLoader.h"
#include "../core/IResourceFactory.h"
#include "../core/Serialization.h"
#include "../math/TVector2.h"
#include "../math/TVector4.h"
#include "../utils/CU8String.h"
#include <string>
#include <vector>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IFileSystem;
	class ITexture2D;
	class CU8String;
	class IDebugUtility;


	/*!
		struct TFontParameters

		\brief The stucture contains fields for creation IFont objects
	*/

	typedef struct TFontParameters : TBaseResourceParameters
	{
	} TFontParameters, *TFontParametersPtr;


	/*!
		struct TFontGlyphInfo

		\brief The structure contains all information about a single glyph of some font,
		which is needed to correctly display it
	*/

	typedef struct TFontGlyphInfo
	{
		U16 mWidth;
		U16 mHeight;
		I16 mXCenter;
		I16 mYCenter;
		F32 mAdvance;
	} TFontGlyphInfo, *TFontGlyphInfoPtr;


	/*!
		interface IFont

		\brief The interface describes a functionality of a font resource
	*/

	class IFont: public ISerializable
	{
		public:
			typedef std::vector<TVector4> TTextVertices;
		public:
			TDE2_REGISTER_TYPE(IFont);

			/*!
				\brief The method initializes an internal state of a font

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation				
				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, const std::string& name) = 0;

			/*!
				\brief The method adds information about a single glyph into the font's resource

				\param[in] codePoint A value of a glyph
				\param[in] info A structure that contains glyph's metrics (advance, width, height, ...)

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE AddGlyphInfo(U8C codePoint, const TFontGlyphInfo& info) = 0;

			/*!
				\brief The method generates 2D mesh for a given text based on font's settings

				\param[in] position A position of a top left corner of a text line
				\param[in] scale A scale of a text mesh
				\param[in] text An input text that should be rendered

				\return An array of vertices positions, each 4 forms a quad which is a single glyph
			*/

			TDE2_API virtual TTextVertices GenerateMesh(const TVector2& position, F32 scale, const CU8String& text) = 0;

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
