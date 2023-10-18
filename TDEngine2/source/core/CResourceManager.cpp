#include "../../include/core/CResourceManager.h"
#include "../../include/core/IResourceLoader.h"
#include "../../include/core/IJobManager.h"
#include "../../include/core/IResourceFactory.h"
#include "../../include/core/IResource.h"
#include "../../include/core/IResourcesRuntimeManifest.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CPerfProfiler.h"
#include <memory>
#include <algorithm>


namespace TDEngine2
{
	CResourceManager::CResourceManager():
		CBaseObject()
	{
	}

	E_RESULT_CODE CResourceManager::Init(TPtr<IJobManager> pJobManager, TPtr<IResourcesRuntimeManifest> pResourcesRuntimeManifest)
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
		mpResourcesRuntimeManifest = pResourcesRuntimeManifest;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CResourceManager::_onFreeInternal()
	{
		return RC_OK;
	}

	TResult<TResourceLoaderId> CResourceManager::RegisterLoader(IResourceLoader* pResourceLoader)
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

		existingDuplicateId = TResourceLoaderId(mRegisteredResourceLoaders.Add(TPtr<IResourceLoader>(pResourceLoader)) + 1);
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
		
		auto result = mRegisteredResourceLoaders[static_cast<U32>(resourceLoaderId) - 1];

		if (result.HasError())
		{
			return result.GetError();
		}

		auto pResourceLoader = result.Get();

		mRegisteredResourceLoaders.ReplaceAt(static_cast<U32>(resourceLoaderId), TPtr<IResourceLoader>(nullptr));

		mResourceLoadersMap.erase(pResourceLoader->GetResourceTypeId()); // an id of a resource's type, which is processed with this loader

		return RC_OK;
	}

	TResult<TResourceFactoryId> CResourceManager::RegisterFactory(IResourceFactory* pResourceFactory)
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
		
		existingDuplicateId = TResourceFactoryId(mRegisteredResourceFactories.Add(TPtr<IResourceFactory>(pResourceFactory)) + 1);
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

		auto result = mRegisteredResourceFactories[index - 1];

		if (result.HasError())
		{
			return result.GetError();
		}

		auto pResourceFactory = result.Get();

		mRegisteredResourceFactories.ReplaceAt(index - 1, TPtr<IResourceFactory>(nullptr));

		mResourceFactoriesMap.erase(pResourceFactory->GetResourceTypeId()); // an id of a resource's type, which is processed with this factory

		return RC_OK;
	}

	E_RESULT_CODE CResourceManager::RegisterResourceTypeAlias(TypeId inputResourceType, TypeId aliasType)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (TypeId::Invalid == inputResourceType || TypeId::Invalid == aliasType)
		{
			return RC_INVALID_ARGS;
		}

		auto it = std::find_if(mResourceTypesAliases.cbegin(), mResourceTypesAliases.cend(), [inputResourceType, aliasType](auto&& entity)
		{
			return std::get<0>(entity) == inputResourceType && std::get<1>(entity) == aliasType;
		});

		if (it != mResourceTypesAliases.cend())
		{
			return RC_FAIL; /// \note There is a duplicate already
		}

		mResourceTypesAliases.emplace_back(inputResourceType, aliasType);

		return RC_OK;
	}

	E_RESULT_CODE CResourceManager::RegisterTypeGlobalLoadingPolicy(TypeId resourceType, E_RESOURCE_LOADING_POLICY policy)
	{
		if (TypeId::Invalid == resourceType)
		{
			return RC_INVALID_ARGS;
		}

		auto it = mResourceTypesPoliciesRegistry.find(resourceType);
		if (it != mResourceTypesPoliciesRegistry.cend())
		{
			return RC_FAIL;
		}

		mResourceTypesPoliciesRegistry[resourceType] = policy;

		return RC_OK;
	}

	E_RESULT_CODE CResourceManager::UnregisterTypeGlobalLoadingPolicy(TypeId resourceType)
	{
		if (TypeId::Invalid == resourceType)
		{
			return RC_INVALID_ARGS;
		}

		auto it = mResourceTypesPoliciesRegistry.find(resourceType);
		if (it == mResourceTypesPoliciesRegistry.cend())
		{
			return RC_FAIL;
		}

		mResourceTypesPoliciesRegistry.erase(it);

		return RC_OK;
	}

	E_ENGINE_SUBSYSTEM_TYPE CResourceManager::GetType() const
	{
		return EST_RESOURCE_MANAGER;
	}

	TPtr<IResource> CResourceManager::GetResource(const TResourceId& handle) const
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

	TResourceId CResourceManager::Load(const std::string& name, TypeId typeId, E_RESOURCE_LOADING_POLICY loadingPolicy)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _loadResource(typeId, name, loadingPolicy);
	}

	E_RESULT_CODE CResourceManager::SetResourceMeta(const std::string& name, std::unique_ptr<TBaseResourceParameters> pMeta)
	{
		if (name.empty() || !pMeta)
		{
			return RC_INVALID_ARGS;
		}

		std::lock_guard<std::mutex> lock(mMutex);
		return mpResourcesRuntimeManifest->AddResourceMeta(name, std::move(pMeta));
	}

	const TBaseResourceParameters* CResourceManager::GetResourceMeta(const std::string& name) const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mpResourcesRuntimeManifest ? mpResourcesRuntimeManifest->GetResourceMeta(name) : nullptr;
	}

	TResourceId CResourceManager::_loadResource(TypeId resourceTypeId, const std::string& name, E_RESOURCE_LOADING_POLICY loadingPolicy)
	{
		if (name.empty())
		{
			LOG_ERROR("[Resource Manager] Empty name is not allowed as a resource's identifier");
			return TResourceId::Invalid;
		}

		return _loadResourceWithResourceProviderInfo(resourceTypeId, resourceTypeId, resourceTypeId, name, loadingPolicy);
	}

	TResourceId CResourceManager::_loadResourceWithResourceProviderInfo(TypeId resourceTypeId, TypeId factoryTypeId, TypeId loaderTypeId, const std::string& name, E_RESOURCE_LOADING_POLICY loadingPolicy)
	{
		TDE2_PROFILER_SCOPE("CResourceManager::_loadResourceWithResourceProviderInfo");
		//std::lock_guard<std::mutex> lock(mMutex);

		auto&& iter = mResourcesMap.find(name);
		if ((iter != mResourcesMap.cend()) && (iter->second != TResourceId::Invalid)) /// needed resource already exists
		{
			auto getResourceResult = mResources[static_cast<U32>(iter->second)];
			if (getResourceResult.HasError())
			{
				return TResourceId::Invalid;
			}

			auto&& pResource = getResourceResult.Get();
			TDE2_ASSERT(pResource);

			if (pResource)
			{
				if (E_RESOURCE_STATE_TYPE::RST_PENDING == pResource->GetState())
				{
					E_RESULT_CODE result = pResource->Load(); /// \note Load is executed in sequential manner, but internally it can create background tasks
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
		auto&& pResourceFactory = _getResourceFactory(factoryTypeId);
		if (!pResourceFactory)
		{
			return TResourceId::Invalid;
		}

		auto it = mResourceTypesPoliciesRegistry.find(resourceTypeId);

		TBaseResourceParameters loadingParameters;
		loadingParameters.mLoadingPolicy = (E_RESOURCE_LOADING_POLICY::DEFAULT != loadingPolicy) || (it == mResourceTypesPoliciesRegistry.cend()) ? loadingPolicy : it->second;

		IResource* pResource = pResourceFactory->CreateDefault(name, loadingParameters);

		const TResourceId resourceId = TResourceId(mResources.Add(TPtr<IResource>(pResource)));

		mResourcesMap[name] = resourceId;

		pResource->Load(); /// \note Load is executed in sequential manner, but internally it can create background tasks

		return resourceId;
	}

	TResourceId CResourceManager::_createResource(TypeId resourceTypeId, const std::string& name, const TBaseResourceParameters& params)
	{
		TDE2_PROFILER_SCOPE("CResourceManager::_createResource");

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

		auto&& pResourceFactory = mRegisteredResourceFactories[static_cast<U32>((*factoryIdIter).second) - 1].Get();
		
		IResource* pResource = nullptr;
				
		/// \todo move it to a background thread
		pResource = pResourceFactory->Create(name, params);
		
		resourceId = TResourceId(mResources.Add(TPtr<IResource>(pResource)));

		mResourcesMap[name] = resourceId;

		pResource->OnCreated(this);

		return resourceId;
	}

	TPtr<IResource> CResourceManager::_getResourceInternal(const TResourceId& handle) const
	{
		if (handle == TResourceId::Invalid)
		{
			return TPtr<IResource>(nullptr);
		}

		const U32 resourceId = static_cast<U32>(handle);

		if (resourceId >= mResources.GetSize())
		{
			return TPtr<IResource>(nullptr);
		}

		return mResources[resourceId].GetOrDefault(TPtr<IResource>(nullptr));
	}

	E_RESULT_CODE CResourceManager::ReleaseResource(const TResourceId& id)
	{
		std::lock_guard<std::mutex> lock(mMutex);
	
		if (TResourceId::Invalid == id)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		auto pResource = _getResourceInternal(id);

		if (!pResource /*|| (E_RESOURCE_STATE_TYPE::RST_DESTROYING != pResource->GetState())*/) /// \note A resource should be marked as DESTROYING to allow its deletion from the registry
		{
			return RC_FAIL;
		}

		result = result | mResources.ReplaceAt(static_cast<U32>(id), TPtr<IResource>(nullptr));
		mResourcesMap.erase(mResourcesMap.find(pResource->GetName()));

		return result;
	}
	
	const TPtr<IResourceLoader> CResourceManager::_getResourceLoader(TypeId resourceTypeId) const
	{
		auto resourceLoaderIdIter = mResourceLoadersMap.find(resourceTypeId);

		if (resourceLoaderIdIter == mResourceLoadersMap.cend())
		{
			return TPtr<IResourceLoader>(nullptr);
		}

		return mRegisteredResourceLoaders[static_cast<U32>(resourceLoaderIdIter->second) - 1].GetOrDefault(TPtr<IResourceLoader>(nullptr));
	}

	const TPtr<IResourceFactory> CResourceManager::_getResourceFactory(TypeId resourceTypeId) const
	{
		auto factoryIdIter = mResourceFactoriesMap.find(resourceTypeId);

		auto getFactoryInternal = [&factoryIdIter, this]() -> TPtr<IResourceFactory>
		{
			if (auto getFactoryResult = mRegisteredResourceFactories[static_cast<U32>((*factoryIdIter).second) - 1])
			{
				return getFactoryResult.Get();
			}

			return TPtr<IResourceFactory>(nullptr);
		};

		if (factoryIdIter == mResourceFactoriesMap.cend())
		{
			/// \note We haven't found factory, try to find an alias type and its corresponding factory
			for (auto&& currResourceAliasInfo : mResourceTypesAliases)
			{
				if (std::get<0>(currResourceAliasInfo) != resourceTypeId)
				{
					continue;
				}

				factoryIdIter = mResourceFactoriesMap.find(std::get<1>(currResourceAliasInfo));
				if (factoryIdIter != mResourceFactoriesMap.cend())
				{
					return getFactoryInternal();
				}
			}

			return TPtr<IResourceFactory>(nullptr);
		}

		return getFactoryInternal();
	}

	std::vector<std::string> CResourceManager::_getResourcesListByType(TypeId resourceTypeId) const
	{
		std::vector<std::string> resourcesList;

		for (U32 i = 0; i < mResources.GetSize(); ++i)
		{
			if (auto getResourceResult = mResources[i])
			{
				auto&& pResource = getResourceResult.Get();

				if (pResource && (resourceTypeId == pResource->GetResourceTypeId()))
				{
					resourcesList.push_back(pResource->GetName());
				}
			}
		}

		return resourcesList;
	}


	IResourceManager* CreateResourceManager(TPtr<IJobManager> pJobManager, TPtr<IResourcesRuntimeManifest> pResourcesRuntimeManifest, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceManager, CResourceManager, result, pJobManager, pResourcesRuntimeManifest);
	}
}