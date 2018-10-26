/*!
	\file CBaseResource.h
	\date 17.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IResource.h"
#include "CBaseObject.h"
#include "./../utils/Utils.h"
#include "IResourceManager.h"
#include "IResourceLoader.h"


namespace TDEngine2
{
	/*!
		class CBaseResource

		\brief The class is a base class of a resource.
		All concrete types of resources should derive it.
	*/

	template <typename T>
	class CBaseResource: public IResource, public CBaseObject
	{
		public:
			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method loads resource data into memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load() override;

			/*!
				\brief The method unloads resource data from memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Unload() = 0;

			/*!
				\brief The method returns an identifier of a resource

				\return The method returns an identifier of a resource
			*/

			TDE2_API TResourceId GetId() const override;

			/*!
				\brief The method returns a resource's name

				\return The method returns a resource's name
			*/

			TDE2_API std::string GetName() const override;
			
			/*!
				\brief The method returns current state, in which a resource stays

				\return The method returns current state, in which a resource stays
			*/

			TDE2_API E_RESOURCE_STATE_TYPE GetState() const override;

			/*!
				\brief The static method returns the type's identifier

				\return The static method returns the type's identifier
			*/

			static TDE2_API U32 GetTypeId();
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseResource)

			virtual TDE2_API E_RESULT_CODE _init(IResourceManager* pResourceManager, const std::string& name, TResourceId id);
		protected:
			static U32               mTypeId;					///< The value of mTypeId is same for all resources of T type

			static const TResourceId mInvalidResourceId;

			IResourceManager*        mpResourceManager;

			std::string              mName;						///< The name's value is unique for each resource

			TResourceId              mId;						///< The id's value is unique for each resource. Used for cross-referencing

			E_RESOURCE_STATE_TYPE    mState;
	};


	template <typename T>
	CBaseResource<T>::CBaseResource():
		CBaseObject(), mId(mInvalidResourceId), mState(RST_PENDING)
	{
	}

	template <typename T>
	TDE2_API E_RESULT_CODE CBaseResource<T>::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		delete this;

		mIsInitialized = false;

		return RC_OK;
	}

	template <typename T>
	TDE2_API E_RESULT_CODE CBaseResource<T>::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		const IResourceLoader* pResourceLoader = mpResourceManager->GetResourceLoader<T>();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		return pResourceLoader->LoadResource(this);
	}
	
	template <typename T>
	TDE2_API TResourceId CBaseResource<T>::GetId() const
	{
		return mId;
	}

	template <typename T>
	TDE2_API std::string CBaseResource<T>::GetName() const
	{
		return mName;
	}

	template <typename T>
	TDE2_API E_RESOURCE_STATE_TYPE CBaseResource<T>::GetState() const
	{
		return mState;
	}

	template <typename T>
	TDE2_API U32 CBaseResource<T>::GetTypeId()
	{
		return mTypeId;
	}

	template <typename T>
	TDE2_API E_RESULT_CODE CBaseResource<T>::_init(IResourceManager* pResourceManager, const std::string& name, TResourceId id)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || name.empty() || id == InvalidResourceId)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mName = name;

		mId = id;

		mIsInitialized = true;

		return RC_OK;
	}

	template <typename T>
	const TResourceId CBaseResource<T>::mInvalidResourceId { 0 };

	template <typename T>
	U32 CBaseResource<T>::mTypeId { TTypeRegistry<IResource>::GetTypeId() }; ///< Compile-time generation of a type's id
}