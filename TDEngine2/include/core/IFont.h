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
#include "../core/IBaseObject.h"
#include "../math/TVector2.h"
#include "../math/TVector4.h"
#include "../math/TRect.h"
#include "../utils/CU8String.h"
#include <string>
#include <vector>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IFileSystem;
	class ITexture2D;
	class IDebugUtility;
	class IBinaryFileReader;


	enum class E_FONT_ALIGN_POLICY : U16
	{
		LEFT_TOP,
		CENTER_TOP,
		RIGHT_TOP,
		LEFT_CENTER,
		CENTER,
		RIGHT_CENTER,
		LEFT_BOTTOM,
		CENTER_BOTTOM,
		RIGHT_BOTTOM,
	};


	enum class E_TEXT_OVERFLOW_POLICY : U16
	{
		NO_BREAK,			///< All the text won't be splitted and is displayed though the single line
		BREAK_ALL,			///< The text is broken based on allowed rectangle
		BREAK_SPACES,		///< The text's splitted based on whitespaces and rectangle's sizes
	};


	/*!
		struct TFontParameters

		\brief The stucture contains fields for creation IFont objects
	*/

	typedef struct TFontParameters : TBaseResourceParameters
	{
		TResourceId mAtlasHandle;
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

	class IFont: public ISerializable, public virtual IBaseObject
	{
		public:
			typedef std::vector<TVector4> TTextVertices;

			typedef struct TTextMeshData
			{
				TTextVertices mVerts;
				TVector2      mTextRectSizes;
				U16           mNeededIndicesCount;
			} TTextMeshData, *TTextMeshDataPtr;

			typedef struct TTextMeshBuildParams
			{
				TRectF32               mBounds; ///< Left bottom point determines a position of a text
				F32                    mScale = 1.0f;
				E_TEXT_OVERFLOW_POLICY mOverflowPolicy = E_TEXT_OVERFLOW_POLICY::NO_BREAK;
				E_FONT_ALIGN_POLICY    mAlignMode = E_FONT_ALIGN_POLICY::CENTER;
			} TTextMeshBuildParams;

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

			TDE2_API virtual E_RESULT_CODE AddGlyphInfo(TUtf8CodePoint codePoint, const TFontGlyphInfo& info) = 0;

			/*!
				\brief The method links a texture atlas resource to current font entity. Notice that
				the method should not be called manually by a user

				\param[in] atlasHandle A handle of a new created atlas

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetTextureAtlasHandle(TResourceId atlasHandle) = 0;

			/*!
				\brief The method generates 2D mesh for a given text based on font's settings

				\param[in] params A set of parameters to configure text
				\param[in] text An input text that should be rendered

				\return An array of vertices positions, each 4 forms a quad which is a single glyph
			*/

			TDE2_API virtual TTextMeshData GenerateMesh(const TTextMeshBuildParams& params, const std::string& text) = 0;

			/*!
				\brief The method returns a pointer to texture atlas that is linked with the font

				\return The method returns a pointer to texture atlas that is linked with the font
			*/

			TDE2_API virtual ITexture2D* GetTexture() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IFont)
	};


	class IFontLoader : public IGenericResourceLoader<IResourceManager*, IFileSystem*> {};
	class IFontFactory : public IGenericResourceFactory<IResourceManager*> {};


	/*!
		struct TRuntimeFontParameters

		\brief The stucture contains fields for creation IRuntimeFont objects
	*/

	typedef struct TRuntimeFontParameters : TBaseResourceParameters
	{
		std::string mTrueTypeFontFilePath;
		F32 mGlyphHeight = 24.0f;
	} TRuntimeFontParameters, *TRuntimeFontParametersPtr;

	/*!
		interface IRuntimeFont

		\brief The interface describes a functionality of runtime generated font atlases
	*/

	class IRuntimeFont : public virtual IFont
	{
		public:
			TDE2_REGISTER_TYPE(IRuntimeFont);

			/*!
				\brief The method loads information from truetype font's file 

				\param[in] pFontFile A pointer to stream of bytes that represents a TTF file

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE LoadFontInfo(IBinaryFileReader* pFontFile) = 0;

			/*!
				\brief The method sets up glyph's height. Should be invoked only after LoadFontInfo. In 
				other cases will return RC_FAIL
			*/

			TDE2_API virtual E_RESULT_CODE SetFontHeight(F32 value) = 0;

			TDE2_API virtual F32 GetFontHeight() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IRuntimeFont)
	};


	class IRuntimeFontLoader : public IGenericResourceLoader<IResourceManager*, IFileSystem*> {};
	class IRuntimeFontFactory : public IGenericResourceFactory<IResourceManager*, IFileSystem*> {};
}
