/*!
	\file ICamera.h
	\date 03.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../ecs/IComponentFactory.h"
#include "../math/TVector3.h"
#include "../math/TMatrix4.h"
#include "../math/MathUtils.h"
#include "../core/IBaseObject.h"


namespace TDEngine2
{
	class ICameraSystem;
	class IFrustum;


	/*!
		struct TBaseCameraParameters

		\brief The structure contains parameters for creation of a base camera's component
	*/

	typedef struct TBaseCameraParameters : public TBaseComponentParameters
	{
		F32 mZNear = 0.1f;

		F32 mZFar = 1000.0f;
	} TBaseCameraParameters;


	enum class E_CAMERA_PROJECTION_TYPE : U8
	{
		ORTHOGRAPHIC,
		PERSPECTIVE
	};


	/*!
		interface ICamera

		\brief The interface describes a functionality of a camera component
	*/

	class ICamera: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method implements "visitor" pattern approach. We compute projection
				matrix based on a type of a camera, but the computations are executed by ICameraSystem

				\param[in] pCameraSystem A pointer to ICameraSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ComputeProjectionMatrix(const ICameraSystem* pCameraSystem) = 0;

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
				\brief The method specifies a projection matrix for a camera

				\param[in] projMatrix A projection matrix
			*/

			TDE2_API virtual void SetProjMatrix(const TMatrix4& projMatrix) = 0;

			/*!
				\brief The method specifies a view matrix for a camera

				\param[in] viewMatrix A view matrix
			*/

			TDE2_API virtual void SetViewMatrix(const TMatrix4& viewMatrix) = 0;

			TDE2_API virtual void SetPosition(const TVector3& position) = 0;

			/*!
				\brief The method specifies a view-projection matrix for a camera

				\param[in] viewProjMatrix A view-projection matrix
				\param[in] zNDCMin A minimal value of Z axis of camera frustum within NDC space
			*/

			TDE2_API virtual void SetViewProjMatrix(const TMatrix4& viewProjMatrix, F32 zNDCMin) = 0;

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
			
			/*!
				\brief The method returns a projection matrix of a camera

				\return The method returns a projection matrix of a camera
			*/

			TDE2_API virtual const TMatrix4& GetProjMatrix() const = 0;

			/*!
				\brief The method returns a view matrix of a camera

				\return The method returns a view matrix of a camera
			*/

			TDE2_API virtual const TMatrix4& GetViewMatrix() const = 0;

			/*!
				\brief The method returns a view-projection matrix of a camera

				\return The method returns a view-projection matrix of a camera
			*/

			TDE2_API virtual const TMatrix4& GetViewProjMatrix() const = 0;

			/*!
				\brief The method returns an inverted view-projection matrix of a camera

				\return The method returns an inverted view-projection matrix of a camera
			*/

			TDE2_API virtual const TMatrix4& GetInverseViewProjMatrix() const = 0;

			TDE2_API virtual const TVector3& GetPosition() const = 0;

			/*!
				\brief The method returns a type of camera's projection matrix

				\return The method returns a type of camera's projection matrix
			*/

			TDE2_API virtual E_CAMERA_PROJECTION_TYPE GetProjType() const = 0;

			/*!
				\brief The method returns a camera's actual frustum

				\return The method returns pointer to camera's actual frustum implementation
			*/

			TDE2_API virtual IFrustum* GetFrustum() const = 0;
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
		F32 mFOV = CMathConstants::Pi * 0.5f;

		F32 mAspect = 1.0f;
	} TPerspectiveCameraParameters;


	/*!
		interface IPerspectiveCamera

		\brief The interface describes a functionality of a perspective camera component
	*/

	class IPerspectiveCamera : public virtual ICamera
	{
		public:
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


	class ICameraFactory: public IGenericComponentFactory<> {};


	struct TAABB;


	/*!
		interface IFrustum

		\brief The interface represents functionality of a camera's view frustum
	*/

	class IFrustum : public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an internal state of a frustum

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method computes planes of the frustum based on camera's parameters

				\param[in] invViewProj Inverted view-projection matrix of the camera
				\param[in] zMin A minimal value for z component of frustum cube vertices in NDC space (either -1 or 0)

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ComputeBounds(const TMatrix4& invViewProj, F32 zMin) = 0;

			/*!
				\brief The method tests given point for inclusion within the frustum

				\param[in] point A point in world space

				\return The method returns true if point lies within frustum's bounds, false in other cases
			*/

			TDE2_API virtual bool TestPoint(const TVector3& point) const = 0;

			/*!
				\brief The method tests whether a given sphere stays within frustum's volume or not

				\param[in] center A 3d position of sphere's center
				\param[in] radius Positive value that defines radius of the sphere

				\return The method returns true if the frustum contains the sphere, false in other cases
			*/

			TDE2_API virtual bool TestSphere(const TVector3& center, F32 radius) const = 0;

			/*!
				\brief The method tests whether a given axis-aligned bounding box is in frustum's volume or not

				\param[in] box A type that defines parameters of the AABB

				\return The method returns true if AABB lies within the frustum's bounds at least partially
			*/

			TDE2_API virtual bool TestAABB(const TAABB& box) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IFrustum);
	};


	typedef struct TCamerasContextParameters : public TBaseComponentParameters
	{
		TEntityId mActiveCameraEntityId;
	} TCamerasContextParameters, *TCamerasContextParametersPtr;
}
