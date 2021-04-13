#include "./../../include/core/CFont.h"
#include "./../../include/core/IResourceManager.h"
#include "./../../include/core/IFileSystem.h"
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
		auto pArchiveFile = pFileSystem->Get<IYAMLFileWriter>(pFileSystem->Open<IYAMLFileWriter>(filename, true).Get());

		PANIC_ON_FAILURE(pArchiveFile->SetString("name", mName));

		E_RESULT_CODE result = RC_OK;

		{
			if ((result = pArchiveFile->BeginGroup("texture_atlas_info")) != RC_OK)
			{
				return result;
			}

			PANIC_ON_FAILURE(pArchiveFile->SetString("path", mpResourceManager->GetResource(mFontTextureAtlasHandle)->GetName()));

			if ((result = pArchiveFile->EndGroup()) != RC_OK)
			{
				return result;
			}
		}

		// TODO: add scale's info
		
		if ((result = pArchiveFile->BeginGroup("glyphs_map_desc", true)) != RC_OK)
		{
			return result;
		}

		for (auto currMapEntry : mGlyphsMap)
		{
			if ((result = pArchiveFile->BeginGroup(std::to_string(currMapEntry.first))) != RC_OK)
			{
				return result;
			}
			
			auto& currInternalGlyphInfo = currMapEntry.second;

			PANIC_ON_FAILURE(pArchiveFile->SetUInt32("width", currInternalGlyphInfo.mWidth));
			PANIC_ON_FAILURE(pArchiveFile->SetUInt32("height", currInternalGlyphInfo.mHeight));
			PANIC_ON_FAILURE(pArchiveFile->SetUInt16("xoffset", currInternalGlyphInfo.mXCenter));
			PANIC_ON_FAILURE(pArchiveFile->SetUInt16("yoffset", currInternalGlyphInfo.mYCenter));
			PANIC_ON_FAILURE(pArchiveFile->SetFloat("advance", currInternalGlyphInfo.mAdvance));

			if ((result = pArchiveFile->EndGroup()) != RC_OK)
			{
				return result;
			}
		}

		if ((result = pArchiveFile->EndGroup()) != RC_OK ||
			(result = pArchiveFile->Close()) != RC_OK)
		{
			return result;
		}

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

		mName = pYAMLFileReader->GetString("name");

		E_RESULT_CODE result = RC_OK;

		/// \note read texture atlas that contains glyphs images
		{
			if ((result = pYAMLFileReader->BeginGroup("texture_atlas_info")) != RC_OK)
			{
				return result;
			}

			const auto& textureAtlasPath = pYAMLFileReader->GetString("path");

			if (!textureAtlasPath.empty())
			{
				mFontTextureAtlasHandle = mpResourceManager->Load<ITextureAtlas>(textureAtlasPath);
				TDE2_ASSERT(mFontTextureAtlasHandle != TResourceId::Invalid);
			}

			if ((result = pYAMLFileReader->EndGroup()) != RC_OK)
			{
				return result;
			}
		}

		/// \note read glyphs parameters
		if ((result = pYAMLFileReader->BeginGroup("glyphs_map_desc")) != RC_OK)
		{
			return result;
		}
		
		while (pYAMLFileReader->HasNextItem())
		{
			std::string key = pYAMLFileReader->GetCurrKey();

			auto& currGlyphMapEntry = mGlyphsMap[std::stoi(key)];

			if ((result = pYAMLFileReader->BeginGroup(key)) != RC_OK)
			{
				return result;
			}

			currGlyphMapEntry.mWidth   = pYAMLFileReader->GetUInt16("width");
			currGlyphMapEntry.mHeight  = pYAMLFileReader->GetUInt16("height");
			currGlyphMapEntry.mXCenter = pYAMLFileReader->GetInt16("xoffset");
			currGlyphMapEntry.mYCenter = pYAMLFileReader->GetInt16("yoffset");
			currGlyphMapEntry.mAdvance = pYAMLFileReader->GetFloat("advance");

			if ((result = pYAMLFileReader->EndGroup()) != RC_OK)
			{
				return result;
			}
		}

		if ((result = pYAMLFileReader->EndGroup()) != RC_OK)
		{
			return result;
		}

		return pYAMLFileReader->Close();
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
		auto pTextureAtlas = dynamic_cast<ITextureAtlas*>(mpResourceManager->GetResource(mFontTextureAtlasHandle));

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

		return dynamic_cast<ITextureAtlas*>(mpResourceManager->GetResource(mFontTextureAtlasHandle))->GetTexture();
	}

	const IResourceLoader* CFont::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IFont>();
	}


	TDE2_API IFont* CreateFontResource(IResourceManager* pResourceManager, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IFont, CFont, result, pResourceManager, name);
	}


	CFontLoader::CFontLoader() :
		CBaseObject()
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

	TypeId CFontLoader::GetResourceTypeId() const
	{
		return IFont::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateFontLoader(IResourceManager* pResourceManager, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CFontLoader, result, pResourceManager, pFileSystem);
	}


	CFontFactory::CFontFactory() :
		CBaseObject()
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

	TypeId CFontFactory::GetResourceTypeId() const
	{
		return IFont::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateFontFactory(IResourceManager* pResourceManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CFontFactory, result, pResourceManager);
	}
}