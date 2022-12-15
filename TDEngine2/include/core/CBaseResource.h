/*!
	\file CBaseResource.h
	\date 17.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IResource.h"
#include "CBaseObject.h"
#include "../utils/Utils.h"
#include "IResourceManager.h"
#include "IResourceLoader.h"
#include "IResourceFactory.h"
#include <mutex>


namespace TDEngine2
{
	class IFileSystem;
	class IYAMLFileReader;


	/*!
		class CBaseResource

		\brief The class is a base class of a resource.
		All concrete types of resources should derive it.
	*/

	class CBaseResource: public virtual IResource, public CBaseObject
	{
		public:
			TDE2_REGISTER_TYPE(CBaseResource)

			/*!
				\brief The method loads resource data into memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load() override;

			/*!
				\brief The method unloads resource data from memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Unload() override;

			/*!
				\brief The method changes current state of the resource (don't use it manually)
			*/

			TDE2_API void SetState(E_RESOURCE_STATE_TYPE state) override;

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Reset() = 0;

			/*!
				\brief The method is called after the resource has been created
			*/

			TDE2_API void OnCreated(IResourceManager* pResourceManager) override;

			/*!
				\brief The method returns an identifier of a resource

				\return The method returns an identifier of a resource
			*/

			TDE2_API TResourceId GetId() const override;

			/*!
				\brief The method returns a resource's name

				\return The method returns a resource's name
			*/

			TDE2_API const std::string& GetName() const override;
			
			/*!
				\brief The method returns current state, in which a resource stays

				\return The method returns current state, in which a resource stays
			*/

			TDE2_API E_RESOURCE_STATE_TYPE GetState() const override;

			TDE2_API void SetLoadingPolicy(E_RESOURCE_LOADING_POLICY policy) override;

			/*!
				\return The method returns currently assigned policy for the resource
			*/

			TDE2_API E_RESOURCE_LOADING_POLICY GetLoadingPolicy() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseResource)

			virtual TDE2_API E_RESULT_CODE _init(IResourceManager* pResourceManager, const std::string& name);

			TDE2_API virtual const TPtr<IResourceLoader> _getResourceLoader() = 0;

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			IResourceManager*                  mpResourceManager;

			std::string                        mName;					///< The name's value is unique for each resource

			std::atomic<TResourceId>           mId;						///< The id's value is unique for each resource. Used for cross-referencing

			std::atomic<E_RESOURCE_STATE_TYPE> mState;

			E_RESOURCE_LOADING_POLICY          mLoadingPolicy;

			mutable std::mutex                 mMutex;
	};


	/*!
		class CBaseResourceLoader

		\brief The class implements a common implementation of a resource loader for simple resource types
		that don't need extra work
	*/

	template <typename TResourceType, typename... TInitArgs>
	class CBaseResourceLoader : public CBaseObject, public IGenericResourceLoader<IResourceManager*, IFileSystem*, TInitArgs...>
	{
		public:
			/*!
				\brief The method initializes an inner state of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem, TInitArgs... args) override
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

			/*!
				\brief The method loads data into the specified resource based on its
				internal information

				\param[in, out] pResource A pointer to an allocated resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE LoadResource(IResource* pResource) const override
			{
				if (!mIsInitialized)
				{
					return RC_FAIL;
				}

				if (TResult<TFileEntryId> fileLoadResult = mpFileSystem->Open<IYAMLFileReader>(pResource->GetName()))
				{
					return dynamic_cast<TResourceType*>(pResource)->Load(mpFileSystem->Get<IYAMLFileReader>(fileLoadResult.Get()));
				}

				return RC_FILE_NOT_FOUND;
			}

			/*!
				\brief The method returns an identifier of a resource's type, which
				the loader serves

				\return The method returns an identifier of a resource's type, which
				the loader serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override
			{
				return TResourceType::GetTypeId();
			}
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseResourceLoader)

		protected:
			IResourceManager* mpResourceManager;
			IFileSystem*      mpFileSystem;
	};


	template <typename TResourceType, typename... TInitArgs> CBaseResourceLoader<TResourceType, TInitArgs...>::CBaseResourceLoader():
		CBaseObject()
	{
	}


	template <typename TResourceType>
	class CBaseResourceLoader<TResourceType> : public CBaseObject, public IGenericResourceLoader<IResourceManager*, IFileSystem*>
	{
		public:
			/*!
				\brief The method initializes an inner state of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem) override
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

			/*!
				\brief The method loads data into the specified resource based on its
				internal information

				\param[in, out] pResource A pointer to an allocated resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE LoadResource(IResource* pResource) const override
			{
				if (!mIsInitialized)
				{
					return RC_FAIL;
				}

				if (TResult<TFileEntryId> fileLoadResult = mpFileSystem->Open<IYAMLFileReader>(pResource->GetName()))
				{
					return dynamic_cast<TResourceType*>(pResource)->Load(mpFileSystem->Get<IYAMLFileReader>(fileLoadResult.Get()));
				}

				return RC_FILE_NOT_FOUND;
			}

			/*!
				\brief The method returns an identifier of a resource's type, which
				the loader serves

				\return The method returns an identifier of a resource's type, which
				the loader serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override
			{
				return TResourceType::GetTypeId();
			}
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseResourceLoader)

		protected:
			IResourceManager* mpResourceManager;
			IFileSystem* mpFileSystem;
	};


	template <typename TResourceType> CBaseResourceLoader<TResourceType>::CBaseResourceLoader() :
		CBaseObject()
	{
	}


	/*!
		class CBaseResourceFactory

		\brief The class is a base implementation for simple types of resources which don't need extra initialization
		after their creation
	*/

	template <typename TResourceType, typename... TInitArgs>
	class CBaseResourceFactory : public CBaseObject, public IGenericResourceFactory<IResourceManager*, TInitArgs...>
	{
		public:
			/*!
				\brief The method initializes an internal state of a material factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, TInitArgs... args) override
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

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource
				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* Create(const std::string& name, const TBaseResourceParameters& params) const override
			{
				return CreateDefault(name, params);
			}

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API virtual IResource* CreateDefault(const std::string& name, const TBaseResourceParameters& params) const = 0;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override
			{
				return TResourceType::GetTypeId();
			}
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseResourceFactory)
		protected:
			IResourceManager* mpResourceManager;
	};


	template <typename TResourceType, typename... TInitArgs> 
	CBaseResourceFactory<TResourceType, TInitArgs...>::CBaseResourceFactory() :
		CBaseObject()
	{
	}


	template <typename TResourceType>
	class CBaseResourceFactory<TResourceType> : public CBaseObject, public IGenericResourceFactory<IResourceManager*>
	{
		public:
			/*!
				\brief The method initializes an internal state of a material factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager) override
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

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource
				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* Create(const std::string& name, const TBaseResourceParameters& params) const override
			{
				return CreateDefault(name, params);
			}

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API virtual IResource* CreateDefault(const std::string& name, const TBaseResourceParameters& params) const = 0;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override
			{
				return TResourceType::GetTypeId();
			}
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseResourceFactory)
		protected:
			IResourceManager* mpResourceManager;
	};


	template <typename TResourceType>
	CBaseResourceFactory<TResourceType>::CBaseResourceFactory() :
		CBaseObject()
	{
	}

#if 0
#define TDE2_RESOURCE_CLASS_NAME(ResourceName)				C ## ResourceName
#define TDE2_RESOURCE_FUNCTION_NAME(ResourceName)			Create ## ResourceName
#define TDE2_RESOURCE_LOADER_NAME(ResourceName)				C ## ResourceName ## Loader
#define TDE2_RESOURCE_LOADER_FUNCTION_NAME(ResourceName)	Create ## ResourceName ## Loader
#define TDE2_RESOURCE_FACTORY_NAME(ResourceName)			C ## ResourceName ## Factory
#define TDE2_RESOURCE_FACTORY_FUNCTION_NAME(ResourceName)	Create ## ResourceName ## Factory


#define TDE2_DECLARE_DEFAULT_RESOURCE_LOADER_IMPL(ResourceName, ResourceLoaderName, ResourceLoaderFactoryName)					\
	TDE2_API IResourceLoader* ResourceLoaderFactoryName(IResourceManager*, IFileSystem*, E_RESULT_CODE&);						\
																																\
	class ResourceLoaderName : public CBaseResourceLoader<ResourceName>															\
	{																															\
		public:																													\
			friend TDE2_API IResourceLoader* ResourceLoaderFactoryName(IResourceManager*, IFileSystem*, E_RESULT_CODE&);		\
		protected:																												\
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(ResourceLoaderName)														\
	};

#define TDE2_DECLARE_DEFAULT_RESOURCE_LOADER(ResourceType)	\
	TDE2_DECLARE_DEFAULT_RESOURCE_LOADER_IMPL(ResourceType, TDE2_RESOURCE_LOADER_NAME(ResourceType), TDE2_RESOURCE_LOADER_FUNCTION_NAME(ResourceType))


#define TDE2_DEFINE_DEFAULT_RESOURCE_LOADER_IMPL(ResourceLoaderName, ResourceLoaderFactoryName)													\
	ResourceLoaderName::ResourceLoaderName() : CBaseResourceLoader() { }																		\
																																				\
	TDE2_API IResourceLoader* ResourceLoaderFactoryName(IResourceManager* pResourceManager, IFileSystem* pFileSystem, E_RESULT_CODE& result)	\
	{																																			\
		return CREATE_IMPL(IResourceLoader, ResourceLoaderName, result, pResourceManager, pFileSystem);											\
	}

#define TDE2_DEFINE_DEFAULT_RESOURCE_LOADER(ResourceType) TDE2_DEFINE_DEFAULT_RESOURCE_LOADER_IMPL(TDE2_RESOURCE_LOADER_NAME(ResourceType), TDE2_RESOURCE_LOADER_FUNCTION_NAME(ResourceType))


#define TDE2_DECLARE_DEFAULT_RESOURCE_FACTORY_IMPL(ResourceName, ResourceFactoryName, ResourceFactoryTypeFactoryName)						\
	TDE2_API IResourceFactory* ResourceFactoryTypeFactoryName(IResourceManager*, E_RESULT_CODE&);											\
																																			\
	class ResourceFactoryName : public CBaseResourceFactory<ResourceName>																	\
	{																																		\
		public:																																\
			friend TDE2_API IResourceFactory* ResourceFactoryTypeFactoryName(IResourceManager*, E_RESULT_CODE&);							\
		public:																																\
			TDE2_API IResource* CreateDefault(const std::string& name, const TBaseResourceParameters& params) const override;				\
		protected:																															\
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(ResourceFactoryName)																	\
	};

#define TDE2_DECLARE_DEFAULT_RESOURCE_FACTORY(ResourceType)	\
	TDE2_DECLARE_DEFAULT_RESOURCE_LOADER_IMPL(ResourceType, TDE2_RESOURCE_FACTORY_NAME(ResourceType), TDE2_RESOURCE_FACTORY_FUNCTION_NAME(ResourceType))

#endif
}