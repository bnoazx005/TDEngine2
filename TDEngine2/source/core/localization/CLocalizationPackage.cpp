#include "../../../include/core/localization/CLocalizationPackage.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/platform/CYAMLFile.h"
#include "../../include/utils/CFileLogger.h"
#include "stringUtils.hpp"
#include <cstring>


namespace TDEngine2
{
	CLocalizationPackage::CLocalizationPackage() :
		CBaseResource()
	{
	}

	E_RESULT_CODE CLocalizationPackage::Init(IResourceManager* pResourceManager, const std::string& name)
	{
		E_RESULT_CODE result = _init(pResourceManager, name);
		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CLocalizationPackage::Reset()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CLocalizationPackage::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		pReader->BeginGroup("dictionary");
		
		while (pReader->HasNextItem())
		{
			pReader->BeginGroup("");
			{
				pReader->BeginGroup("locale_entity");
				{
					mDictionary.emplace(pReader->GetString("key"), pReader->GetString("value"));
				}
				pReader->EndGroup();
			}
			pReader->EndGroup();
		}
		
		pReader->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CLocalizationPackage::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	const CLocalizationPackage::TDictionary& CLocalizationPackage::GetDictionaryData() const
	{
		return mDictionary;
	}

	const TPtr<IResourceLoader> CLocalizationPackage::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<ILocalizationPackage>();
	}


	TDE2_API ILocalizationPackage* CreateLocalizationPackage(IResourceManager* pResourceManager, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ILocalizationPackage, CLocalizationPackage, result, pResourceManager, name);
	}


	CLocalizationPackageLoader::CLocalizationPackageLoader() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CLocalizationPackageLoader::Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem)
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

	E_RESULT_CODE CLocalizationPackageLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (TResult<TFileEntryId> fileHandle = mpFileSystem->Open<IYAMLFileReader>(pResource->GetName()))
		{
			return dynamic_cast<ILocalizationPackage*>(pResource)->Load(mpFileSystem->Get<IYAMLFileReader>(fileHandle.Get()));
		}

		return RC_FILE_NOT_FOUND;
	}

	TypeId CLocalizationPackageLoader::GetResourceTypeId() const
	{
		return ILocalizationPackage::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateLocalizationPackageLoader(IResourceManager* pResourceManager, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CLocalizationPackageLoader, result, pResourceManager, pFileSystem);
	}


	CLocalizationPackageFactory::CLocalizationPackageFactory() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CLocalizationPackageFactory::Init(IResourceManager* pResourceManager)
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

	IResource* CLocalizationPackageFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;
		return dynamic_cast<IResource*>(CreateLocalizationPackage(mpResourceManager, name, result));
	}

	IResource* CLocalizationPackageFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;
		return dynamic_cast<IResource*>(CreateLocalizationPackage(mpResourceManager, name, result));
	}

	TypeId CLocalizationPackageFactory::GetResourceTypeId() const
	{
		return ILocalizationPackage::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateLocalizationPackageFactory(IResourceManager* pResourceManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CLocalizationPackageFactory, result, pResourceManager);
	}
}