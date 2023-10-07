/*!
	\file CUIEventsSystem.h
	\date 22.05.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include "../math/MathUtils.h"
#include "../math/TRect.h"
#include <vector>


namespace TDEngine2
{
	class IInputContext;
	class IDesktopInputContext;
	class CTransform;
	class CInputReceiver;
	class CLayoutElement;
	class IImGUIContext;


	/*!
		\brief A factory function for creation objects of CUIEventsSystem's type.

		\param[in, out] pInputContext A pointer to IInputContext's implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CUIEventsSystem's implementation
	*/

	TDE2_API ISystem* CreateUIEventsSystem(IInputContext* pInputContext, IImGUIContext* pImGUIContext, E_RESULT_CODE& result);


	/*!
		class CUIEventsSystem

		\brief The class is a system that processes events through UI elements environment
	*/

	class CUIEventsSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateUIEventsSystem(IInputContext*, IImGUIContext*, E_RESULT_CODE&);
		public:
			struct TSystemContext
			{
				std::vector<CTransform*>     mpTransforms;
				std::vector<CLayoutElement*> mpLayoutElements;
				std::vector<CInputReceiver*> mpInputReceivers;
				std::vector<U32>             mPriorities;
				std::vector<USIZE>           mParents;
				std::vector<TRectF32>        mMaskRects; ///< contains computed UI masks relative to parent
				std::vector<TRange<USIZE>>   mCanvasesRanges;
			};
		public:
			TDE2_SYSTEM(CUIEventsSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pInputContext A pointer to IInputContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IInputContext* pInputContext, IImGUIContext* pImGUIContext);

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CUIEventsSystem)
		protected:
			TSystemContext mContext;

			IInputContext* mpInputContext; 
			IDesktopInputContext* mpDesktopInputContext;

			IImGUIContext* mpImGUIContext;

			std::string mInputBuffer;
	};


	TDE2_API std::vector<TEntityId> FindMainCanvases(IWorld* pWorld);
}