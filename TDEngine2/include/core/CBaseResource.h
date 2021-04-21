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
#include <mutex>


namespace TDEngine2
{
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

			/*!
				\return The method returns currently assigned policy for the resource
			*/

			TDE2_API E_RESOURCE_LOADING_POLICY GetLoadingPolicy() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseResource)

			virtual TDE2_API E_RESULT_CODE _init(IResourceManager* pResourceManager, const std::string& name);

			TDE2_API virtual const IResourceLoader* _getResourceLoader() = 0;
		protected:
			IResourceManager*                  mpResourceManager;

			std::string                        mName;					///< The name's value is unique for each resource

			std::atomic<TResourceId>           mId;						///< The id's value is unique for each resource. Used for cross-referencing

			std::atomic<E_RESOURCE_STATE_TYPE> mState;

			E_RESOURCE_LOADING_POLICY          mLoadingPolicy;

			mutable std::mutex                 mMutex;
	};
}