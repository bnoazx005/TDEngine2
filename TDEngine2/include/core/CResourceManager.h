/*!
	\file CResourceManager.h
	\date 18.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IResourceManager.h"
#include "CBaseObject.h"
#include "../utils/CResourceContainer.h"
#include <unordered_map>
#include <atomic>
#include <vector>
#include <mutex>
#include <list>


namespace TDEngine2
{
	class IResourceHandler;
	class IResource;


	/*!
		\brief A factory function for creation objects of CResourceManager's type.

		\param[in] pJobManager A pointer to IJobManager's implementation
				\param[in] pResourcesRuntimeManifest A pointer to IResourcesRuntimeManifest's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CResourceManager's implementation
	*/

	TDE2_API IResourceManager* CreateResourceManager(TPtr<IJobManager> pJobManager, TPtr<IResourcesRuntimeManifest> pResourcesRuntimeManifest, E_RESULT_CODE& result);


	/*!
		class CResourceManager

		\brief The class represents a root resource manager's functionality
	*/

	class CResourceManager : public IResourceManager, public CBaseObject
	{
		public:
			friend TDE2_API IResourceManager* CreateResourceManager(TPtr<IJobManager>, TPtr<IResourcesRuntimeManifest>, E_RESULT_CODE&);
		protected:
			typedef std::unordered_map<TypeId, TResourceLoaderId>         TResourceLoadersMap;
			
			typedef CResourceContainer<TPtr<IResourceLoader>>             TResourceLoadersContainer;

			typedef std::unordered_map<TypeId, TResourceFactoryId>        TResourceFactoriesMap;
			
			typedef CResourceContainer<TPtr<IResourceFactory>>            TResourceFactoriesContainer;

			typedef std::unordered_map<std::string, TResourceId>          TResourcesMap;

			typedef CResourceContainer<TPtr<IResource>>                   TResourcesContainer;

			typedef std::vector<std::tuple<TypeId, TypeId>>               TResourceTypesAliasesMap;

			typedef std::unordered_map<TypeId, E_RESOURCE_LOADING_POLICY> TResourceTypesPoliciesMap;
		public:
			/*!
				\brief The method initializes an inner state of a resource manager

				\param[in] pJobManager A pointer to IJobManager's implementation
				\param[in] pResourcesRuntimeManifest A pointer to IResourcesRuntimeManifest's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(TPtr<IJobManager> pJobManager, TPtr<IResourcesRuntimeManifest> pResourcesRuntimeManifest) override;

			/*!
				\brief The method registers specified resource loader within a manager

				\param[in] pResourceLoader A pointer to IResourceLoader's implementation

				\return The method returns an object, which contains a status of method's execution and
				an identifier of the registred loader
			*/

			TDE2_API TResult<TResourceLoaderId> RegisterLoader(IResourceLoader* pResourceLoader) override;

			/*!
				\brief The method unregisters a resource loader with the specified identifier

				\param[in] resourceLoaderId An identifier of a resource loader

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnregisterLoader(const TResourceLoaderId& resourceLoaderId) override;
			
			/*!
				\brief The method registers specified resource factory within a manager

				\param[in] pResourceFactory A pointer to IResourceFactory's implementation

				\return The method returns an object, which contains a status of method's execution and
				an identifier of the registred factory
			*/

			TDE2_API TResult<TResourceFactoryId> RegisterFactory(IResourceFactory* pResourceFactory) override;

			/*!
				\brief The method unregisters a resource factory with the specified identifier

				\param[in] resourceFactoryId An identifier of a resource factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnregisterFactory(const TResourceFactoryId& resourceFactoryId) override;

			/*!
				\brief The method registers an alternative type that's used when a factory of original one isn't allowed

				\param[in] inputResourceType An original resource type (resource interface type)
				\param[in] aliasType An alternative of the original type that's used to seek off a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RegisterResourceTypeAlias(TypeId inputResourceType, TypeId aliasType) override;

			/*!
				\brief The method allows to define a global loading policy for specified type of a resource

				\param[in] resourceType A type of a resource for which the policy should be added
				\param[in] policy A type of loading that will be applied for all resources of given type

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RegisterTypeGlobalLoadingPolicy(TypeId resourceType, E_RESOURCE_LOADING_POLICY policy) override;

			/*!
				\brief The method allows to remove previously registered a global loading policy for specified type of a resource

				\param[in] resourceType A type of a resource for which the policy should be unregistered

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnregisterTypeGlobalLoadingPolicy(TypeId resourceType) override;

			/*!
				\brief The method loads specified type of a resource using its name. This version is the same
				as invokation of Load<T>(name)

				\param[in] name A name of a resource that should be loaded
				\param[in] typeId A identifier of type which we try to load

				\return A handle of loaded resource, TResourceId::Invalid if some error has happened
			*/

			TDE2_API TResourceId Load(const std::string& name, TypeId typeId, E_RESOURCE_LOADING_POLICY loadingPolicy) override;

			/*!
				\brief The method decrements internal reference counter of the resource which corresponds to given identifier
				If the coutner goes down to zero the resource is unloaded and destroyed

				\param[in] id Unique identifier of the resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ReleaseResource(const TResourceId& id) override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

			/*!
				\brief The method returns a raw pointer to a resource based on specified handler

				\param[in] handle An identifier of a resource

				\return The method returns a raw pointer to a resource based on specified handler
			*/

			TDE2_API TPtr<IResource> GetResource(const TResourceId& handle) const override;

			/*!
				\brief The method returns an identifier of a resource with a given name. If there is no
				specified resource method returns TInvalidResourceId

				\param[in] name A name of a resource

				\return The method returns an identifier of a resource with a given name. If there is no
				specified resource method returns TInvalidResourceId
			*/

			TDE2_API TResourceId GetResourceId(const std::string& name) const override;

			TDE2_API E_RESULT_CODE SetResourceMeta(const std::string& name, std::unique_ptr<TBaseResourceParameters> pMeta) override;
			TDE2_API const TBaseResourceParameters* GetResourceMeta(const std::string& name) const override;

			TDE2_API TPtr<IResourcesRuntimeManifest> GetResourcesRuntimeManifest() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CResourceManager)

			TDE2_API TResourceId _loadResource(TypeId resourceTypeId, const std::string& name, E_RESOURCE_LOADING_POLICY loadingPolicy) override;
			TDE2_API TResourceId _loadResourceWithResourceProviderInfo(TypeId resourceTypeId, TypeId factoryTypeId, TypeId loaderTypeId, const std::string& name, E_RESOURCE_LOADING_POLICY loadingPolicy) override;

			TDE2_API TResourceId _createResource(TypeId resourceTypeId, const std::string& name, const TBaseResourceParameters& params) override;
			
			TDE2_API TPtr<IResource> _getResourceInternal(const TResourceId& handle) const;

			TDE2_API const TPtr<IResourceLoader> _getResourceLoader(TypeId resourceTypeId) const override;
			TDE2_API const TPtr<IResourceFactory> _getResourceFactory(TypeId resourceTypeId) const;

			TDE2_API std::vector<std::string> _getResourcesListByType(TypeId resourceTypeId) const override;

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			TResourceLoadersMap             mResourceLoadersMap;

			TResourceLoadersContainer       mRegisteredResourceLoaders;

			TResourceFactoriesMap           mResourceFactoriesMap;

			TResourceFactoriesContainer     mRegisteredResourceFactories;

			TResourcesMap                   mResourcesMap;

			TResourcesContainer             mResources;

			TResourceTypesAliasesMap        mResourceTypesAliases;

			TResourceTypesPoliciesMap       mResourceTypesPoliciesRegistry;

			TPtr<IJobManager>               mpJobManager;

			TPtr<IResourcesRuntimeManifest> mpResourcesRuntimeManifest;

			mutable std::mutex              mMutex;
	};
}
