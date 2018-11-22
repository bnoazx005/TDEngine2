/*!
	\file CBaseCamera.h
	\date 22.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "ICamera.h"
#include "./../ecs/CBaseComponent.h"


namespace TDEngine2
{
	/*!
		class CBaseCamera

		\brief The class represents a foundation for all types of camera's 
		implementation in the engine. If you want to implement your custom
		camera component use this class as a basis.
	*/

	class CBaseCamera : public virtual ICamera, public CBaseComponent
	{
		public:
			TDE2_REGISTER_TYPE(CBaseCamera)

			/*!
				\brief The method sets up a position of a near clip plane along Z axis

				\param[in] zn A position of a near clip plane on Z axis
			*/

			TDE2_API void SetNearPlane(F32 zn) override;

			/*!
				\brief The method sets up a position of a far clip plane along Z axis

				\param[in] zn A position of a far clip plane on Z axis
			*/

			TDE2_API void SetFarPlane(F32 zf) override;

			/*!
				\brief The method returns a position of a near clip plane on Z axis

				\return The method returns a position of a near clip plane on Z axis
			*/

			TDE2_API F32 GetNearPlane() const override;

			/*!
				\brief The method returns a position of a far clip plane on Z axis

				\return The method returns a position of a far clip plane on Z axis
			*/

			TDE2_API F32 GetFarPlane() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseCamera)
		protected:
			F32 mZNear;

			F32 mZFar;
	};
}
