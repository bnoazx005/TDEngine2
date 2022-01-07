/*!
	\file CTransformSystem.h
	\date 04.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include <vector>


namespace TDEngine2
{
	class CTransform;
	class CBoundsComponent;
	class IGraphicsContext;


	/*!
		\brief A factory function for creation objects of CTransformSystem's type.

		\param[in] pGraphicsContext A pointer to IGraphicsContext implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTransformSystem's implementation
	*/

	TDE2_API ISystem* CreateTransformSystem(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CTransformSystem

		\brief The class is a system that processes ITransform components
	*/

	class CTransformSystem: public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateTransformSystem(IGraphicsContext*, E_RESULT_CODE&);
		public:
			struct TSystemContext
			{
				std::vector<CTransform*>       mpTransforms;
				std::vector<CBoundsComponent*> mpBounds;
				std::vector<U32>               mParentsHashTable;
				std::vector<bool>              mHasCameras;
			};
		public:
			TDE2_SYSTEM(CTransformSystem);

			/*!
				\brief The method initializes an inner state of a system
				
				\param[in] pGraphicsContext A pointer to IGraphicsContext implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext);

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTransformSystem)
		protected:
			TSystemContext    mComponentsContext;

			IGraphicsContext* mpGraphicsContext;
	};
}