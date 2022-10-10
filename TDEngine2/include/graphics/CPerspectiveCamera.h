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
		
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CPerspectiveCamera's implementation
	*/

	TDE2_API IComponent* CreatePerspectiveCamera(E_RESULT_CODE& result);


	/*!
		class CPerspectiveCamera

		\brief The class represents a camera which uses a perspective projection
	*/

	class CPerspectiveCamera : public CBaseCamera, public IPerspectiveCamera, public CPoolMemoryAllocPolicy<CPerspectiveCamera, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreatePerspectiveCamera(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CPerspectiveCamera)

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

			/*!
				\brief The method creates a new deep copy of the instance and returns a smart pointer to it.
				The original state of the object stays the same

				\param[in] pDestObject A valid pointer to an object which the properties will be assigned into
			*/

			TDE2_API E_RESULT_CODE Clone(IComponent*& pDestObject) const override;

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

			/*!
				\brief The method returns a type of camera's projection matrix

				\return The method returns a type of camera's projection matrix
			*/

			TDE2_API E_CAMERA_PROJECTION_TYPE GetProjType() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPerspectiveCamera)
		protected:
			F32 mAspectRatio;

			F32 mFOV;
	};


	TDE2_DECLARE_COMPONENT_FACTORY(PerspectiveCamera, TPerspectiveCameraParameters);
}
