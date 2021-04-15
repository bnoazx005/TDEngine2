/*!
	\file IResourceFactory.h
	\date 24.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IBaseObject.h"
#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include <string>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IResource;


	/*!
		struct TBaseResourceParameters

		\brief The struct is used to pass different sets of parameters into
		Create method of IResourceFactory and IResourceManager respectively.
		Also the struct contains common parameters for all resources
	*/

	typedef struct TBaseResourceParameters
	{
		TDE2_API virtual ~TBaseResourceParameters() = default;

		E_RESOURCE_LOADING_POLICY mLoadingPolicy = E_RESOURCE_LOADING_POLICY::SYNCED;
	} TBaseResourceParameters, *TBaseResourceParametersPtr;


	/*!
		interface IResourceFactory

		\brief The interface describes a functionality of a resource factory
	*/

	class IResourceFactory: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API virtual IResource* Create(const std::string& name, const TBaseResourceParameters& params) const = 0;

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

			TDE2_API virtual TypeId GetResourceTypeId() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IResourceFactory)
	};
}
