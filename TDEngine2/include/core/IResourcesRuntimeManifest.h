/*!
	\file IResourcesRuntimeManifest.h
	\date 12.07.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "IBaseObject.h"
#include "Serialization.h"
#include <memory>


namespace TDEngine2
{
	struct TBaseResourceParameters;


	/*!
		interface IResourcesRuntimeManifest
		
		\brief The interfaces describes functionality of a runtime resources manifest. It's used to configure
		runtime parameters of resources
	*/

	class IResourcesRuntimeManifest : public virtual IBaseObject, public ISerializable
	{
		public:
			/*!
				\brief The method initializes internal state of the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			TDE2_API virtual E_RESULT_CODE AddResourceMeta(const std::string& resourceId, std::unique_ptr<TBaseResourceParameters> pResourceMeta) = 0;

			/*!
				\return The method returns true if there is configuration for the given resource's identifier
			*/

			TDE2_API virtual bool HasResourceMeta(const std::string& resourceId) const = 0;

			TDE2_API virtual const TBaseResourceParameters* const GetResourceMeta(const std::string& resourceId) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IResourcesRuntimeManifest)
	};
}