/*!
	\file CPrefabLinkInfoComponent.h
	\date 30.05.2023
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../core/memory/CPoolAllocator.h"
#include "../../ecs/CBaseComponent.h"
#include <vector>


namespace TDEngine2
{
	class CPrefabChangesList;
	TDE2_DECLARE_SCOPED_PTR(CPrefabChangesList)


	/*!
		\brief A factory function for creation objects of CPrefabLinkInfoComponent's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IComponent's implementation
	*/

	TDE2_API IComponent* CreatePrefabLinkInfoComponent(E_RESULT_CODE& result);


	class CPrefabLinkInfoComponent : public CBaseComponent, public CPoolMemoryAllocPolicy<CPrefabLinkInfoComponent, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreatePrefabLinkInfoComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CPrefabLinkInfoComponent)

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

			TDE2_API void SetPrefabLinkId(const std::string& id);

			TDE2_API const std::string& GetPrefabLinkId() const;

			TDE2_API TPtr<CPrefabChangesList> GetPrefabsChangesList() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPrefabLinkInfoComponent)
		protected:
			TPtr<CPrefabChangesList> mpChangesList;

			std::string mPrefabLinkId;
	};


	typedef struct TPrefabLinkInfoComponentParameters : public TBaseComponentParameters
	{
		std::string mPrefabLinkId;
	} TPrefabLinkInfoComponentParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(PrefabLinkInfoComponent, TPrefabLinkInfoComponentParameters);
}