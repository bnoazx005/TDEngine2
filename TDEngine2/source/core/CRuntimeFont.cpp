#include "../../include/core/CRuntimeFont.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/platform/CYAMLFile.h"
#include "../../include/graphics/CTextureAtlas.h"
#include "../../include/graphics/ITexture2D.h"
#include "../../include/utils/CU8String.h"
#include "../../include/graphics/IDebugUtility.h"
#include "../../include/math/MathUtils.h"
#include <cstring>
#define STB_TRUETYPE_IMPLEMENTATION
#include "../../deps/stb/stb_truetype.h"


namespace TDEngine2
{
	CRuntimeFont::CRuntimeFont() :
		CFont()
	{
	}

	CRuntimeFont::~CRuntimeFont()
	{
	}

	E_RESULT_CODE CRuntimeFont::Init(IResourceManager* pResourceManager, const std::string& name)
	{
		E_RESULT_CODE result = CFont::Init(pResourceManager, name);

		mFontInfoBytes.clear();

		return result;
	}

	E_RESULT_CODE CRuntimeFont::Reset()
	{
		E_RESULT_CODE result = CFont::Reset();

		mFontInfoBytes.clear();

		return result;
	}


	static bool IsControlCharacter(U8C codePoint)
	{
		return codePoint == ' ' || codePoint == '\n';
	}


	CFont::TTextMeshData CRuntimeFont::GenerateMesh(const TTextMeshBuildParams& params, const CU8String& text)
	{
		for (U32 i = 0; i < text.Length(); ++i)
		{
			auto codePoint = text.At(i);

			if (mCachedGlyphs.find(codePoint) != mCachedGlyphs.cend() || IsControlCharacter(codePoint))
			{
				continue;
			}

			mCachedGlyphs.emplace(codePoint);
			mIsDirty = true;
		}

		if (mIsDirty)
		{
			/// update font texture's cache
			if (TPtr<IResource> pFontCacheTexture = mpResourceManager->GetResource(mFontTextureAtlasHandle))
			{
				pFontCacheTexture->Reset();
			}
			
			if (TPtr<ITextureAtlas> pTextureAtlas = mpResourceManager->GetResource<ITextureAtlas>(mFontTextureAtlasHandle))
			{
				E_RESULT_CODE result = _updateFontTextureCache(pTextureAtlas.Get());
				TDE2_ASSERT(RC_OK == result);
			}

			mIsDirty = false;
		}

		return std::move(CFont::GenerateMesh(params, text));
	}

	E_RESULT_CODE CRuntimeFont::LoadFontInfo(IBinaryFileReader* pFontFile)
	{
		if (!pFontFile)
		{
			return RC_INVALID_ARGS;
		}

		const U64 fileSize = pFontFile->GetFileLength();

		mFontInfoBytes.resize(static_cast<size_t>(fileSize));

		E_RESULT_CODE result = pFontFile->Read(static_cast<void*>(&mFontInfoBytes.front()), static_cast<U32>(fileSize));
		if (RC_OK != result)
		{
			TDE2_ASSERT(false);
			return result;
		}

		/// \note Initialize stb_truetype font's data
		mpInternalFontInfo = std::make_unique<stbtt_fontinfo>();
		TDE2_ASSERT(mpInternalFontInfo);

		const U8* pFontInfoPtr = &mFontInfoBytes.front();

		if (!stbtt_InitFont(mpInternalFontInfo.get(), pFontInfoPtr, stbtt_GetFontOffsetForIndex(pFontInfoPtr, 0)))
		{
			return RC_FAIL;
		}

		return result;
	}

	E_RESULT_CODE CRuntimeFont::SetFontHeight(F32 value)
	{
		if (value < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		if (mFontInfoBytes.empty()) /// \note The method should be invoked after font's initialization
		{
			return RC_FAIL;
		}

		mFontHeight = value;
		mFontInternalScale = stbtt_ScaleForPixelHeight(mpInternalFontInfo.get(), mFontHeight);

		return RC_OK;
	}

	F32 CRuntimeFont::GetFontHeight() const
	{
		return mFontHeight;
	}

	ITexture2D* CRuntimeFont::GetTexture() const
	{
		return CFont::GetTexture();
	}

	E_RESULT_CODE CRuntimeFont::_updateFontTextureCache(ITextureAtlas* pFontCacheTexture)
	{
		mGlyphsMap.clear();

		I32 width, height, xoff, yoff;
		I32 advance, leftBearing;

		const stbtt_fontinfo* pFontInfo = mpInternalFontInfo.get();

		I32 ascent, descent, lineGap;
		stbtt_GetFontVMetrics(pFontInfo, &ascent, &descent, &lineGap);

		const F32 scale = mFontInternalScale;

		ascent = static_cast<I32>(ascent * scale);
		descent = static_cast<I32>(descent * scale);

		I32 x0, y0, x1, y1;
		stbtt_GetFontBoundingBox(pFontInfo, &x0, &y0, &x1, &y1);

		F32 w = scale * (x1 - x0);
		F32 h = scale * (y1 - y0);

		E_RESULT_CODE result = RC_OK;

		// \note Generate SDF glyph data 
		for (U8C currCodepoint : mCachedGlyphs)
		{
			U8* pBitmap = stbtt_GetCodepointSDF(pFontInfo, scale, currCodepoint, 10, 255, 20.0f, &width, &height, &xoff, &yoff); /// \note Replace magic constants

			const C8* pStr = reinterpret_cast<const C8*>(&currCodepoint);

			if (RC_OK != (result = pFontCacheTexture->AddRawTexture(std::string(pStr), width, height, FT_NORM_UBYTE1, pBitmap)))
			{
				TDE2_ASSERT(false);
			}

			stbtt_GetCodepointHMetrics(pFontInfo, currCodepoint, &advance, &leftBearing);

			if (RC_OK != (result = AddGlyphInfo(currCodepoint, { static_cast<U16>(width), static_cast<U16>(height), static_cast<I16>(xoff), static_cast<I16>(yoff), scale * advance })))
			{
				TDE2_ASSERT(false);
			}
		}

		return pFontCacheTexture->Bake();
	}

	const TPtr<IResourceLoader> CRuntimeFont::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IRuntimeFont>();
	}


	TDE2_API IRuntimeFont* CreateRuntimeFontResource(IResourceManager* pResourceManager, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IRuntimeFont, CRuntimeFont, result, pResourceManager, name);
	}


	CRuntimeFontLoader::CRuntimeFontLoader() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CRuntimeFontLoader::Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || !pFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mpFileSystem = pFileSystem;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CRuntimeFontLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (TResult<TFileEntryId> fontFileId = mpFileSystem->Open<IYAMLFileReader>(pResource->GetName()))
		{
			return dynamic_cast<IFont*>(pResource)->Load(mpFileSystem->Get<IYAMLFileReader>(fontFileId.Get()));
		}

		return RC_FILE_NOT_FOUND;
	}

	TypeId CRuntimeFontLoader::GetResourceTypeId() const
	{
		return IRuntimeFont::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateRuntimeFontLoader(IResourceManager* pResourceManager, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CRuntimeFontLoader, result, pResourceManager, pFileSystem);
	}


	CRuntimeFontFactory::CRuntimeFontFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CRuntimeFontFactory::Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || !pFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpFileSystem = pFileSystem;

		mIsInitialized = true;

		return RC_OK;
	}

	IResource* CRuntimeFontFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		const TRuntimeFontParameters& fontParams = dynamic_cast<const TRuntimeFontParameters&>(params);

		auto pResource = CreateDefault(name, params);
		
		if (IRuntimeFont* pFont = dynamic_cast<IRuntimeFont*>(pResource))
		{
			/// \note Create a new texture atlas for a font cache
			pFont->SetTextureAtlasHandle(mpResourceManager->Create<ITextureAtlas>("Atlas_" + name, TTexture2DParameters { mAtlasSize, mAtlasSize, FT_NORM_UBYTE1 }));

			/// \note Load TTF file into the memory
			if (auto fontFileOpenResult = mpFileSystem->Open<IBinaryFileReader>(fontParams.mTrueTypeFontFilePath))
			{
				if (IBinaryFileReader* pFontFile = mpFileSystem->Get<IBinaryFileReader>(fontFileOpenResult.Get()))
				{
					E_RESULT_CODE result = pFont->LoadFontInfo(pFontFile);
					if (RC_OK != result)
					{
						TDE2_ASSERT(false);
						return nullptr;
					}

					pFontFile->Close();

					result = pFont->SetFontHeight(fontParams.mGlyphHeight);
					if (RC_OK != result)
					{
						TDE2_ASSERT(false);
						return nullptr;
					}
				}
			}
			else
			{
				return nullptr;
			}
		}

		return pResource;
	}

	IResource* CRuntimeFontFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateRuntimeFontResource(mpResourceManager, name, result));
	}

	TypeId CRuntimeFontFactory::GetResourceTypeId() const
	{
		return IRuntimeFont::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateRuntimeFontFactory(IResourceManager* pResourceManager, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CRuntimeFontFactory, result, pResourceManager, pFileSystem);
	}
}