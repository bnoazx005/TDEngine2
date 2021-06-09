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
#include "../../deps/stb/stb_truetype.h"


namespace TDEngine2
{
	CRuntimeFont::CRuntimeFont() :
		CFont()
	{
	}

	E_RESULT_CODE CRuntimeFont::Init(IResourceManager* pResourceManager, const std::string& name)
	{
		E_RESULT_CODE result = CFont::Init(pResourceManager, name);
		return result;
	}

	E_RESULT_CODE CRuntimeFont::Reset()
	{
		E_RESULT_CODE result = CFont::Reset();
		return result;
	}

	CFont::TTextMeshData CRuntimeFont::GenerateMesh(const TTextMeshBuildParams& params, const CU8String& text)
	{
		for (U32 i = 0; i < text.Length(); ++i)
		{
			auto codePoint = text.At(i);

			if (mCachedGlyphs.find(codePoint) != mCachedGlyphs.cend())
			{
				continue;
			}

			mCachedGlyphs.emplace(codePoint);
			mIsDirty = true;
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

		return pFontFile->Read(static_cast<void*>(&mFontInfoBytes.front()), static_cast<U32>(fileSize));
	}
	
	ITexture2D* CRuntimeFont::GetTexture() const
	{
		/// update font texture's cache
		if (ITextureAtlas* pTextureAtlas = mpResourceManager->GetResource<ITextureAtlas>(mFontTextureAtlasHandle))
		{
			// \note Generate SDF glyph data 

			//pTextureAtlas->AddRawTexture();

			E_RESULT_CODE result = pTextureAtlas->Bake();
			TDE2_ASSERT(RC_OK == result);
		}

		mIsDirty = false;

		return CFont::GetTexture();
	}

	const IResourceLoader* CRuntimeFont::_getResourceLoader()
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

	E_RESULT_CODE CRuntimeFontLoader::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

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

	E_RESULT_CODE CRuntimeFontFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

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
					pFont->LoadFontInfo(pFontFile);

					pFontFile->Close();
				}
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