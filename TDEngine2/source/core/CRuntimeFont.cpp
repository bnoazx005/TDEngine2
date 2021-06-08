#include "../../include/core/CRuntimeFont.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/platform/CYAMLFile.h"
#include "../../include/graphics/CTextureAtlas.h"
#include "../../include/utils/CU8String.h"
#include "../../include/graphics/IDebugUtility.h"
#include "../../include/math/MathUtils.h"
#include <cstring>


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

	E_RESULT_CODE CRuntimeFont::AddGlyphInfo(U8C codePoint, const TFontGlyphInfo& info)
	{
		E_RESULT_CODE result = CFont::AddGlyphInfo(codePoint, info);

		return result;
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

	E_RESULT_CODE CRuntimeFontFactory::Init(IResourceManager* pResourceManager)
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
		const TFontParameters& fontParams = dynamic_cast<const TFontParameters&>(params);

		auto pResource = CreateDefault(name, params);
		
		if (IFont* pFont = dynamic_cast<IFont*>(pResource))
		{
			pFont->SetTextureAtlasHandle(fontParams.mAtlasHandle);
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


	TDE2_API IResourceFactory* CreateRuntimeFontFactory(IResourceManager* pResourceManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CRuntimeFontFactory, result, pResourceManager);
	}
}