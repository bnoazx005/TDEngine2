#include "../../include/core/CResourceManager.h"
#include "../../include/core/IResourceLoader.h"
#include "../../include/core/IJobManager.h"
#include "../../include/core/IResourceFactory.h"
#include "../../include/core/IResource.h"
#include <memory>


namespace TDEngine2
{
	CResourceManager::CResourceManager():
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CResourceManager::Init(IJobManager* pJobManager)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pJobManager)
		{
			return RC_INVALID_ARGS;
		}

		mpJobManager = pJobManager;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CResourceManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		{
			std::lock_guard<std::mutex> lock(mMutex);

			E_RESULT_CODE result = _unloadAllResources();
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	TResult<TResourceLoaderId> CResourceManager::RegisterLoader(const IResourceLoader* pResourceLoader)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!mIsInitialized || !pResourceLoader)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		TypeId resourceTypeId = pResourceLoader->GetResourceTypeId(); // an id of a resource's type, which is processed with this loader

		TResourceLoaderId existingDuplicateId = mResourceLoadersMap[resourceTypeId];

		// if the duplicate exists, just return it
		if (existingDuplicateId != TResourceLoaderId::Invalid)
		{
			return Wrench::TOkValue<TResourceLoaderId>(existingDuplicateId);
		}

		existingDuplicateId = TResourceLoaderId(mRegisteredResourceLoaders.Add(pResourceLoader) + 1);
		mResourceLoadersMap[resourceTypeId] = existingDuplicateId;

		return Wrench::TOkValue<TResourceLoaderId>(existingDuplicateId);
	}
	
	E_RESULT_CODE CResourceManager::UnregisterLoader(const TResourceLoaderId& resourceLoaderId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (resourceLoaderId == TResourceLoaderId::Invalid)
		{
			return RC_INVALID_ARGS;
		}
		
		TResult<const IResourceLoader*> result = mRegisteredResourceLoaders[static_cast<U32>(resourceLoaderId) - 1];

		if (result.HasError())
		{
			return result.GetError();
		}

		const IResourceLoader* pResourceLoader = result.Get();

		mRegisteredResourceLoaders.RemoveAt(static_cast<U32>(resourceLoaderId));

		mResourceLoadersMap.erase(pResourceLoader->GetResourceTypeId()); // an id of a resource's type, which is processed with this loader

		return RC_OK;
	}

	TResult<TResourceFactoryId> CResourceManager::RegisterFactory(const IResourceFactory* pResourceFactory)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		
		if (!mIsInitialized || !pResourceFactory)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}
		
		TypeId resourceTypeId = pResourceFactory->GetResourceTypeId(); // an id of a resource's type, which is processed with this loader

		TResourceFactoryId existingDuplicateId = mResourceFactoriesMap[resourceTypeId];

		// if the duplicate exists, just return it
		if (existingDuplicateId != TResourceFactoryId::Invalid)
		{
			return Wrench::TOkValue<TResourceFactoryId>(existingDuplicateId);
		}
		
		existingDuplicateId = TResourceFactoryId(mRegisteredResourceFactories.Add(pResourceFactory) + 1);
		mResourceFactoriesMap[resourceTypeId] = existingDuplicateId;

		return Wrench::TOkValue<TResourceFactoryId>(existingDuplicateId);
	}

	E_RESULT_CODE CResourceManager::UnregisterFactory(const TResourceFactoryId& resourceFactoryId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		U32 index = static_cast<U32>(resourceFactoryId);

		if ((resourceFactoryId == TResourceFactoryId::Invalid) || index > mRegisteredResourceFactories.GetSize())
		{
			return RC_INVALID_ARGS;
		}

		TResult<const IResourceFactory*> result = mRegisteredResourceFactories[index - 1];

		if (result.HasError())
		{
			return result.GetError();
		}

		const IResourceFactory* pResourceFactory = result.Get();

		mRegisteredResourceFactories.RemoveAt(index - 1);

		mResourceFactoriesMap.erase(pResourceFactory->GetResourceTypeId()); // an id of a resource's type, which is processed with this factory

		return RC_OK;
	}

	E_ENGINE_SUBSYSTEM_TYPE CResourceManager::GetType() const
	{
		return EST_RESOURCE_MANAGER;
	}

	IResource* CResourceManager::GetResource(const TResourceId& handle) const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return _getResourceInternal(handle);
	}

	TResourceId CResourceManager::GetResourceId(const std::string& name) const
	{
		if (name.empty())
		{
			return TResourceId::Invalid;
		}

		TResourcesMap::const_iterator resourceIter = mResourcesMap.find(name);

		if (resourceIter == mResourcesMap.cend())
		{
			return TResourceId::Invalid;
		}

		return (*resourceIter).second;
	}

	TResourceId CResourceManager::Load(const std::string& name, TypeId typeId)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _loadResource(typeId, name);
	}

	TResourceId CResourceManager::_loadResource(TypeId resourceTypeId, const std::string& name)
	{
		auto&& iter = mResourcesMap.find(name);
		if ((iter != mResourcesMap.cend()) && (iter->second != TResourceId::Invalid)) /// needed resource already exists
		{
			auto getResourceResult = mResources[static_cast<U32>(iter->second)];
			if (getResourceResult.HasError())
			{
				return TResourceId::Invalid;
			}

			IResource* pResource = getResourceResult.Get();
			TDE2_ASSERT(pResource);

			if (pResource)
			{
				if (E_RESOURCE_STATE_TYPE::RST_PENDING == pResource->GetState())
				{
					E_RESULT_CODE result = pResource->Load(); /// \todo move loading in the background thread
					if (RC_OK != result)
					{
						return TResourceId::Invalid;
					}
				}

				return iter->second;
			}

			TDE2_UNREACHABLE();
			return TResourceId::Invalid;
		}

		/// \note Create a new resource and load it				
		auto factoryIdIter = mResourceFactoriesMap.find(resourceTypeId);
		if (factoryIdIter == mResourceFactoriesMap.cend())
		{
			return TResourceId::Invalid;
		}

		const IResourceFactory* pResourceFactory = mRegisteredResourceFactories[static_cast<U32>((*factoryIdIter).second) - 1].Get();
			
		IResource* pResource = pResourceFactory->CreateDefault(name, {});

		const TResourceId resourceId = TResourceId(mResources.Add(pResource));

		mResourcesMap[name] = resourceId;
		
		pResource->Load(); /// \todo move loading in the background thread

		return resourceId;
	}

	TResourceId CResourceManager::_createResource(TypeId resourceTypeId, const std::string& name, const TBaseResourceParameters& params)
	{
		TResourceId resourceId = GetResourceId(name);

		if (TResourceId::Invalid != resourceId)
		{
			return resourceId;
		}

		auto factoryIdIter = mResourceFactoriesMap.find(resourceTypeId);
		if (factoryIdIter == mResourceFactoriesMap.cend())
		{
			return TResourceId::Invalid;
		}

		const IResourceFactory* pResourceFactory = mRegisteredResourceFactories[static_cast<U32>((*factoryIdIter).second) - 1].Get();
		
		IResource* pResource = nullptr;
				
		/// \todo move it to a background thread
		pResource = pResourceFactory->Create(name, params);
		
		resourceId = TResourceId(mResources.Add(pResource));

		mResourcesMap[name] = resourceId;

		return resourceId;
	}

	IResource* CResourceManager::_getResourceInternal(const TResourceId& handle) const
	{
		if (handle == TResourceId::Invalid)
		{
			return nullptr;
		}

		const U32 resourceId = static_cast<U32>(handle);

		if (resourceId >= mResources.GetSize())
		{
			return nullptr;
		}

		return mResources[resourceId].GetOrDefault(nullptr);
	}

	E_RESULT_CODE CResourceManager::ReleaseResource(const TResourceId& id)
	{
		std::lock_guard<std::mutex> lock(mMutex);
	
		if (TResourceId::Invalid == id)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		IResource* pResource = _getResourceInternal(id);

		if (!pResource /*|| (E_RESOURCE_STATE_TYPE::RST_DESTROYING != pResource->GetState())*/) /// \note A resource should be marked as DESTROYING to allow its deletion from the registry
		{
			return RC_FAIL;
		}

		result = result | mResources.RemoveAt(static_cast<U32>(id));
		mResourcesMap.erase(mResourcesMap.find(pResource->GetName()));

		if (pResource)
		{
			result = pResource->Free();
		}

		return result;
	}
	
	const IResourceLoader* CResourceManager::_getResourceLoader(TypeId resourceTypeId) const
	{
		auto resourceLoaderIdIter = mResourceLoadersMap.find(resourceTypeId);

		if (resourceLoaderIdIter == mResourceLoadersMap.cend())
		{
			return nullptr;
		}

		return mRegisteredResourceLoaders[static_cast<U32>(resourceLoaderIdIter->second) - 1].GetOrDefault(nullptr);
	}

	std::vector<std::string> CResourceManager::_getResourcesListByType(TypeId resourceTypeId) const
	{
		std::vector<std::string> resourcesList;

		for (U32 i = 0; i < mResources.GetSize(); ++i)
		{
			if (auto getResourceResult = mResources[i])
			{
				const IResource* pResource = getResourceResult.Get();

				if (pResource && (resourceTypeId == pResource->GetResourceTypeId()))
				{
					resourcesList.push_back(pResource->GetName());
				}
			}
		}

		return resourcesList;
	}

	E_RESULT_CODE CResourceManager::_unloadAllResources()
	{
		E_RESULT_CODE result = RC_OK;

		IResource* pCurrResource = nullptr;

		for (U32 i = 0; i < mResources.GetSize(); ++i)
		{
			if (auto resourceItem = mResources[i])
			{
				if (pCurrResource = resourceItem.Get())
				{
					result = result | pCurrResource->Free();
				}
			}
		}

		mResources.RemoveAll();

		return result;
	}


	IResourceManager* CreateResourceManager(IJobManager* pJobManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceManager, CResourceManager, result, pJobManager);
	}
}