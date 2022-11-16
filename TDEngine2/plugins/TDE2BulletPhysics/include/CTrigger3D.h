/*!
	\file CTrigger3D.h
	\date 07.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include <core/CBaseObject.h>
#include <ecs/CBaseComponent.h>
#include <physics/3D/ITrigger3D.h>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CTrigger3D's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTrigger3D's implementation
	*/

	TDE2_API IComponent* CreateTrigger3D(E_RESULT_CODE& result);


	/*!
		class CTrigger3D

		\brief The class implements a functionality of a 3D trigger
	*/

	class CTrigger3D : public ITrigger3D, public CBaseComponent, public CPoolMemoryAllocPolicy<CTrigger3D, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateTrigger3D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CTrigger3D)

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTrigger3D)
	};


	/*!
		struct TTrigger3DParameters

		\brief The structure contains parameters for creation of CTrigger3D
	*/

	typedef struct TTrigger3DParameters : public TBaseComponentParameters
	{
	} TTrigger3DParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(Trigger3D, TTrigger3DParameters);
}
