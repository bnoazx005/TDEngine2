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

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to COrthoCamera's implementation
	*/

	TDE2_API IComponent* CreateOrthoCamera(E_RESULT_CODE& result);


	/*!
		class COrthoCamera

		\brief The class represents a camera which uses an orthographic projection
	*/

	class COrthoCamera : public CBaseCamera, public IOrthoCamera, public CPoolMemoryAllocPolicy<COrthoCamera, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateOrthoCamera(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(COrthoCamera)

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


	TDE2_DECLARE_COMPONENT_FACTORY(OrthoCamera, TOrthoCameraParameters);
}
