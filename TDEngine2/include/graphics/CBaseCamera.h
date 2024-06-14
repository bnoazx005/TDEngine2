/*!
	\file CBaseCamera.h
	\date 22.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "ICamera.h"
#include "../ecs/CBaseComponent.h"
#include "../math/TMatrix4.h"
#include "../math/TPlane.h"
#include <array>


namespace TDEngine2
{
	class IWorld;

	TDE2_DECLARE_SCOPED_PTR(IWorld);


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
				\brief The method specifies a projection matrix for a camera

				\param[in] projMatrix A projection matrix
			*/

			TDE2_API void SetProjMatrix(const TMatrix4& projMatrix) override;

			/*!
				\brief The method specifies a view matrix for a camera

				\param[in] viewMatrix A view matrix
			*/

			TDE2_API void SetViewMatrix(const TMatrix4& viewMatrix) override;

			/*!
				\brief The method specifies a view-projection matrix for a camera

				\param[in] viewProjMatrix A view-projection matrix
				\param[in] zNDCMin A minimal value of Z axis of camera frustum within NDC space
			*/

			TDE2_API void SetViewProjMatrix(const TMatrix4& viewProjMatrix, F32 zNDCMin) override;

			TDE2_API void SetPosition(const TVector3& position) override;

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

			/*!
				\brief The method returns a projection matrix of a camera

				\return The method returns a projection matrix of a camera
			*/

			TDE2_API const TMatrix4& GetProjMatrix() const override;

			/*!
				\brief The method returns a view matrix of a camera

				\return The method returns a view matrix of a camera
			*/

			TDE2_API const TMatrix4& GetViewMatrix() const override;

			/*!
				\brief The method returns a view-projection matrix of a camera

				\return The method returns a view-projection matrix of a camera
			*/

			TDE2_API const TMatrix4& GetViewProjMatrix() const override;

			/*!
				\brief The method returns an inverted view-projection matrix of a camera

				\return The method returns an inverted view-projection matrix of a camera
			*/

			TDE2_API const TMatrix4& GetInverseViewProjMatrix() const override;

			TDE2_API const TVector3& GetPosition() const override;

			/*!
				\brief The method returns a camera's actual frustum

				\return The method returns pointer to camera's actual frustum implementation
			*/

			TDE2_API IFrustum* GetFrustum() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseCamera)

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			F32       mZNear;

			F32       mZFar;
			
			TVector3  mPosition;

			TMatrix4  mProjMatrix;

			TMatrix4  mViewMatrix;

			TMatrix4  mViewProjMatrix;

			TMatrix4  mInvViewProjMatrix;

			IFrustum* mpCameraFrustum;
	};


	/*!
		\brief A factory function for creation objects of CFrustum's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFrustum's implementation
	*/

	TDE2_API IFrustum* CreateFrustum(E_RESULT_CODE& result);


	/*!
		class CFrustum

		\brief The class implements functionality of camera's frustum
	*/

	class CFrustum : public CBaseObject, public IFrustum
	{
		public:
			friend TDE2_API IFrustum* CreateFrustum(E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of a frustum

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method computes planes of the frustum based on camera's parameters

				\param[in] invViewProj Inverted view-projection matrix of the camera
				\param[in] zMin A minimal value for z component of frustum cube vertices in NDC space (either -1 or 0)

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ComputeBounds(const TMatrix4& invViewProj, F32 zMin) override;

			/*!
				\brief The method computes vertices of the frustum

				\param[in] invViewProj Inverted view-projection matrix of the camera
				\param[in] zMin A minimal value for z component of frustum cube vertices in NDC space (either -1 or 0)

				\return The first 4 vertices form near plane. The order is clockwise starting from left top corner
			*/

			TDE2_API std::array<TVector4, 8> GetVertices(const TMatrix4& invViewProj, F32 zMin) const override;

			/*!
				\brief The method tests given point for inclusion within the frustum

				\param[in] point A point in world space

				\return The method returns true if point lies within frustum's bounds, false in other cases
			*/

			TDE2_API bool TestPoint(const TVector3& point) const override;
			
			/*!
				\brief The method tests whether a given sphere stays within frustum's volume or not

				\param[in] center A 3d position of sphere's center
				\param[in] radius Positive value that defines radius of the sphere

				\return The method returns true if the frustum contains the sphere, false in other cases
			*/

			TDE2_API bool TestSphere(const TVector3& center, F32 radius) const override;

			/*!
				\brief The method tests whether a given axis-aligned bounding box is in frustum's volume or not

				\param[in] box A type that defines parameters of the AABB

				\return The method returns true if AABB lies within the frustum's bounds at least partially
			*/

			TDE2_API bool TestAABB(const TAABB& box) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFrustum)
		protected:
			std::array<TPlaneF32, 6> mPlanes {};
	};


	/*!
		\brief A factory function for creation objects of CFrustum's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFrustum's implementation
	*/

	TDE2_API class CCamerasContextComponent* CreateCamerasContextComponent(E_RESULT_CODE& result);


	/*!
		class CCamerasContextComponent

		\brief The class is a storage for all the data that's relative with game cameras including editor one.
		The component is a signleton and the world contains the only one instance of it
	*/

	class CCamerasContextComponent : public CBaseComponent
	{
		public:
			friend TDE2_API CCamerasContextComponent* CreateCamerasContextComponent(E_RESULT_CODE&);
			public:
				TDE2_REGISTER_COMPONENT_TYPE(CCamerasContextComponent)

				TDE2_API void SetActiveCameraEntity(TEntityId entityId);
				TDE2_API E_RESULT_CODE RestorePreviousCameraEntity();

				TDE2_API TEntityId GetActiveCameraEntityId() const;
			protected:
				DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CCamerasContextComponent)
			private:
				TEntityId mActiveCameraEntityId = TEntityId::Invalid;
				TEntityId mPrevCameraEntityId = TEntityId::Invalid;
	};


	TDE2_DECLARE_COMPONENT_FACTORY(CamerasContextComponent, TCamerasContextParameters);


#if TDE2_EDITORS_ENABLED
	TDE2_DECLARE_FLAG_COMPONENT(EditorCamera)
#endif


	TDE2_API E_RESULT_CODE SetActiveCamera(TPtr<IWorld> pWorld, TEntityId cameraEntityId);
}
