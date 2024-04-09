/*!
	\file CSpotLight.h
	\date 23.03.2024
	\authors Kasimov Ildar
*/

#pragma once


#include "ILight.h"
#include "CBaseLight.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CSpotLight's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A Spoter to CSpotLight's implementation
	*/

	TDE2_API IComponent* CreateSpotLight(E_RESULT_CODE& result);


	/*!
		class CSpotLight 
	*/

	class CSpotLight : public CBaseLight, public ISpotLight, public CPoolMemoryAllocPolicy<CSpotLight, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateSpotLight(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CSpotLight)

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
				\brief The method creates a new deep copy of the instance and returns a smart Spoter to it.
				The original state of the object stays the same

				\param[in] pDestObject A valid Spoter to an object which the properties will be assigned into
			*/

			TDE2_API E_RESULT_CODE Clone(IComponent*& pDestObject) const override;
			
			/*!
				\brief The method specifies an angle of a spotlight

				\param[in] angle value of cone's angle in degrees

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetAngle(F32 value) override;

			/*!
				\brief The method returns an angle of a cone of the Spot light
			*/

			TDE2_API F32 GetAngle() const override;

			/*!
				\brief The method specifies radius of the point light

				\param[in] range A maximum distance at which the light affects to objects

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetRange(F32 range) override;

			/*!
				\brief The method returns a radius of the point light
				\return The method returns a radius of the point light
			*/

			TDE2_API F32 GetRange() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSpotLight)
		protected:
			F32 mConeAngle = 0.0f;
			F32 mRange = 1.0f;
	};


	TDE2_DECLARE_COMPONENT_FACTORY(SpotLight, TSpotLightParameters);
}
