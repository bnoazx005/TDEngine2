/*!
	\file CSpriteRendererSystem.h
	\date 04.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "ISystem.h"
#include "./../core/CBaseObject.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CSpriteRendererSystem's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSpriteRendererSystem's implementation
	*/

	TDE2_API ISystem* CreateSpriteRendererSystem(E_RESULT_CODE& result);


	/*!
		class CSpriteRendererSystem

		\brief The class is a system that processes ISprite components
	*/

	class CSpriteRendererSystem : public ISystem, public CBaseObject
	{
		public:
			friend TDE2_API ISystem* CreateSpriteRendererSystem(E_RESULT_CODE& result);
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
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] pWorld A pointer to a main scene's object

				\param[in] dt A delta time's value
			*/

			TDE2_API void Update(IWorld* pWorld, F32 dt) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSpriteRendererSystem)
	};
}