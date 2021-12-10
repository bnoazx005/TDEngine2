/*!
	\file CDirectionalLight.h
	\date 05.05.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "ILight.h"
#include "CBaseLight.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CDirectionalLight's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CDirectionalLight's implementation
	*/

	TDE2_API IComponent* CreateDirectionalLight(E_RESULT_CODE& result);


	/*!
		class CDirectionalLight

		\brief The class represents directed light source which is an analogue of sun light 
	*/

	class CDirectionalLight : public CBaseLight, public IDirectionalLight
	{
		public:
			friend TDE2_API IComponent* CreateDirectionalLight(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CDirectionalLight)

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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CDirectionalLight)
	};


	TDE2_DECLARE_COMPONENT_FACTORY(DirectionalLight, TDirectionalLightParameters);
}
