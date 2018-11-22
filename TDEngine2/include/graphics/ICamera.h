/*!
	\file ICamera.h
	\date 03.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../ecs/IComponentFactory.h"


namespace TDEngine2
{
	/*!
		struct TBaseCameraParameters

		\brief The structure contains parameters for creation of a base camera's component
	*/

	typedef struct TBaseCameraParameters : public TBaseComponentParameters
	{
		F32 mZNear;

		F32 mZFar;
	} TBaseCameraParameters;


	/*!
		interface ICamera

		\brief The interface describes a functionality of a camera component
	*/

	class ICamera
	{
		public:
			/*!
				\brief The method sets up a position of a near clip plane along Z axis

				\param[in] zn A position of a near clip plane on Z axis
			*/

			TDE2_API virtual void SetNearPlane(F32 zn) = 0;

			/*!
				\brief The method sets up a position of a far clip plane along Z axis

				\param[in] zn A position of a far clip plane on Z axis
			*/

			TDE2_API virtual void SetFarPlane(F32 zf) = 0;

			/*!
				\brief The method returns a position of a near clip plane on Z axis

				\return The method returns a position of a near clip plane on Z axis
			*/

			TDE2_API virtual F32 GetNearPlane() const = 0;

			/*!
				\brief The method returns a position of a far clip plane on Z axis

				\return The method returns a position of a far clip plane on Z axis
			*/

			TDE2_API virtual F32 GetFarPlane() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ICamera)
	};


	/*!
		struct TOrthoCameraParameters

		\brief The structure contains parameters for creation of an orthographic camera's component
	*/

	typedef struct TOrthoCameraParameters : public TBaseCameraParameters
	{
		F32 mViewportWidth;

		F32 mViewportHeight;
	} TOrthoCameraParameters;


	/*!
		interface IOrthoCamera

		\brief The interface describes a functionality of an orthographic camera component
	*/

	class IOrthoCamera: public virtual ICamera
	{
		public:
			/*!
				\brief The method initializes an internal state of a camera

				\param[in] width A camera's width

				\param[in] height A camera's height

				\param[in] zn Z value of a near clip plane

				\param[in] zf Z value of a far clip plane

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(F32 width, F32 height, F32 zn, F32 zf) = 0;

			/*!
				\brief The method specifies a camera viewport's width
				
				\param[in] width A camera's width
			*/

			TDE2_API virtual void SetWidth(F32 width) = 0;

			/*!
				\brief The method specifies a camera viewport's height

				\param[in] height A camera's height
			*/

			TDE2_API virtual void SetHeight(F32 height) = 0;

			/*!
				\brief The method returns a camera viewport's width

				\return The method returns a camera viewport's width
			*/

			TDE2_API virtual F32 GetWidth() const = 0;

			/*!
				\brief The method returns a camera viewport's height

				\return The method returns a camera viewport's height
			*/

			TDE2_API virtual F32 GetHeight() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IOrthoCamera)
	};


	/*!
		struct TPerspectiveCameraParameters

		\brief The structure contains parameters for creation of a perspective camera's component
	*/

	typedef struct TPerspectiveCameraParameters : public TBaseCameraParameters
	{
		F32 mFOV;

		F32 mAspect;
	} TPerspectiveCameraParameters;


	/*!
		interface IPerspectiveCamera

		\brief The interface describes a functionality of a perspective camera component
	*/

	class IPerspectiveCamera : public virtual ICamera
	{
		public:
			/*!
				\brief The method initializes an internal state of a camera

				\param[in] fov A field of view of a camera

				\param[in] aspect A screen's aspect ratio

				\param[in] zn Z value of a near clip plane

				\param[in] zf Z value of a far clip plane

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(F32 fov, F32 aspect, F32 zn, F32 zf) = 0;

			/*!
				\brief The method sets up a field of view of a camera

				\param[in] fov A field of view of a camera
			*/

			TDE2_API virtual void SetFOV(F32 fov) = 0;

			/*!
				\brief The method sets up an aspect ratio of a camera

				\param[in] aspect A screen's aspect ratio
			*/

			TDE2_API virtual void SetAspect(F32 aspect) = 0;

			/*!
				\brief The method returns a camera's field of view

				\return The method returns a camera's field of view
			*/

			TDE2_API virtual F32 GetFOV() const = 0;

			/*!
				\brief The method returns a camera's aspect ratio

				\return The method returns a camera's aspect ratio
			*/

			TDE2_API virtual F32 GetAspect() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPerspectiveCamera)
	};


	/*!
		interface ICameraFactory

		\brief The interface represents a functionality of a factory of ICamera objects
	*/

	class ICameraFactory : public IComponentFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ICameraFactory)
	};
}
