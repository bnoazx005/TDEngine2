/*!
	\file IComponentFactory.h
	\date 30.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/IBaseObject.h"
#include "./../math/TVector3.h"
#include "./../math/TQuaternion.h"


namespace TDEngine2
{
	class IComponent;


	/*!
		struct TBaseComponentParameters

		\brief The structure contains parameters list for IComponent type's creation
	*/
	
	typedef struct TBaseComponentParameters
	{
	} TBaseComponentParameters, *TBaseComponentParametersPtr;


	/*!
		struct TTransformParameters

		\brief The structure contains parameters for creation of CTransform
	*/

	typedef struct TTransformParameters: public TBaseComponentParameters
	{
		TVector3    mPosition;

		TQuaternion mRotation;

		TVector3    mScale;
	} TTransformParameters;


	/*!
		interface IComponentFactory

		\brief The interface describes a factory of IComponent objects
	*/

	class IComponentFactory : public virtual IBaseObject
	{
		public:
			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API virtual IComponent* Create(const TBaseComponentParameters* pParams) const = 0;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API virtual IComponent* CreateDefault(const TBaseComponentParameters& params) const = 0;

			/*!
				\brief The method returns an identifier of a component's type, which
				the factory serves

				\return The method returns an identifier of a component's type, which
				the factory serves
			*/

			TDE2_API virtual TypeId GetComponentTypeId() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IComponentFactory)
	};


	/*!
		interface ITransformFactory

		\brief The interface represents a functionality of a factory of ITransform objects
	*/

	class ITransformFactory : public IComponentFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a factory
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;
		protected:
	};
}
