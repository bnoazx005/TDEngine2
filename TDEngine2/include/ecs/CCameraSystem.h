/*!
	\file CCameraSystem.h
	\date 28.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../core/CBaseObject.h"
#include "ICameraSystem.h"


namespace TDEngine2
{
	class IGraphicsContext;
	class IWindowSystem;
	class CBaseCamera;
	class CTransform;


	/*!
		\brief A factory function for creation objects of CCameraSystem's type.

		\param[in] pWindowSystem A pointer to IWindowSystem implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

		\param[in, out] pRenderer A pointer to IRenderer implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CCameraSystem's implementation
	*/

	TDE2_API ISystem* CreateCameraSystem(const IWindowSystem* pWindowSystem, IGraphicsContext* pGraphicsContext,
										 IRenderer* pRenderer, E_RESULT_CODE& result);


	/*!
		class CCameraSystem

		\brief The class represents a system that updates cameras data
	*/

	class CCameraSystem : public CBaseObject, public ICameraSystem
	{
		public:
			friend TDE2_API ISystem* CreateCameraSystem(const IWindowSystem* pWindowSystem, IGraphicsContext* pGraphicsContext, 
														IRenderer* pRenderer, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an inner state of a system

				\param[in] pWindowSystem A pointer to IWindowSystem implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

				\param[in, out] pRenderer A pointer to IRenderer implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(const IWindowSystem* pWindowSystem, IGraphicsContext* pGraphicsContext, IRenderer* pRenderer) override;

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

			/*!
				\brief The method computes a perspective projection matrix based on parameters of a given camera

				\param[in, out] pCamera A pointer to ICamera component

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ComputePerspectiveProjection(IPerspectiveCamera* pCamera) const override;

			/*!
				\brief The method computes a orthographic projection matrix based on parameters of a given camera

				\param[in, out] pCamera A pointer to ICamera component

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ComputeOrthographicProjection(IOrthoCamera* pCamera) const override;
			
			/*!
				\brief The method sets up a main camera from which the scene will be rendered

				\param[in] pCamera A pointer to ICamera component

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetMainCamera(const ICamera* pCamera) override;

			/*!
				\brief The method returns a pointer to a main camera from which the scene is rendered

				\brief The method returns a pointer to a main camera from which the scene is rendered
			*/

			TDE2_API const ICamera* GetMainCamera() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CCameraSystem)
		protected:
			std::vector<CTransform*>  mCamerasTransforms;
									  
			std::vector<CBaseCamera*> mCameras;

			IGraphicsContext*         mpGraphicsContext;

			const IWindowSystem*      mpWindowSystem;

			IRenderer*                mpRenderer;

			const ICamera*            mpMainCamera;
	};
}