/*!
	/file CProxyInputContext.h
	/date 15.09.2022
	/authors Kasimov Ildar
*/

#pragma once


#include "IInputContext.h"
#include "../core/CBaseObject.h"
#include "../core/Event.h"
#include "../utils/Utils.h"
#include <unordered_set>


namespace TDEngine2
{
	class IKeyboard;
	class IMouse;
	class IGamepad;


	TDE2_DECLARE_SCOPED_PTR(IKeyboard)
	TDE2_DECLARE_SCOPED_PTR(IMouse)
	TDE2_DECLARE_SCOPED_PTR(IGamepad)

	struct TProxyInputContextDesc
	{
		TVector3             mPrevMousePosition = ZeroVector3;
		TVector3             mMousePosition = ZeroVector3;

		static constexpr U16 mKeysCount = 256;
		static constexpr U16 mMouseButtonsCount = 8;

		U8                   mKeyboardState[mKeysCount];
		U8                   mPrevKeyboardState[mKeysCount];

		U8                   mMouseButtonsState[mMouseButtonsCount];
		U8                   mPrevMouseButtonsState[mMouseButtonsCount];

		std::unordered_set<E_KEYCODES> mFrameKeysInputBuffer;
		std::unordered_set<U8>         mFrameMouseButtonsInputBuffer;


	};


	/*!
		\brief A factory function for creation objects of CProxyInputContext's type

		\param[in, out] pProxyystem A pointer to IWindowSystem implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CProxyInputContext's implementation
	*/

	TDE2_API IInputContext* CreateProxyInputContext(TProxyInputContextDesc* pDesc, TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result);


	/*!
		class CProxyInputContext

		\brief The class is a mock of the input context which is used in auto-tests
	*/

	class CProxyInputContext : public IDesktopInputContext, public CBaseObject, public IEventHandler
	{
		public:
			friend TDE2_API IInputContext* CreateProxyInputContext(TProxyInputContextDesc*, TPtr<IWindowSystem>, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of an input context

				\param[in, out] pProxyystem A pointer to IProxyystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(TPtr<IWindowSystem> pWindowSystem) override;

			/*!
				\brief The method updates the current state of a context

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Update() override;

			/*!
				\brief The method polls input devices and checks up has some key been pressed

				\param[in] keyCode A key's code
			*/

			TDE2_API bool IsKeyPressed(E_KEYCODES keyCode) override;

			/*!
				\brief The method polls input devices and checks up is some key still being pressed

				\param[in] keyCode A key's code
			*/

			TDE2_API bool IsKey(E_KEYCODES keyCode) override;

			/*!
				\brief The method polls input devices and checks up has some key been unpressed

				\param[in] keyCode A key's code
			*/

			TDE2_API bool IsKeyUnpressed(E_KEYCODES keyCode) override;

			/*!
				\brief The method polls input devices and checks up has some button been pressed

				\param[in] button A button's index
			*/

			TDE2_API bool IsMouseButtonPressed(U8 button) override;

			/*!
				\brief The method polls input devices and checks up is some button still being pressed

				\param[in] button A button's index
			*/

			TDE2_API bool IsMouseButton(U8 button) override;

			/*!
				\brief The method polls input devices and checks up has some button been unpressed

				\param[in] button A button's index
			*/

			TDE2_API bool IsMouseButtonUnpressed(U8 button) override;

			/*!
				\brief The method receives a given event and processes it

				\param[in] pEvent A pointer to event data

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE OnEvent(const TBaseEvent* pEvent) override;


			/*!
				\brief The method returns an identifier of a listener

				\return The method returns an identifier of a listener
			*/

			TDE2_API TEventListenerId GetListenerId() const override;

			/*!
				\brief The method returns a position of a cursor

				\return The method returns a position of a cursor
			*/

			TDE2_API TVector3 GetMousePosition() const override;

			/*!
				\brief The method returns normalized mouse position where the center of the screen is (0, 0)
				and each component lies in range of [-1; 1]

				\return The method returns normalized mouse position where the center of the screen is (0, 0)
				and each component lies in range of [-1; 1]
			*/

			TDE2_API TVector2 GetNormalizedMousePosition() const override;

			/*!
				\brief The method returns a 3d vector that's composed from
				shift values of each mouse axis

				\return The method returns a 3d vector that's composed from
				shift values of each mouse axis
			*/

			TDE2_API TVector3 GetMouseShiftVec() const override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

			/*!
				\brief The method return a pointer to IGamepad implementation

				\param[in] gamepadId An identifier of a gamepad

				\return The method return a pointer to IGamepad implementation
			*/

			TDE2_API TPtr<IGamepad> GetGamepad(U8 gamepadId) const override;

	#if TDE2_EDITORS_ENABLED
			TDE2_API void SetOnCharInputCallback(const TOnCharActionCallback& onEventAction) override;
	#endif
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CProxyInputContext)
		protected:
	#if TDE2_EDITORS_ENABLED
			TOnCharActionCallback   mOnCharInputCallback;
	#endif

			TPtr<IWindowSystem>     mpWindowSystem;

			TProxyInputContextDesc* mpContextDesc;
	};
}