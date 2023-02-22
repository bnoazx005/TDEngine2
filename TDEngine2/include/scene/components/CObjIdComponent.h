/*!
	\file CObjIdComponent.h
	\date 22.02.2023
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include <string>


namespace TDEngine2
{
	/*!
		struct TObjIdComponentParameters
	*/

	typedef struct TObjIdComponentParameters : public TBaseComponentParameters
	{
	} TObjIdComponentParameters;


	/*!
		\brief A factory function for creation objects of CObjIdComponent's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CObjIdComponent's implementation
	*/

	TDE2_API IComponent* CreateObjIdComponent(E_RESULT_CODE& result);


	/*!
		class CObjIdComponent

		The class is used to resolve references to entities that're part of prefabs in runtime
	*/

	class CObjIdComponent : public CBaseComponent, public CPoolMemoryAllocPolicy<CObjIdComponent, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateObjIdComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CObjIdComponent)

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

			/*!
				\brief The method creates a new deep copy of the instance and returns a smart pointer to it.
				The original state of the object stays the same

				\param[in] pDestObject A valid pointer to an object which the properties will be assigned into
			*/

			TDE2_API E_RESULT_CODE Clone(IComponent*& pDestObject) const override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CObjIdComponent)
		public:
			U32 mId;
	};


	TDE2_DECLARE_COMPONENT_FACTORY(ObjIdComponent, TObjIdComponentParameters);
}