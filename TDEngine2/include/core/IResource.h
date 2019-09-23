/*!
	\file IResource.h
	\date 17.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "IBaseObject.h"
#include <string>


namespace TDEngine2
{
	class IResourceManager;


	/*!
		interface IResource

		\brief The interface describes a functionality of a in-engine resource
		This term includes the resources such as textures, shaders, audio clips and
		etc. All of them should implement this one's methods.
	*/

	class IResource : public virtual IBaseObject
	{
		public:
			/*!
				\brief The method loads resource data into memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Load() = 0;

			/*!
				\brief The method unloads resource data from memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Unload() = 0;

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Reset() = 0;

			/*!
				\brief The method returns an identifier of a resource

				\return The method returns an identifier of a resource
			*/

			TDE2_API virtual TResourceId GetId() const = 0;

			/*!
				\brief The method returns a resource's name

				\return The method returns a resource's name
			*/

			TDE2_API virtual const std::string& GetName() const = 0;

			/*!
				\brief The method returns current state, in which a resource stays

				\return The method returns current state, in which a resource stays
			*/

			TDE2_API virtual E_RESOURCE_STATE_TYPE GetState() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IResource)
	};
}