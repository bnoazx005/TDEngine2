/*!
	\file CCameraSystem.h
	\date 28.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../math/TRay.h"
#include "CBaseSystem.h"


namespace TDEngine2
{
	class IGraphicsContext;
	class IWindowSystem;
	class ICamera;
	class CCamera;
	class CTransform;
	class CCamerasContextComponent;
	class IRenderer;


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

	class CCameraSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateCameraSystem(const IWindowSystem*, IGraphicsContext*, IRenderer*, E_RESULT_CODE&);

		public:
			struct TSystemContext
			{
				Vector<CCamera*>    mpCameras;
				Vector<CTransform*> mpTransforms;
				TEntitiesArray      mEntities;
			};
		public:
			TDE2_SYSTEM(CCameraSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in] pWindowSystem A pointer to IWindowSystem implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

				\param[in, out] pRenderer A pointer to IRenderer implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const IWindowSystem* pWindowSystem, IGraphicsContext* pGraphicsContext, IRenderer* pRenderer);

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

#if TDE2_EDITORS_ENABLED

			/*!
				\brief The method is targeted to draw debug information (textual or graphical) which is related with the given system
			*/

			TDE2_API void DebugOutput(IDebugUtility* pDebugUtility, F32 dt) const override;

#endif
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CCameraSystem)
		protected:
			TSystemContext            mCamerasContext;

			IGraphicsContext*         mpGraphicsContext = nullptr;

			CCamerasContextComponent* mpCamerasContextComponent = nullptr;

			const IWindowSystem*      mpWindowSystem = nullptr;

			IRenderer*                mpRenderer = nullptr;

#if TDE2_EDITORS_ENABLED
			IWorld*                   mpWorld = nullptr;
#endif
	};


	/*!
		\brief The function is a converter which maps normalized screen position into world space ray

		\param[in] pCamera A pointer to ICamera component implementation
		\param[in] pos A position of a point in clip space, each component lies in range of [-1;1]

		\return The function is a converter which maps normalized screen position into world space ray
	*/

	TDE2_API TRay3D NormalizedScreenPointToWorldRay(const ICamera& pCamera, const TVector2& pos);

	/*!
		\brief The function is a converter which maps world space point into screen space's one

		\param[in] pCamera A pointer to ICamera component implementation
		\param[in] pos A position of a point in world space

		\return The position in screen space, each component of the point lies in range [-1; 1] except z axis
	*/

	TDE2_API TVector3 WorldToNormalizedScreenPoint(const ICamera& pCamera, const TVector3& pos);
}