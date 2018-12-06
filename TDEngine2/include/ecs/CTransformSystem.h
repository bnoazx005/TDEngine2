/*!
	\file CTransformSystem.h
	\date 04.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "ISystem.h"
#include "./../core/CBaseObject.h"
#include <vector>


namespace TDEngine2
{
	class CTransform;


	/*!
		\brief A factory function for creation objects of CTransformSystem's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTransformSystem's implementation
	*/

	TDE2_API ISystem* CreateTransformSystem(E_RESULT_CODE& result);


	/*!
		class CTransformSystem

		\brief The class is a system that processes ITransform components
	*/

	class CTransformSystem: public ISystem, public CBaseObject
	{
		public:
			friend TDE2_API ISystem* CreateTransformSystem(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an inner state of a system
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init();

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method inject components array into a system

				\param[in] pWorld A pointer to a main scene's object
			*/

			TDE2_API void InjectBindings(IWorld* pWorld) override;

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] pWorld A pointer to a main scene's object

				\param[in] dt A delta time's value
			*/

			TDE2_API void Update(IWorld* pWorld, F32 dt) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTransformSystem)
		protected:
			std::vector<CTransform*> mTransforms;
	};
}