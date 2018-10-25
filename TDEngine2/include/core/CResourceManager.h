/*!
	\file CResourceManager.h
	\date 18.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IResourceManager.h"
#include <unordered_map>
#include <vector>
#include <mutex>


namespace TDEngine2
{
	class CResourceHandler;


	/*!
		\brief A factory function for creation objects of CResourceManager's type.

		\param[in] pJobManager A pointer to IJobManager's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CResourceManager's implementation
	*/

	TDE2_API IResourceManager* CreateResourceManager(IJobManager* pJobManager, E_RESULT_CODE& result);


	/*!
		class CResourceManager

		\brief The class represents a root resource manager's functionality
	*/

	class CResourceManager : public IResourceManager
	{
		public:
			friend TDE2_API IResourceManager* CreateResourceManager(IJobManager* pJobManager, E_RESULT_CODE& result);
		protected:
			typedef std::unordered_map<U32, TResourceLoaderId>  TResourceLoadersMap;

			typedef std::vector<const IResourceLoader*>         TResourceLoadersArray;

			typedef std::list<U32>                              TFreeEntriesRegistry;

			typedef std::unordered_map<U32, TResourceFactoryId> TResourceFactoriesMap;

			typedef std::vector<const IResourceFactory*>        TResourceFactoriesArray;
		public:
			/*!
				\brief The method initializes an inner state of a resource manager

				\param[in] pJobManager A pointer to IJobManager's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IJobManager* pJobManager) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method registers specified resource loader within a manager

				\param[in] pResourceLoader A pointer to IResourceLoader's implementation

				\return The method returns an object, which contains a status of method's execution and
				an identifier of the registred loader
			*/

			TDE2_API TRegisterLoaderResult RegisterLoader(const IResourceLoader* pResourceLoader) override;

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

			TDE2_API TRegisterFactoryResult RegisterFactory(const IResourceFactory* pResourceFactory) override;

			/*!
				\brief The method unregisters a resource factory with the specified identifier

				\param[in] resourceFactoryId An identifier of a resource factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnregisterFactory(const TResourceFactoryId& resourceFactoryId) override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

			/*!
				\brief The method returns a raw pointer to a resource based on specified handler

				\param[in] pResourceHandler A pointer to a resource's handler

				\return The method returns a raw pointer to a resource based on specified handler
			*/

			TDE2_API IResource* GetResourceByHandler(const IResourceHandler* pResourceHandler) const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CResourceManager)

			TDE2_API IResourceHandler* _loadResource(U32 resourceTypeId, const std::string& name) override;

			TDE2_API IResourceHandler* _createResource(U32 resourceTypeId, const TBaseResourceParameters* pParams) override;
		protected:
			bool                    mIsInitialized;

			TResourceLoadersMap     mResourceLoadersMap;

			TResourceLoadersArray   mRegistredResourceLoaders; 

			TFreeEntriesRegistry    mFreeLoadersEntriesRegistry;

			TResourceFactoriesMap   mResourceFactoriesMap;

			TResourceFactoriesArray mRegistredResourceFactories;

			TFreeEntriesRegistry    mFreeFactoriesEntriesRegistry;

			IJobManager*            mpJobManager;

			mutable std::mutex      mMutex;
	};
}
