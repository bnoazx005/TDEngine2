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

		\param[in] direction A direction of the light source
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CDirectionalLight's implementation
	*/

	TDE2_API IComponent* CreateDirectionalLight(const TVector3& direction, E_RESULT_CODE& result);


	/*!
		class CDirectionalLight

		\brief The class represents directed light source which is an analogue of sun light 
	*/

	class CDirectionalLight : public CBaseLight, public IDirectionalLight
	{
		public:
			friend TDE2_API IComponent* CreateDirectionalLight(const TVector3&, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CDirectionalLight)

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
				\brief The method specifies direction of the light source

				\param[in] direction A normalized 3d vector that determines direction of the sun light

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetDirection(const TVector3& direction) override;

			/*!
				\brief The method returns a sun light's direction
				\return The method returns a sun light's direction
			*/

			TDE2_API const TVector3& GetDirection() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CDirectionalLight)
		protected:
			TVector3 mDirection;
	};


	/*!
		\brief A factory function for creation objects of CDirectionalLightFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CDirectionalLightFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateDirectionalLightFactory(E_RESULT_CODE& result);


	/*!
		class CDirectionalLightFactory

		\brief The interface represents a functionality of a factory of IDirectionalLight objects
	*/

	class CDirectionalLightFactory : public ILightFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateDirectionalLightFactory(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CDirectionalLightFactory)
	};
}
