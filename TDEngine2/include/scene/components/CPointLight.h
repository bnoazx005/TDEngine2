/*!
	\file CPointLight.h
	\date 24.02.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "ILight.h"
#include "CBaseLight.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CPointLight's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CPointLight's implementation
	*/

	TDE2_API IComponent* CreatePointLight(E_RESULT_CODE& result);


	/*!
		class CPointLight

		\brief The class represents omni directional light source 
	*/

	class CPointLight : public CBaseLight, public IPointLight
	{
		public:
			friend TDE2_API IComponent* CreatePointLight(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CPointLight)

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPointLight)
		protected:
			F32 mRange;
	};


	TDE2_DECLARE_COMPONENT_FACTORY(PointLight, TPointLightParameters);
}
