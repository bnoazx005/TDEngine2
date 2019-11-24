/*!
	\file COrthoCamera.h
	\date 22.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "ICamera.h"
#include "CBaseCamera.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of COrthoCamera's type

		\param[in] width A camera's width

		\param[in] height A camera's height

		\param[in] zn Z value of a near clip plane

		\param[in] zf Z value of a far clip plane

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to COrthoCamera's implementation
	*/

	TDE2_API IComponent* CreateOrthoCamera(F32 width, F32 height, F32 zn, F32 zf, E_RESULT_CODE& result);


	/*!
		class COrthoCamera

		\brief The class represents a camera which uses an orthographic projection
	*/

	class COrthoCamera : public CBaseCamera, public IOrthoCamera
	{
		public:
			friend TDE2_API IComponent* CreateOrthoCamera(F32 width, F32 height, F32 zn, F32 zf, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(COrthoCamera)

			/*!
				\brief The method initializes an internal state of a camera

				\param[in] width A camera's width

				\param[in] height A camera's height

				\param[in] zn Z value of a near clip plane

				\param[in] zf Z value of a far clip plane

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(F32 width, F32 height, F32 zn, F32 zf) override;

			/*!
				\brief The method implements "visitor" pattern approach. We compute projection
				matrix based on a type of a camera, but the computations are executed by ICameraSystem

				\param[in] pCameraSystem A pointer to ICameraSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ComputeProjectionMatrix(const ICameraSystem* pCameraSystem) override;

			/*!
				\brief The method specifies a camera viewport's width

				\param[in] width A camera's width
			*/

			TDE2_API void SetWidth(F32 width) override;

			/*!
				\brief The method specifies a camera viewport's height

				\param[in] height A camera's height
			*/

			TDE2_API void SetHeight(F32 height) override;

			/*!
				\brief The method returns a camera viewport's width

				\return The method returns a camera viewport's width
			*/

			TDE2_API F32 GetWidth() const override;

			/*!
				\brief The method returns a camera viewport's height

				\return The method returns a camera viewport's height
			*/

			TDE2_API F32 GetHeight() const override;

			/*!
				\brief The method returns a type of camera's projection matrix

				\return The method returns a type of camera's projection matrix
			*/

			TDE2_API E_CAMERA_PROJECTION_TYPE GetProjType() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COrthoCamera)
		protected:
			F32 mWidth;

			F32 mHeight;
	};


	/*!
		\brief A factory function for creation objects of COrthoCameraFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to COrthoCameraFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateOrthoCameraFactory(E_RESULT_CODE& result);


	/*!
		class COrthoCameraFactory

		\brief The interface represents a functionality of a factory of IOrthoCamera objects
	*/

	class COrthoCameraFactory : public ICameraFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateOrthoCameraFactory(E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COrthoCameraFactory)
	};
}
