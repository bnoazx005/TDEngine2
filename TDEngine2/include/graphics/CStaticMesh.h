/*!
	\file CStaticMesh.h
	\date 10.10.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "IMesh.h"
#include "./../ecs/CBaseComponent.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CStaticMesh's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CStaticMesh's implementation
	*/

	TDE2_API IComponent* CreateStaticMesh(E_RESULT_CODE& result);


	/*!
		interface ISprite

		\brief The interface describes a functionality of a static mesh
	*/

	class CStaticMesh: public IStaticMesh, public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateStaticMesh(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CStaticMesh)

			/*!
				\brief The method initializes an internal state of a quad sprite

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CStaticMesh)
		protected:
	};


	/*!
		struct TStaticMeshParameters

		\brief The structure contains parameters for creation of CStaticMesh
	*/

	typedef struct TStaticMeshParameters : public TBaseComponentParameters
	{
	} TStaticMeshParameters;


	/*!
		\brief A factory function for creation objects of CStaticMeshFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CStaticMeshFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateStaticMeshFactory(E_RESULT_CODE& result);


	/*!
		class CStaticMeshFactory

		\brief The class is factory facility to create a new objects of CStaticMesh type
	*/

	class CStaticMeshFactory : public IStaticMeshFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateStaticMeshFactory(E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CStaticMeshFactory)
	};
}