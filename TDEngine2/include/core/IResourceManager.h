/*!
	\file IResourceManager.h
	\date 17.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IEngineSubsystem.h"
#include "../utils/Types.h"
#include "../utils/Utils.h"
#include <type_traits>
#include <string>


namespace TDEngine2
{
	class IResource;
	class IResourceLoader;
	class IJobManager;
	class IResourceFactory;

	struct TBaseResourceParameters;


	template <typename TResourceFactory, typename TResourceLoader>
	struct TResourceProviderInfo
	{
		static TypeId GetFactoryResourceId() { return TResourceFactory::GetTypeId(); }
		static TypeId GetLoaderResourceId() { return TResourceLoader::GetTypeId(); }
	};


	/*!
		interface IResourceManager

		\brief The manager describes a functionality of a resource manager,
		that is used within the engine
	*/

	class IResourceManager : public IEngineSubsystem
	{
		public:
			/*!
				\brief The method initializes an inner state of a resource manager
				
				\param[in] pJobManager A pointer to IJobManager's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IJobManager* pJobManager) = 0;

			/*!
				\brief The method registers specified resource loader within a manager

				\param[in] pResourceLoader A pointer to IResourceLoader's implementation

				\return The method returns an object, which contains a status of method's execution and
				an identifier of the registred loader
			*/

			TDE2_API virtual TResult<TResourceLoaderId> RegisterLoader(const IResourceLoader* pResourceLoader) = 0;

			/*!
				\brief The method unregisters a resource loader with the specified identifier

				\param[in] resourceLoaderId An identifier of a resource loader

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE UnregisterLoader(const TResourceLoaderId& resourceLoaderId) = 0;

			/*!
				\brief The method loads specified type of a resource using its name

				\param[in] name A name of a resource that should be loaded

				\return A handle of loaded resource, TResourceId::Invalid if some error has happened
			*/

			template <typename T> 
#if 0
			TDE2_API 
#if _HAS_CXX17
			std::enable_if_t<std::is_base_of_v<IResource, T>, TResourceId>
#else
			typename std::enable_if<std::is_base_of<IResource, T>::value, TResourceId>::type
#endif
#endif
			TResourceId
			Load(const std::string& name)
			{
				return _loadResource(T::GetTypeId(), name);
			}

			/*!
				\brief The method loads specified type with particular factory and loader

				\param[in] name A name of a resource that should be loaded

				\return A handle of loaded resource, TResourceId::Invalid if some error has happened
			*/

			template <typename T, typename TResourceProviderInfo>
			TDE2_API
#if _HAS_CXX17
			std::enable_if_t<std::is_base_of_v<IResource, T>, TResourceId>
#else
			typename std::enable_if<std::is_base_of<IResource, T>::value, TResourceId>::type
#endif
			Load(const std::string& name)
			{
				return _loadResourceWithResourceProviderInfo(T::GetTypeId(), TResourceProviderInfo::GetFactoryResourceId(), TResourceProviderInfo::GetLoaderResourceId(), name);
			}

			/*!
				\brief The method loads specified type of a resource using its name. This version is the same
				as invokation of Load<T>(name)

				\param[in] name A name of a resource that should be loaded
				\param[in] typeId A identifier of type which we try to load

				\return A handle of loaded resource, TResourceId::Invalid if some error has happened
			*/

			TDE2_API virtual TResourceId Load(const std::string& name, TypeId typeId) = 0;

			/*!
				\brief The method registers specified resource factory within a manager

				\param[in] pResourceFactory A pointer to IResourceFactory's implementation

				\return The method returns an object, which contains a status of method's execution and
				an identifier of the registred factory
			*/

			TDE2_API virtual TResult<TResourceFactoryId> RegisterFactory(const IResourceFactory* pResourceFactory) = 0;

			/*!
				\brief The method unregisters a resource factory with the specified identifier

				\param[in] resourceFactoryId An identifier of a resource factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE UnregisterFactory(const TResourceFactoryId& resourceFactoryId) = 0;
			
			/*!
				\brief The method creates a new instance of specified type. If there is existing resource of type T with
				specified name it will be returned as a result

				\param[in] name A name of a resource that will be created

				\param[in] params A parameters of a creating instance

				\return A handle of loaded resource, TResourceId::Invalid if some error has happened
			*/

			template <typename T>
#if 0 /// \fixme Check this later
			TDE2_API
#if _HAS_CXX17
			std::enable_if_t<std::is_base_of_v<IResource, T>, TResourceId>
#else
			typename std::enable_if<std::is_base_of<IResource, T>::value, TResourceId>::type
#endif
#endif
			TResourceId
			Create(const std::string& name, const TBaseResourceParameters& params)
			{
				return _createResource(T::GetTypeId(), name, params);
			}

			/*!
				\brief The method decrements internal reference counter of the resource which corresponds to given identifier
				If the coutner goes down to zero the resource is unloaded and destroyed.

				Should be called only from IResource::Free when it's guaranteed that the resource's memory will be released 

				\param[in] id Unique identifier of the resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ReleaseResource(const TResourceId& id) = 0;

			/*!
				\brief The method returns a raw pointer to a resource based on specified handler

				\param[in] handle An identifier of a resource

				\return The method returns a raw pointer to a resource based on specified handler
			*/

			TDE2_API virtual IResource* GetResource(const TResourceId& handle) const = 0;
		
			template <typename T>
#if 0 /// \todo Fix this concept 
			TDE2_API
#if _HAS_CXX17
			std::enable_if_t<std::is_base_of_v<IResource, T>, T*>
#else
			typename std::enable_if<std::is_base_of<IResource, T>::value, T*>::type
#endif
#endif 
			T* GetResource(const TResourceId& handle) const
			{
				return dynamic_cast<T*>(GetResource(handle));
			}

			/*!
				\brief The method returns a pointer to IResourceLoader, which is a loader of specific type of resources
				
				\return The method returns a pointer to IResourceLoader, which is a loader of specific type of resources
			*/
			
			template <typename T>
			TDE2_API const IResourceLoader* GetResourceLoader() const
			{
				return _getResourceLoader(T::GetTypeId());
			}

			template <typename T>
			TDE2_API
#if _HAS_CXX17
				std::enable_if_t<std::is_base_of_v<IResource, T>, std::vector<std::string>>
#else
				typename std::enable_if<std::is_base_of<IResource, T>::value, std::vector<std::string>>::type
#endif
			GetResourcesListByType() const
			{
				return _getResourcesListByType(T::GetTypeId());
			}

			/*!
				\brief The method returns an identifier of a resource with a given name. If there is no
				specified resource method returns TInvalidResourceId

				\param[in] name A name of a resource

				\return The method returns an identifier of a resource with a given name. If there is no
				specified resource method returns TInvalidResourceId
			*/

			TDE2_API virtual TResourceId GetResourceId(const std::string& name) const = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_RESOURCE_MANAGER; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IResourceManager)

			TDE2_API virtual TResourceId _loadResource(TypeId resourceTypeId, const std::string& name) = 0;
			TDE2_API virtual TResourceId _loadResourceWithResourceProviderInfo(TypeId resourceTypeId, TypeId factoryTypeId, TypeId loaderTypeId, const std::string& name) = 0;

			TDE2_API virtual TResourceId _createResource(TypeId resourceTypeId, const std::string& name, const TBaseResourceParameters& params) = 0;

			TDE2_API virtual const IResourceLoader* _getResourceLoader(TypeId resourceTypeId) const = 0;

			TDE2_API virtual std::vector<std::string> _getResourcesListByType(TypeId resourceTypeId) const = 0;
	};
}
