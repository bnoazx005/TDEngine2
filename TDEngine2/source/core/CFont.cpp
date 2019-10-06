#include "./../../include/core/CFont.h"
#include "./../../include/core/IResourceManager.h"
#include "./../../include/core/IFileSystem.h"
#include "./../../include/core/IResourceHandler.h"
#include "./../../include/platform/CYAMLFile.h"
#include "./../../include/graphics/CTextureAtlas.h"
#include "./../../include/utils/CU8String.h"
#include "./../../include/graphics/IDebugUtility.h"
#include <cstring>


namespace TDEngine2
{
	CFont::CFont() :
		CBaseResource()
	{
	}

	E_RESULT_CODE CFont::Init(IResourceManager* pResourceManager, const std::string& name)
	{
		E_RESULT_CODE result = _init(pResourceManager, name);

		if (result != RC_OK)
		{
			return result;
		}

		mLastGeneratedMesh.reserve(4 * 100); /// \note preallocate space for at least 100 letters

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CFont::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		const IResourceLoader* pResourceLoader = mpResourceManager->GetResourceLoader<CFont>();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = pResourceLoader->LoadResource(this);

		if (result != RC_OK)
		{
			mState = RST_PENDING;

			return result;
		}

		mState = RST_LOADED;

		return result;
	}

	E_RESULT_CODE CFont::Unload()
	{
		return Reset();
	}

	E_RESULT_CODE CFont::Reset()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}
	
	E_RESULT_CODE CFont::Serialize(IFileSystem* pFileSystem, const std::string& filename)
	{
		if (!pFileSystem || filename.empty())
		{
			return RC_INVALID_ARGS;
		}

		/// \note try to create YAML file with the given name
		auto pFile = pFileSystem->Get<IYAMLFileWriter>(pFileSystem->Open<IYAMLFileWriter>(filename, true).Get());

		Yaml::Node fontDesc{};

		fontDesc["name"] = mName;

		auto& textureAtlasReference = fontDesc["texture_atlas_info"];
		textureAtlasReference["path"] = dynamic_cast<IResource*>(mpFontTextureAtlas->Get(RAT_BLOCKING))->GetName();

		// TODO: add scale's info
		
		auto& glyphsMapDesc = fontDesc["glyphs_map_desc"];

		for (auto currMapEntry : mGlyphsMap)
		{
			auto& currGlyphDesc = glyphsMapDesc[std::to_string(currMapEntry.first)];
			
			auto& currInternalGlyphInfo = currMapEntry.second;

			currGlyphDesc["width"]   = std::to_string(currInternalGlyphInfo.mWidth);
			currGlyphDesc["height"]  = std::to_string(currInternalGlyphInfo.mHeight);
			currGlyphDesc["xoffset"] = std::to_string(currInternalGlyphInfo.mXCenter);
			currGlyphDesc["yoffset"] = std::to_string(currInternalGlyphInfo.mYCenter);
			currGlyphDesc["advance"] = std::to_string(currInternalGlyphInfo.mAdvance);
		}

		E_RESULT_CODE result = pFile->Serialize(fontDesc);
		result = result | pFile->Close();

		return result;
	}

	E_RESULT_CODE CFont::Deserialize(IFileSystem* pFileSystem, const std::string& filename)
	{
		if (!pFileSystem || filename.empty())
		{
			return RC_INVALID_ARGS;
		}

		TResult<TFileEntryId> fileReadingResult = pFileSystem->Open<IYAMLFileReader>(filename);

		if (fileReadingResult.HasError())
		{
			return fileReadingResult.GetError();
		}

		auto pYAMLFileReader = pFileSystem->Get<IYAMLFileReader>(fileReadingResult.Get());

		Yaml::Node fontDataRoot;

		E_RESULT_CODE result = RC_OK;

		if ((result = pYAMLFileReader->Deserialize(fontDataRoot)) != RC_OK)
		{
			return result;
		}

		pYAMLFileReader->Close();

		mName = fontDataRoot["name"].As<std::string>();

		/// \note read texture atlas that contains glyphs images
		const auto& textureAtlasPath = fontDataRoot["texture_atlas_info"]["path"].As<std::string>();

		if (!textureAtlasPath.empty())
		{
			mpFontTextureAtlas = mpResourceManager->Load<CTextureAtlas>(textureAtlasPath);
		}

		/// \note read glyphs parameters
		auto& glyphsMapDesc = fontDataRoot["glyphs_map_desc"];

		for (auto iter = glyphsMapDesc.Begin(); iter != glyphsMapDesc.End(); iter++)
		{
			auto& glyphDesc = (*iter).second;

			auto& currGlyphMapEntry = mGlyphsMap[std::stoi((*iter).first)];

			currGlyphMapEntry.mWidth   = glyphDesc["width"].As<U16>();
			currGlyphMapEntry.mHeight  = glyphDesc["height"].As<U16>();
			currGlyphMapEntry.mXCenter = glyphDesc["xoffset"].As<I16>();
			currGlyphMapEntry.mYCenter = glyphDesc["yoffset"].As<I16>();
			currGlyphMapEntry.mAdvance = glyphDesc["advance"].As<F32>();
		}

		return RC_OK;
	}

	E_RESULT_CODE CFont::AddGlyphInfo(U8C codePoint, const TFontGlyphInfo& info)
	{
		if (mGlyphsMap.find(codePoint) != mGlyphsMap.cend())
		{
			return RC_FAIL;
		}

		mGlyphsMap.emplace(codePoint, info);

		return RC_OK;
	}

	const CFont::TTextVertices& CFont::GenerateMesh(const TVector2& position, F32 scale, const CU8String& text, IDebugUtility* pDebugUtility)
	{
		auto pTextureAtlas = dynamic_cast<ITextureAtlas*>(mpFontTextureAtlas->Get(RAT_BLOCKING));

		TVector2 currPosition{ position };

		TRectF32 normalizedUVs;

		U8C currCodePoint = 0x0;

		F32 yOffset = 0.0f;

		TFontGlyphInfo* pCurrGlyphInfo = nullptr;

		F32 x0, x1, y0, y1;

		mLastGeneratedMesh.clear();

		for (U32 i = 0; i < text.Length(); ++i)
		{
			currCodePoint = text.At(i);

			if (currCodePoint != ' ')
			{
				pCurrGlyphInfo = &mGlyphsMap[currCodePoint];

				auto result = pTextureAtlas->GetNormalizedTextureRect(UTF8CharToString(currCodePoint));

				if (result.HasError())
				{
					continue;
				}

				normalizedUVs = result.Get();

				x0 = currPosition.x + scale * pCurrGlyphInfo->mXCenter;
				y0 = currPosition.y - scale * pCurrGlyphInfo->mYCenter;
				x1 = scale * pCurrGlyphInfo->mWidth;
				y1 = scale * (pCurrGlyphInfo->mHeight);

				mLastGeneratedMesh.push_back({ x0,      y0,      normalizedUVs.x,                       normalizedUVs.y });
				mLastGeneratedMesh.push_back({ x0 + x1, y0,      normalizedUVs.x + normalizedUVs.width, normalizedUVs.y });
				mLastGeneratedMesh.push_back({ x0,      y0 - y1, normalizedUVs.x,                       normalizedUVs.y + normalizedUVs.height });
				mLastGeneratedMesh.push_back({ x0 + x1, y0 - y1, normalizedUVs.x + normalizedUVs.width, normalizedUVs.y + normalizedUVs.height });
			}

			currPosition = currPosition + TVector2{ scale * (currCodePoint != ' ' ? pCurrGlyphInfo->mAdvance : 20.0f), 0.0f };
		}

		return mLastGeneratedMesh;
	}

	ITexture2D* CFont::GetTexture() const
	{
		if (!mIsInitialized)
		{
			return nullptr;
		}

		return dynamic_cast<ITextureAtlas*>(mpFontTextureAtlas)->GetTexture();
	}


	TDE2_API IFont* CreateFontResource(IResourceManager* pResourceManager, const std::string& name, E_RESULT_CODE& result)
	{
		CFont* pFontInstance = new (std::nothrow) CFont();

		if (!pFontInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pFontInstance->Init(pResourceManager, name);

		if (result != RC_OK)
		{
			delete pFontInstance;

			pFontInstance = nullptr;
		}

		return pFontInstance;
	}


	CFontLoader::CFontLoader() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CFontLoader::Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem)
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

	E_RESULT_CODE CFontLoader::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CFontLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		return dynamic_cast<IFont*>(pResource)->Deserialize(mpFileSystem, pResource->GetName() + ".font");
	}

	U32 CFontLoader::GetResourceTypeId() const
	{
		return CFont::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateFontLoader(IResourceManager* pResourceManager, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		CFontLoader* pFontLoaderInstance = new (std::nothrow) CFontLoader();

		if (!pFontLoaderInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pFontLoaderInstance->Init(pResourceManager, pFileSystem);

		if (result != RC_OK)
		{
			delete pFontLoaderInstance;

			pFontLoaderInstance = nullptr;
		}

		return pFontLoaderInstance;
	}


	CFontFactory::CFontFactory() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CFontFactory::Init(IResourceManager* pResourceManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CFontFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IResource* CFontFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		return CreateDefault(name, params);
	}

	IResource* CFontFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateFontResource(mpResourceManager, name, result));
	}

	U32 CFontFactory::GetResourceTypeId() const
	{
		return CFont::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateFontFactory(IResourceManager* pResourceManager, E_RESULT_CODE& result)
	{
		CFontFactory* pFontFactoryInstance = new (std::nothrow) CFontFactory();

		if (!pFontFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pFontFactoryInstance->Init(pResourceManager);

		if (result != RC_OK)
		{
			delete pFontFactoryInstance;

			pFontFactoryInstance = nullptr;
		}

		return pFontFactoryInstance;
	}
}