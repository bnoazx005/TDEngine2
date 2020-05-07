/*!
	\file CLightingSystem.h
	\date 07.05.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include <vector>


namespace TDEngine2
{
	class IRenderer;


	/*!
		\brief A factory function for creation objects of CLightingSystem's type.

		\param[in, out] pRenderer A pointer to IRenderer implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CLightingSystem's implementation
	*/

	TDE2_API ISystem* CreateLightingSystem(IRenderer* pRenderer, E_RESULT_CODE& result);


	/*!
		class CLightingSystem

		\brief The class is a system that processes ILighting components
	*/

	class CLightingSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateLightingSystem(IRenderer*, E_RESULT_CODE& result);
		public:
			TDE2_SYSTEM(CLightingSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pRenderer A pointer to IRenderer implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IRenderer* pRenderer);

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLightingSystem)
		protected:
			IRenderer*             mpRenderer;

			std::vector<TEntityId> mDirectionalLightsEntities;
	};
}