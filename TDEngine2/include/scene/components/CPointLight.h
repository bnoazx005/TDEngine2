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
				\brief The method initializes an internal state of the light

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init();

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


	/*!
		\brief A factory function for creation objects of CPointLightFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CPointLightFactory's implementation
	*/

	TDE2_API IComponentFactory* CreatePointLightFactory(E_RESULT_CODE& result);


	/*!
		class CPointLightFactory

		\brief The interface represents a functionality of a factory of IPointLight objects
	*/

	class CPointLightFactory : public ILightFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreatePointLightFactory(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* Create(const TBaseComponentParameters* pParams) const override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* CreateDefault(const TBaseComponentParameters& params) const override;

			/*!
				\brief The method returns an identifier of a component's type, which
				the factory serves

				\return The method returns an identifier of a component's type, which
				the factory serves
			*/

			TDE2_API TypeId GetComponentTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPointLightFactory)
	};
}
