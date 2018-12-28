/*!
	\file ICameraSystem.h
	\date 28.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "ISystem.h"


namespace TDEngine2
{	
	class IWindowSystem;
	class IGraphicsContext;
	class IPerspectiveCamera;
	class IOrthoCamera;


	/*!
		interface ICameraSystem

		\brief The interface describes a functionality of a system that tracks and
		updates cameras data
	*/

	class ICameraSystem : public ISystem
	{
		public:
			/*!
				\brief The method initializes an inner state of a system

				\param[in] pWindowSystem A pointer to IWindowSystem implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const IWindowSystem* pWindowSystem, IGraphicsContext* pGraphicsContext) = 0;

			/*!
				\brief The method computes a perspective projection matrix based on parameters of a given camera

				\param[in, out] pCamera A pointer to ICamera component

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ComputePerspectiveProjection(IPerspectiveCamera* pCamera) const = 0;

			/*!
				\brief The method computes a orthographic projection matrix based on parameters of a given camera

				\param[in, out] pCamera A pointer to ICamera component

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ComputeOrthographicProjection(IOrthoCamera* pCamera) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ICameraSystem)
	};
}