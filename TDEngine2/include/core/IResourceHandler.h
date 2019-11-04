/*!
	\file IResourceHandler.h
	\date 18.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "IBaseObject.h"
#include "./../utils/Utils.h"
#include "IResource.h"
#include <functional>
#include <cassert>


namespace TDEngine2
{
	class IResourceManager;

	
	/*!
		interface IResourceHandler

		\brief The interface represents a handler of a single resource. It encapsulates
		a direct access to the resource providing other mechanism. The handler can be
		used in both blocking and asynchronous manners.
	*/

	class IResourceHandler : public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an inner state of a resource's handler

				\param[in] pResourceManager A pointer to implementation of IResourceManager

				\param[in] id A resource's identifier

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, TResourceId id) = 0;

			/*!
				\brief The method sets up a new resource's identifier

				\param[in] id A new identifier's value

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetResourceId(TResourceId id) = 0;

			/*!
				\brief The method returns a raw pointer to a resource it's binded to

				\param[in] type A type of an access to a resource (can be blocking or asynchronous). Use
				RAT_BLOCKING to get result at the place of a call, or RAT_STREAMING to get high-quality
				version of the resource when it'll be fully loaded into a memory

				\return A raw pointer to a binded resource
			*/

			template <typename T>
//#if _HAS_CXX17
//			std::enable_if_t<std::is_base_of_v<IResource, T>, T*>
//#else
//			typename std::enable_if<std::is_base_of<IResource, T>::value, T*>::type
//#endif
			T* Get(E_RESOURCE_ACCESS_TYPE type)
			{
				return dynamic_cast<T*>(_getInternal(type));
			}

			/*!
				\brief The method returns an identifier of a binded resource

				\return The method returns an identifier of a binded resource
			*/

			TDE2_API virtual TResourceId GetResourceId() const = 0;

			/*!
				\brief The method executes specified callback when a binded resource is fully loaded

				\param[in] onLoadedCallback A callback, which will be executed when a resource will be fully loaded
			*/

			TDE2_API virtual void OnLoaded(const std::function<void(IResource*)>& onLoadedCallback) = 0;

			/*!
				\brief The predicate tells whether the handler stay in valid state or not

				\return The method returns true if the handler points to valid resource, false in other cases
			*/

			TDE2_API virtual bool IsValid() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IResourceHandler)

			TDE2_API virtual IResource* _getInternal(E_RESOURCE_ACCESS_TYPE type) = 0;
	};
}
