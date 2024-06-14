/*!
	\file CWeatherSystem.h
	\date 24.12.2023
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include "../ecs/IWorld.h"


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IJobManager;

	TDE2_DECLARE_SCOPED_PTR(IResourceManager)
	TDE2_DECLARE_SCOPED_PTR(IJobManager)
	TDE2_DECLARE_SCOPED_PTR(IGraphicsContext)


	struct TWeatherSystemInitParams
	{
		IResourceManager* mpResourceManager = nullptr;
		IGraphicsContext* mpGraphicsContext = nullptr;
		IJobManager*      mpJobManager = nullptr;
	};


	/*!
		\brief A factory function for creation objects of CWeatherSystem's type.

		\param[in] params An input parameters for the system
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CWeatherSystem's implementation
	*/

	TDE2_API ISystem* CreateWeatherSystem(const TWeatherSystemInitParams& params, E_RESULT_CODE& result);


	class CWeatherSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateWeatherSystem(const TWeatherSystemInitParams&, E_RESULT_CODE& result);
		public:
			TDE2_SYSTEM(CWeatherSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in] params An input parameters for the system
				\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(const TWeatherSystemInitParams& params);

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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CWeatherSystem)
		protected:
			IGraphicsContext* mpGraphicsContext = nullptr;
			IResourceManager* mpResourceManager = nullptr;
			IJobManager*      mpJobManager = nullptr;
	};
}