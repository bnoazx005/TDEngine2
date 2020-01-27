/*!
	\file CEditorCameraControlSystem.h
	\date 17.01.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "./../../utils/Types.h"
#include "./../../ecs/CBaseSystem.h"
#include "./../../math/TVector3.h"
#include <vector>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class CTransform;
	class IInputContext;
	class IDesktopInputContext;
	class IEditorsManager;


	/*!
		\brief A factory function for creation objects of CEditorCameraControlSystem's type.

		\param[in, out] pInputContext A pointer to IInputContext implementation
		\param[in, out] pEditorManager A pointer to IEditorManager implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CEditorCameraControlSystem's implementation
	*/

	TDE2_API ISystem* CreateEditorCameraControlSystem(IInputContext* pInputContext, IEditorsManager* pEditorManager, E_RESULT_CODE& result);


	/*!
		class CEditorCameraControlSystem

		\brief The class implements a system that controls a camera when level editor's mode is enabled
	*/

	class CEditorCameraControlSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateEditorCameraControlSystem(IInputContext* pInputContext, IEditorsManager* pEditorManager, E_RESULT_CODE& result);
		public:
			typedef std::vector<TEntityId> TCameraEntitiesArray;
		public:
			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pInputContext A pointer to IInputContext implementation
				\param[in, out] pEditorManager A pointer to IEditorManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IInputContext* pInputContext, IEditorsManager* pEditorManager);

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CEditorCameraControlSystem)

			TDE2_API void _processCameraRotation(IDesktopInputContext& inputContext, CTransform& currTransform);
		protected:
			TCameraEntitiesArray  mCameras;

			IDesktopInputContext* mpInputContext;

			IEditorsManager*      mpEditorManager;

			TVector3              mLastClickedPosition;
			TVector3              mCurrRotation;
			TVector3              mCurrDeltaRotation;
	};
}

#endif
