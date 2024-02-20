/*!
	\file CSplashScreenItemComponent.h
	\date 19.02.2024
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../core/memory/CPoolAllocator.h"
#include "../../ecs/CBaseComponent.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CSplashScreenItemComponent's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IComponent's implementation
	*/

	TDE2_API IComponent* CreateSplashScreenItemComponent(E_RESULT_CODE& result);


	class CSplashScreenItemComponent : public CBaseComponent, public CPoolMemoryAllocPolicy<CSplashScreenItemComponent, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateSplashScreenItemComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CSplashScreenItemComponent)

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

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSplashScreenItemComponent)
		public:
			float mAtmosphereThickness = 4000.0f;

			TVector2 mWindDirection = RightVector2;
	};


	typedef struct TSplashScreenItemComponentParameters : public TBaseComponentParameters
	{
	} TSplashScreenItemComponentParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(SplashScreenItemComponent, TSplashScreenItemComponentParameters);
}