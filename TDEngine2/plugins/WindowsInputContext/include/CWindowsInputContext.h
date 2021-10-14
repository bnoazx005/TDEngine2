/*!
	/file CWindowsInputContext.h
	/date 15.12.2018
	/authors Kasimov Ildar
*/

#pragma once


#include <core/IInputContext.h>
#include <core/CBaseObject.h>
#include <utils/Utils.h>


#if defined (TDE2_USE_WIN32PLATFORM)

#include <dinput.h>
#include <Xinput.h>


#pragma comment(lib, "dinput8")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "xinput")


namespace TDEngine2
{
	class IKeyboard;
	class IMouse;
	class IGamepad;


	TDE2_DECLARE_SCOPED_PTR(IKeyboard)
	TDE2_DECLARE_SCOPED_PTR(IMouse)
	TDE2_DECLARE_SCOPED_PTR(IGamepad)


	/*!
		struct TInternalInputData

		\brief The structure contains all the internal data that is used by CDirectInputContext
	*/

	typedef struct TInternalInputData
	{
		IDirectInput8* mpInput;

		HWND           mWindowHandler;
	} TInternalInputData, *TInternalInputDataPtr;


	/*!
		\brief A factory function for creation objects of CWindowsInputContext's type

		\param[in, out] pWindowSystem A pointer to IWindowSystem implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CWindowsInputContext's implementation
	*/

	TDE2_API IInputContext* CreateWindowsInputContext(TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result);


	/*!
		class CWindowsInputContext

		\brief The class is an implementation of a low-level input context based
		on DirectInput and XInput APIs
	*/

	class CWindowsInputContext : public IDesktopInputContext, public CBaseObject
	{
		public:
			friend TDE2_API IInputContext* CreateWindowsInputContext(TPtr<IWindowSystem>, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of an input context

				\param[in, out] pWindowSystem A pointer to IWindowSystem implementation

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
				\brief The method returns an object which contains an internal data of the input context

				\return The method returns an object which contains an internal data of the input context
			*/

			TDE2_API const TInternalInputData& GetInternalHandler() const;
			
			/*!
				\brief The method return a pointer to IGamepad implementation

				\param[in] gamepadId An identifier of a gamepad

				\return The method return a pointer to IGamepad implementation
			*/

			TDE2_API TPtr<IGamepad> GetGamepad(U8 gamepadId) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CWindowsInputContext)

			TDE2_API E_RESULT_CODE _createInputInternalHandler(HINSTANCE windowHandler);

			TDE2_API E_RESULT_CODE _releaseInternalHandler();

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			IDirectInput8*     mpInput;

			TPtr<IKeyboard>    mpKeyboardDevice;

			TPtr<IMouse>       mpMouseDevice;

			static const U8    mMaxNumOfGamepads = XUSER_MAX_COUNT;

			TPtr<IGamepad>     mpGamepads[mMaxNumOfGamepads];

			TInternalInputData mInternalData;
	};
}

#endif