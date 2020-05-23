#include "./../../include/core/CResourceManager.h"
#include "./../../include/core/IResourceLoader.h"
#include "./../../include/core/IJobManager.h"
#include "./../../include/core/IResourceFactory.h"
#include "./../../include/core/CResourceHandler.h"
#include "./../../include/core/IResource.h"
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

		E_RESULT_CODE result = RC_OK;

		IResourceHandler* pInvalidResourceHandler = CreateResourceHandler(this, TResourceId::Invalid, result);

		if (result != RC_OK)
		{
			return result;
		}

		mResourceHandlers.Add(pInvalidResourceHandler);

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CResourceManager::Free()
	{
		{
			std::lock_guard<std::mutex> lock(mMutex);

			if (!mIsInitialized)
			{
				return RC_FAIL;
			}

			E_RESULT_CODE result = _unloadAllResources();

			mIsInitialized = false;
		}

		delete this;

		return RC_OK;
	}

	TResult<TResourceLoaderId> CResourceManager::RegisterLoader(const IResourceLoader* pResourceLoader)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!mIsInitialized || !pResourceLoader)
		{
			return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		TypeId resourceTypeId = pResourceLoader->GetResourceTypeId(); // an id of a resource's type, which is processed with this loader

		TResourceLoaderId existingDuplicateId = mResourceLoadersMap[resourceTypeId];

		// if the duplicate exists, just return it
		if (existingDuplicateId != TResourceLoaderId::Invalid)
		{
			return TOkValue<TResourceLoaderId>(existingDuplicateId);
		}

		existingDuplicateId = TResourceLoaderId(mRegisteredResourceLoaders.Add(pResourceLoader) + 1);
		mResourceLoadersMap[resourceTypeId] = existingDuplicateId;

		return TOkValue<TResourceLoaderId>(existingDuplicateId);
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
			return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}
		
		TypeId resourceTypeId = pResourceFactory->GetResourceTypeId(); // an id of a resource's type, which is processed with this loader

		TResourceFactoryId existingDuplicateId = mResourceFactoriesMap[resourceTypeId];

		// if the duplicate exists, just return it
		if (existingDuplicateId != TResourceFactoryId::Invalid)
		{
			return TOkValue<TResourceFactoryId>(existingDuplicateId);
		}
		
		existingDuplicateId = TResourceFactoryId(mRegisteredResourceFactories.Add(pResourceFactory) + 1);
		mResourceFactoriesMap[resourceTypeId] = existingDuplicateId;

		return TOkValue<TResourceFactoryId>(existingDuplicateId);
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

	IResource* CResourceManager::GetResourceByHandler(const IResourceHandler* pResourceHandler) const
	{
		if (!pResourceHandler)
		{
			return nullptr;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		U32 resourceId = static_cast<U32>(pResourceHandler->GetResourceId());

		if ((static_cast<TResourceId>(resourceId) == TResourceId::Invalid) ||
			resourceId >= mResources.GetSize() + 1)
		{
			return nullptr;
		}

		return mResources[resourceId - 1].GetOrDefault(nullptr);
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

	IResourceHandler* CResourceManager::_loadResource(TypeId resourceTypeId, const std::string& name)
	{
		TResourceId resourceId = mResourcesMap[name];

		IResource* pResource = nullptr;

		IResourceHandler* pResourceHandler = nullptr;

		if (resourceId != TResourceId::Invalid) /// needed resource already exists
		{
			pResourceHandler = _createOrGetResourceHandler(resourceId);

			pResource = mResources[static_cast<U32>(resourceId) - 1].Get();

			if (pResource->GetState() == RST_PENDING)
			{
				pResource->Load(); /// \todo move loading in the background thread
			}

			return pResourceHandler;
		}

		auto factoryIdIter = mResourceFactoriesMap.find(resourceTypeId);

		if (factoryIdIter == mResourceFactoriesMap.cend())
		{
			return mResourceHandlers[0].Get(); /// return invalid handler
		}

		const IResourceFactory* pResourceFactory = mRegisteredResourceFactories[static_cast<U32>((*factoryIdIter).second) - 1].Get();
			
		pResource = pResourceFactory->CreateDefault(name, {});

		resourceId = TResourceId(mResources.Add(pResource) + 1);

		mResourcesMap[name] = resourceId;

		pResourceHandler = _createOrGetResourceHandler(resourceId);

		pResource->Load(); /// \todo move loading in the background thread

		return pResourceHandler;
	}

	IResourceHandler* CResourceManager::_createResource(TypeId resourceTypeId, const std::string& name, const TBaseResourceParameters& params)
	{
		TResourceId resourceId = GetResourceId(name);

		if (resourceId != TResourceId::Invalid)
		{
			return _createOrGetResourceHandler(resourceId);
		}

		auto factoryIdIter = mResourceFactoriesMap.find(resourceTypeId);

		if (factoryIdIter == mResourceFactoriesMap.cend())
		{
			return mResourceHandlers[0].Get(); /// return invalid handler
		}

		const IResourceFactory* pResourceFactory = mRegisteredResourceFactories[static_cast<U32>((*factoryIdIter).second) - 1].Get();
		
		IResource* pResource = nullptr;

		IResourceHandler* pResourceHandler = nullptr;
				
		/// \todo move it to a background thread
		pResource = pResourceFactory->Create(name, params);
		
		resourceId = TResourceId(mResources.Add(pResource) + 1);

		mResourcesMap[name] = resourceId;

		pResourceHandler = _createOrGetResourceHandler(resourceId);

		return pResourceHandler;
	}

	IResourceHandler* CResourceManager::_createOrGetResourceHandler(TResourceId resourceId)
	{
		U32 handlerHashValue = mResourceHandlersMap[resourceId];

		if (handlerHashValue) /// just return existing handler
		{
			return mResourceHandlers[handlerHashValue - 1].GetOrDefault(mResourceHandlers[0].Get());
		}

		E_RESULT_CODE result = RC_OK;

		IResourceHandler* pNewHandlerInstance = CreateResourceHandler(this, resourceId, result);
		
		if (result != RC_OK)
		{
			return mResourceHandlers[0].Get(); /// invalid handler
		}

		handlerHashValue = mResourceHandlers.Add(pNewHandlerInstance) + 1;

		mResourceHandlersMap[resourceId] = handlerHashValue;

		return pNewHandlerInstance;
	}
	
	E_RESULT_CODE CResourceManager::_freeResourceHandler(TResourceId resourceId)
	{
		U32 handlerHashValue = mResourceHandlersMap[resourceId];

		if (!handlerHashValue) /// there is no handler associated with resourceId
		{
			return RC_FAIL;
		}

		IResourceHandler* pNewHandlerInstance = mResourceHandlers[handlerHashValue].Get();

		pNewHandlerInstance->SetResourceId(TResourceId::Invalid);

		mResourceHandlers.RemoveAt(handlerHashValue);

		return RC_OK;
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
		CResourceManager* pResourceManagerInstance = new (std::nothrow) CResourceManager();

		if (!pResourceManagerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pResourceManagerInstance->Init(pJobManager);

		if (result != RC_OK)
		{
			delete pResourceManagerInstance;

			pResourceManagerInstance = nullptr;
		}

		return dynamic_cast<IResourceManager*>(pResourceManagerInstance);
	}
}