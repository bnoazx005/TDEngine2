/*!
	\file CResourceHandler.h
	\date 18.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IResourceHandler.h"
#include "CBaseObject.h"
#include <mutex>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CResourceHandler's type.

		\param[in] pResourceManager A pointer to implementation of IResourceManager

		\param[in] id A resource's identifier

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CResourceHandler's implementation
	*/

	TDE2_API IResourceHandler* CreateResourceHandler(IResourceManager* pResourceManager, TResourceId id, E_RESULT_CODE& result);


	/*!
		class IResourceHandler

		\brief The class implements a basic resource's handler within the engine
	*/

	class CResourceHandler : public CBaseObject, public IResourceHandler
	{
		public:
			friend TDE2_API IResourceHandler* CreateResourceHandler(IResourceManager* pResourceManager, TResourceId id, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an inner state of a resource's handler

				\param[in] pResourceManager A pointer to implementation of IResourceManager

				\param[in] id A resource's identifier

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, TResourceId id) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method sets up a new resource's identifier

				\param[in] id A new identifier's value

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetResourceId(TResourceId id) override;

			/*!
				\brief The method returns a raw pointer to a resource it's binded to

				\param[in] type A type of an access to a resource (can be blocking or asynchronous). Use
				RAT_BLOCKING to get result at the place of a call, or RAT_STREAMING to get high-quality
				version of the resource when it'll be fully loaded into a memory

				\return A raw pointer to a binded resource
			*/

			TDE2_API IResource* Get(E_RESOURCE_ACCESS_TYPE type) override;

			/*!
				\brief The method returns an identifier of a binded resource

				\return The method returns an identifier of a binded resource
			*/

			TDE2_API TResourceId GetResourceId() const override;

			/*!
				\brief The method executes specified callback when a binded resource is fully loaded

				\param[in] onLoadedCallback A callback, which will be executed when a resource will be fully loaded
			*/

			TDE2_API void OnLoaded(const std::function<void(IResource*)>& onLoadedCallback) override;

			/*!
				\brief The predicate tells whether the handler stay in valid state or not

				\return The method returns true if the handler points to valid resource, false in other cases
			*/

			TDE2_API bool IsValid() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CResourceHandler)
		protected:
			IResourceManager*  mpResourceManager;
			TResourceId        mResourceId;
			mutable std::mutex mMutex;
	};
}
