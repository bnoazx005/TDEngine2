/*!
	\file CPerspectiveCamera.h
	\date 22.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "ICamera.h"
#include "CBaseCamera.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CPerspectiveCamera's type

		\param[in] fov A field of view of a camera

		\param[in] aspect A screen's aspect ratio

		\param[in] zn Z value of a near clip plane

		\param[in] zf Z value of a far clip plane

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CPerspectiveCamera's implementation
	*/

	TDE2_API IComponent* CreatePerspectiveCamera(F32 fov, F32 aspect, F32 zn, F32 zf, E_RESULT_CODE& result);


	/*!
		class CPerspectiveCamera

		\brief The class represents a camera which uses a perspective projection
	*/

	class CPerspectiveCamera : public CBaseCamera, public IPerspectiveCamera
	{
		public:
			friend TDE2_API IComponent* CreatePerspectiveCamera(F32 fov, F32 aspect, F32 zn, F32 zf, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CPerspectiveCamera)

			/*!
				\brief The method initializes an internal state of a camera

				\param[in] fov A field of view of a camera

				\param[in] aspect A screen's aspect ratio

				\param[in] zn Z value of a near clip plane

				\param[in] zf Z value of a far clip plane

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(F32 fov, F32 aspect, F32 zn, F32 zf) override;

			/*!
				\brief The method implements "visitor" pattern approach. We compute projection
				matrix based on a type of a camera, but the computations are executed by ICameraSystem

				\param[in] pCameraSystem A pointer to ICameraSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ComputeProjectionMatrix(const ICameraSystem* pCameraSystem) override;

			/*!
				\brief The method sets up a field of view of a camera

				\param[in] fov A field of view of a camera
			*/

			TDE2_API void SetFOV(F32 fov) override;

			/*!
				\brief The method sets up an aspect ratio of a camera

				\param[in] aspect A screen's aspect ratio
			*/

			TDE2_API void SetAspect(F32 aspect) override;
			
			/*!
				\brief The method returns a camera's field of view

				\return The method returns a camera's field of view
			*/

			TDE2_API F32 GetFOV() const override;

			/*!
				\brief The method returns a camera's aspect ratio

				\return The method returns a camera's aspect ratio
			*/

			TDE2_API F32 GetAspect() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPerspectiveCamera)
		protected:
			F32 mAspectRatio;

			F32 mFOV;
	};


	/*!
		\brief A factory function for creation objects of CPerspectiveCameraFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CPerspectiveCameraFactory's implementation
	*/

	TDE2_API IComponentFactory* CreatePerspectiveCameraFactory(E_RESULT_CODE& result);


	/*!
		class CPerspectiveCameraFactory

		\brief The interface represents a functionality of a factory of IPerspectiveCamera objects
	*/

	class CPerspectiveCameraFactory : public ICameraFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreatePerspectiveCameraFactory(E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPerspectiveCameraFactory)
	};
}
