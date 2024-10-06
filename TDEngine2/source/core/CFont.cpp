#include "../../include/core/CFont.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/platform/CYAMLFile.h"
#include "../../include/graphics/CTextureAtlas.h"
#include "../../include/utils/CU8String.h"
#include "../../include/graphics/IDebugUtility.h"
#include "../../include/math/MathUtils.h"
#include "../../include/editor/CPerfProfiler.h"
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

		mFontTextureAtlasHandle = TResourceId::Invalid;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CFont::Reset()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}
	
	E_RESULT_CODE CFont::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		std::lock_guard<std::mutex> lock(mMutex);
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);

		PANIC_ON_FAILURE(pWriter->SetString("name", mName));

		pWriter->SetFloat("size", mFontHeight);

		E_RESULT_CODE result = RC_OK;

		{
			if ((result = pWriter->BeginGroup("texture_atlas_info")) != RC_OK)
			{
				return result;
			}

			PANIC_ON_FAILURE(pWriter->SetString("path", mpResourceManager->GetResource(mFontTextureAtlasHandle)->GetName()));

			if ((result = pWriter->EndGroup()) != RC_OK)
			{
				return result;
			}
		}

		// TODO: add scale's info

		if ((result = pWriter->BeginGroup("glyphs_map_desc", true)) != RC_OK)
		{
			return result;
		}

		for (auto currMapEntry : mGlyphsMap)
		{
			if ((result = pWriter->BeginGroup(std::to_string(static_cast<U32>(currMapEntry.first)))) != RC_OK)
			{
				return result;
			}

			auto& currInternalGlyphInfo = currMapEntry.second;

			PANIC_ON_FAILURE(pWriter->SetUInt32("width", currInternalGlyphInfo.mWidth));
			PANIC_ON_FAILURE(pWriter->SetUInt32("height", currInternalGlyphInfo.mHeight));
			PANIC_ON_FAILURE(pWriter->SetUInt16("xoffset", currInternalGlyphInfo.mXCenter));
			PANIC_ON_FAILURE(pWriter->SetUInt16("yoffset", currInternalGlyphInfo.mYCenter));
			PANIC_ON_FAILURE(pWriter->SetFloat("advance", currInternalGlyphInfo.mAdvance));

			if ((result = pWriter->EndGroup()) != RC_OK)
			{
				return result;
			}
		}

		if ((result = pWriter->EndGroup()) != RC_OK)
		{
			return result;
		}

		return RC_OK;
	}

	E_RESULT_CODE CFont::Load(IArchiveReader* pReader)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);

		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		mName = pReader->GetString("name");

		mFontHeight = pReader->GetFloat("size", 14.0f);

		E_RESULT_CODE result = RC_OK;

		/// \note read texture atlas that contains glyphs images
		{
			if ((result = pReader->BeginGroup("texture_atlas_info")) != RC_OK)
			{
				return result;
			}

			const auto& textureAtlasPath = pReader->GetString("path");

			if (!textureAtlasPath.empty())
			{
				mFontTextureAtlasHandle = mpResourceManager->Load<ITextureAtlas>(textureAtlasPath);
				TDE2_ASSERT(mFontTextureAtlasHandle != TResourceId::Invalid);
			}

			if ((result = pReader->EndGroup()) != RC_OK)
			{
				return result;
			}
		}

		/// \note read glyphs parameters
		if ((result = pReader->BeginGroup("glyphs_map_desc")) != RC_OK)
		{
			return result;
		}

		while (pReader->HasNextItem())
		{
			std::string key = pReader->GetCurrKey();

			auto& currGlyphMapEntry = mGlyphsMap[static_cast<TUtf8CodePoint>(std::stoi(key))];

			if ((result = pReader->BeginGroup(key)) != RC_OK)
			{
				return result;
			}

			currGlyphMapEntry.mWidth = pReader->GetUInt16("width");
			currGlyphMapEntry.mHeight = pReader->GetUInt16("height");
			currGlyphMapEntry.mXCenter = pReader->GetInt16("xoffset");
			currGlyphMapEntry.mYCenter = pReader->GetInt16("yoffset");
			currGlyphMapEntry.mAdvance = pReader->GetFloat("advance");

			if ((result = pReader->EndGroup()) != RC_OK)
			{
				return result;
			}
		}

		if ((result = pReader->EndGroup()) != RC_OK)
		{
			return result;
		}

		return RC_OK;
	}

	E_RESULT_CODE CFont::AddGlyphInfo(TUtf8CodePoint codePoint, const TFontGlyphInfo& info)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);

		return _addGlyphInfoInternal(codePoint, info);
	}

	E_RESULT_CODE CFont::SetTextureAtlasHandle(TResourceId atlasHandle)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);

		if (TResourceId::Invalid == atlasHandle)
		{
			return RC_INVALID_ARGS;
		}

		mFontTextureAtlasHandle = atlasHandle;

		return RC_OK;
	}

	E_RESULT_CODE CFont::SetFontHeight(F32 height)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);

		if (height < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mFontHeight = height;

		return RC_OK;
	}


	static bool IsHorCenterizeAlignPolicy(E_FONT_ALIGN_POLICY type)
	{
		return (E_FONT_ALIGN_POLICY::CENTER == type) || (E_FONT_ALIGN_POLICY::CENTER_BOTTOM == type) || (E_FONT_ALIGN_POLICY::CENTER_TOP == type);
	}

	static bool IsVertCenterizeAlignPolicy(E_FONT_ALIGN_POLICY type)
	{
		return (E_FONT_ALIGN_POLICY::LEFT_CENTER == type) || (E_FONT_ALIGN_POLICY::CENTER == type) || (E_FONT_ALIGN_POLICY::RIGHT_CENTER == type);
	}

	static bool IsRightsidedAlignPolicy(E_FONT_ALIGN_POLICY type)
	{
		return (E_FONT_ALIGN_POLICY::RIGHT_TOP == type) || (E_FONT_ALIGN_POLICY::RIGHT_CENTER == type) || (E_FONT_ALIGN_POLICY::RIGHT_BOTTOM == type);
	}

	static bool IsTopsidedAlignPolicy(E_FONT_ALIGN_POLICY type)
	{
		return (E_FONT_ALIGN_POLICY::RIGHT_TOP == type) || (E_FONT_ALIGN_POLICY::LEFT_TOP == type) || (E_FONT_ALIGN_POLICY::CENTER_TOP == type);
	}

	static bool IsControlCharacter(TUtf8CodePoint codePoint)
	{
		return codePoint == TUtf8CodePoint(' ') || codePoint == TUtf8CodePoint('\n');
	}


	CFont::TTextMeshData CFont::GenerateMesh(const TTextMeshBuildParams& params, const std::string& text)
	{
		TDE2_PROFILER_SCOPE("CFont::GenerateMesh");

		std::lock_guard<std::mutex> lock(mMutex);
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);

		TPtr<ITextureAtlas> pTextureAtlas = mpResourceManager->GetResource<ITextureAtlas>(mFontTextureAtlasHandle);
		if (!pTextureAtlas)
		{
			return { {}, ZeroVector2 };
		}

		const TRectF32& bounds = params.mBounds;
		const F32 scale = params.mScale;

		TRectF32 normalizedUVs;

		TUtf8CodePoint currCodePoint = TUtf8CodePoint::Invalid;

		F32 yOffset = 0.0f;

		TFontGlyphInfo* pCurrGlyphInfo = nullptr;

		F32 x0, x1, y0, y1;

		TTextVertices textMesh;

		TVector2 sizes = ZeroVector2;

		U16 indicesCount = 0;

		const TVector2 leftBottomPos = bounds.GetLeftBottom();
		const TVector2 rectSizes = bounds.GetSizes();

		TVector2 currPosition = leftBottomPos;

		auto&& it = text.cbegin();

		while (CU8String::MoveNext(it, text.cend(), currCodePoint))
		{
			if (!IsControlCharacter(currCodePoint))
			{
				pCurrGlyphInfo = &mGlyphsMap[currCodePoint];

				auto result = pTextureAtlas->GetNormalizedTextureRect(CU8String::UTF8CodePointToString(currCodePoint));

				if (result.HasError())
				{
					continue;
				}

				normalizedUVs = result.Get();

				x0 = currPosition.x + scale * pCurrGlyphInfo->mXCenter;
				y0 = currPosition.y - scale * pCurrGlyphInfo->mYCenter;
				x1 = scale * pCurrGlyphInfo->mWidth;
				y1 = scale * (pCurrGlyphInfo->mHeight);

				if (E_TEXT_OVERFLOW_POLICY::NO_BREAK == params.mOverflowPolicy)
				{
					sizes.y = CMathUtils::Max(sizes.y, y1);
				}

				textMesh.push_back({ x0,      y0,      normalizedUVs.x,                       normalizedUVs.y });
				textMesh.push_back({ x0 + x1, y0,      normalizedUVs.x + normalizedUVs.width, normalizedUVs.y });
				textMesh.push_back({ x0,      y0 - y1, normalizedUVs.x,                       normalizedUVs.y + normalizedUVs.height });
				textMesh.push_back({ x0 + x1, y0 - y1, normalizedUVs.x + normalizedUVs.width, normalizedUVs.y + normalizedUVs.height });

				indicesCount += 6;
			}

			/// \todo Replace 40.0f with proper variable fontHeight or something equivalent to that
			currPosition = currPosition + TVector2{ scale * ((currCodePoint != TUtf8CodePoint(' ')) ? pCurrGlyphInfo->mAdvance : 20.0f), (currCodePoint == TUtf8CodePoint('\n')) ? 40.0f : 0.0f };
			
			/// \note If the text goes out of bounds split it into a few lines
			if (currPosition.x > (leftBottomPos.x + rectSizes.x))
			{
				if ((E_TEXT_OVERFLOW_POLICY::BREAK_ALL == params.mOverflowPolicy) || (E_TEXT_OVERFLOW_POLICY::BREAK_SPACES == params.mOverflowPolicy && (currCodePoint == TUtf8CodePoint(' '))))
				{
					sizes.x = currPosition.x;

					currPosition.x = leftBottomPos.x;
					currPosition.y -= 40.0f;
				}
			}			
		}

		sizes.x = (sizes.x > 0 ? sizes.x : currPosition.x) - leftBottomPos.x;
		sizes.y = (sizes.y > 0 ? sizes.y : currPosition.y) - leftBottomPos.y;

		TVector2 textOffsetPosition = CFont::GetPositionFromAlignType(params.mAlignMode) * rectSizes;

		if (IsHorCenterizeAlignPolicy(params.mAlignMode))
		{
			textOffsetPosition.x -= sizes.x * 0.5f;
		}

		if (IsVertCenterizeAlignPolicy(params.mAlignMode))
		{
			textOffsetPosition.y -= sizes.y * 0.25f;
		}

		if (IsRightsidedAlignPolicy(params.mAlignMode))
		{
			textOffsetPosition.x -= sizes.x;
		}

		if (IsTopsidedAlignPolicy(params.mAlignMode))
		{
			textOffsetPosition.y -= sizes.y * 0.5f;
		}

		for (auto& v : textMesh)
		{
			v.x += textOffsetPosition.x;
			v.y += textOffsetPosition.y;
		}

		return { std::move(textMesh), sizes, indicesCount };
	}

	ITexture2D* CFont::GetTexture() const
	{
		if (!mIsInitialized)
		{
			return nullptr;
		}

		return mpResourceManager->GetResource<ITextureAtlas>(mFontTextureAtlasHandle)->GetTexture();
	}

	const TPtr<IResourceLoader> CFont::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IFont>();
	}

	E_RESULT_CODE CFont::_addGlyphInfoInternal(TUtf8CodePoint codePoint, const TFontGlyphInfo& info)
	{
		if (mGlyphsMap.find(codePoint) != mGlyphsMap.cend())
		{
			return RC_FAIL;
		}

		mGlyphsMap.emplace(codePoint, info);

		return RC_OK;
	}

	U32 CFont::GetDataVersionNumber() const
	{
		return 0x1;
	}

	F32 CFont::GetFontHeight() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);

		return mFontHeight;
	}

	F32 CFont::GetTextLength(const TTextMeshBuildParams& params, const std::string& text, USIZE pos, USIZE count) const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);

		const F32 scale = params.mScale;
		F32 textLength = 0.0f;

		TUtf8CodePoint currCodePoint = TUtf8CodePoint::Invalid;
		auto&& it = text.cbegin();

		USIZE counter = 0;

		if (!count)
		{
			return textLength;
		}

		while (CU8String::MoveNext(it, text.cend(), currCodePoint))
		{
			if (pos > counter || counter >= count || mGlyphsMap.find(currCodePoint) == mGlyphsMap.cend())
			{
				continue;
			}

			counter++;

			auto pCurrGlyphInfo = &mGlyphsMap.at(currCodePoint);
			if (!pCurrGlyphInfo)
			{
				continue;
			}

			textLength += scale * ((currCodePoint != TUtf8CodePoint(' ')) ? pCurrGlyphInfo->mAdvance : 20.0f); /// 20.0f is a constant for spacing \todo replace it later
		}

		return textLength;
	}


	TVector2 CFont::GetPositionFromAlignType(E_FONT_ALIGN_POLICY type)
	{
		switch (type)
		{
			case E_FONT_ALIGN_POLICY::LEFT_TOP:
				return TVector2(0.0f, 1.0f);

			case E_FONT_ALIGN_POLICY::CENTER_TOP:
				return TVector2(0.5f, 1.0f);

			case E_FONT_ALIGN_POLICY::RIGHT_TOP:
				return TVector2(1.0f, 1.0f);

			case E_FONT_ALIGN_POLICY::LEFT_CENTER:
				return TVector2(0.0f, 0.5f);

			case E_FONT_ALIGN_POLICY::CENTER:
				return TVector2(0.5f, 0.5f);

			case E_FONT_ALIGN_POLICY::RIGHT_CENTER:
				return TVector2(1.0f, 0.5f);

			case E_FONT_ALIGN_POLICY::LEFT_BOTTOM:
				return TVector2(0.0f, 0.0f);

			case E_FONT_ALIGN_POLICY::CENTER_BOTTOM:
				return TVector2(0.5f, 0.0f);

			case E_FONT_ALIGN_POLICY::RIGHT_BOTTOM:
				return TVector2(1.0f, 0.0f);
		}

		TDE2_UNREACHABLE();
		return ZeroVector2;
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

	E_RESULT_CODE CFontLoader::LoadResource(IResource* pResource) const
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

	IResource* CFontFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		const TFontParameters& fontParams = dynamic_cast<const TFontParameters&>(params);

		auto pResource = CreateDefault(name, params);
		
		if (IFont* pFont = dynamic_cast<IFont*>(pResource))
		{
			pFont->SetTextureAtlasHandle(fontParams.mAtlasHandle);
		}

		return pResource;
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