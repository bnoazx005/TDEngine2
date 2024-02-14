/*!
	\file CVideoProcessSystem.h
	\date 14.02.2024
	\authors Kasimov Ildar
*/

#pragma once


#include <utils/Types.h>
#include <utils/Utils.h>
#include <ecs/CBaseSystem.h>
#include <ecs/IWorld.h>
#include <vector>


namespace TDEngine2
{
	class CUIVideoContainerComponent;
	class CImage;


	/*!
		\brief A factory function for creation objects of CVideoProcessSystem's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CVideoProcessSystem's implementation
	*/

	TDE2_API ISystem* CreateVideoProcessSystem(E_RESULT_CODE& result);


	/*!
		class CVideoProcessSystem
	*/

	class CVideoProcessSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateVideoProcessSystem(E_RESULT_CODE&);
		public:
			TDE2_SYSTEM(CVideoProcessSystem);

			/*!
				\brief The method initializes an inner state of a system

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init();

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVideoProcessSystem)

		protected:
			std::vector<CUIVideoContainerComponent*> mVideoContainers;
			std::vector<CImage*>                     mVideoReceivers;
	};
}