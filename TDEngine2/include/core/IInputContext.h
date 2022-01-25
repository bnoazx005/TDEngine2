/*!
	/file IInputContext.h
	/date 15.12.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "../utils/Utils.h"
#include "../utils/Types.h"
#include "IEngineSubsystem.h"
#include "Event.h"
#include "../math/TVector2.h"
#include "../math/TVector3.h"
#include "../utils/CU8String.h"


namespace TDEngine2
{
	class IWindowSystem;
	class IGamepad;


	TDE2_DECLARE_SCOPED_PTR(IWindowSystem)
	TDE2_DECLARE_SCOPED_PTR(IGamepad)


#if TDE2_EDITORS_ENABLED

	/*!
		struct TOnCharInputEvent

		\brief The type is used to inform listeners about some character is entered using a keyboard.
		The only purpose is to provide correct input for editors. Not used in game runtime code
	*/

	typedef struct TOnCharInputEvent : TBaseEvent
	{
		virtual ~TOnCharInputEvent() = default;

		TDE2_REGISTER_TYPE(TOnCharInputEvent)
		REGISTER_EVENT_TYPE(TOnCharInputEvent)

		U32 mCharCode = 0x0;
	} TOnCharInputEvent, *TOnCharInputEventPtr;

#endif


	/*!
		enum class E_KEYCODES

		\brief The enumeration contains a list of all available keycodes
	*/

	enum class E_KEYCODES: U16
	{
		KC_ESCAPE,
		KC_ALPHA0, KC_ALPHA1, KC_ALPHA2, KC_ALPHA3, KC_ALPHA4,
		KC_ALPHA5, KC_ALPHA6, KC_ALPHA7, KC_ALPHA8, KC_ALPHA9,
		KC_NUMPAD0, KC_NUMPAD1, KC_NUMPAD2, KC_NUMPAD3, KC_NUMPAD4,
		KC_NUMPAD5, KC_NUMPAD6, KC_NUMPAD7, KC_NUMPAD8, KC_NUMPAD9,
		KC_A, KC_B, KC_C, KC_D, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M,
		KC_N, KC_O, KC_P, KC_Q, KC_R, KC_S, KC_T, KC_U, KC_V, KC_W, KC_X, KC_Y, KC_Z,
		KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7,
		KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, KC_F13, KC_F14, KC_F15,
		KC_SPACE,
		KC_LSHIFT, KC_RSHIFT,
		KC_LALT, KC_RALT,
		KC_CAPSLOCK,
		KC_TAB,
		KC_RETURN,
		KC_LCONTROL, KC_RCONTROL,	/// LCOMMAND / RCOMMAND on OS X
		KC_RIGHT, KC_UP, KC_LEFT, KC_DOWN,
		KC_HOME, KC_END,
		KC_PAGEUP, KC_PAGEDOWN,
		KC_PAUSE,
		KC_BACKSPACE,
		KC_DELETE,
		KC_INSERT,
		KC_LSYS, KC_RSYS, /// Windows / OS X / UNIX key
		KC_EXCLAIM, KC_AT, KC_DOUBLE_QUOTE, KC_HASH, KC_DOLLAR, KC_PERCENT, KC_AMPERSAND,
		KC_QUOTE, KC_LPAREN, KC_RPAREN, KC_ASTERISK, KC_PLUS, KC_MINUS, KC_EQUALS,
		KC_COMMA, KC_PERIOD, KC_SLASH, KC_BACKSLASH, KC_COLON, KC_SEMICOLON, KC_LESS,
		KC_GREATER, KC_QUESTION, KC_LBRACKET, KC_RBRACKET, KC_CARET, KC_UNDERSCORE, KC_BACK_QUOTE,
		KC_LCURLY_BRACKET, KC_RCURLY_BRACKET, KC_PIPE, KC_TILDE,
		KC_NUMLOCK,
		KC_SCROLL_LOCK,
		KC_PRINT,
		KC_SYSREQ,
		KC_BREAK,
		KC_NONE
	};


	/*!
		\brief The function converts E_KEYCODE's value into UTF-8 character
		\note The definition is placed in CU8String.cpp file

		\param[in] keyCode A value of E_KEYCODE type

		\return A character in UTF-8 encoding
	*/

	TDE2_API TUtf8CodePoint KeyCodeToUTF8Char(const E_KEYCODES& keyCode);

	TDE2_API bool IsAlphaNumericKeyCode(const E_KEYCODES& keyCode);


	/*!
		enum class E_GAMEPAD_BUTTONS

		\brief The enumeration contains all available buttons that all XInput compatible gamepads have
	*/

	enum class E_GAMEPAD_BUTTONS
	{
		GB_UP, GB_LEFT, GB_RIGHT, GB_DOWN,
		GB_START,
		GB_BACK,
		GB_LTHUMB, GB_RTHUMB,
		GB_LSHOULDER, GB_RSHOULDER,
		GB_A, GB_B, GB_X, GB_Y
	};


	/*!
		interface IInputContext

		\brief The interface represents a low-level input system that
		wraps up calls of some raw input API
	*/

	class IInputContext: public IEngineSubsystem
	{
		public:
			/*!
				\brief The method initializes an internal state of an input context

				\param[in, out] pWindowSystem A pointer to IWindowSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TPtr<IWindowSystem> pWindowSystem) = 0;

			/*!
				\brief The method updates the current state of a context

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Update() = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_INPUT_CONTEXT; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IInputContext)
	};


	/*!
		interface IDesktopInputContext

		\brief The interface describes a common functionality of a basic input context
		that is targeted for desktops
	*/

	class IDesktopInputContext: public IInputContext
	{
		public:
			typedef std::function<void(TUtf8CodePoint)> TOnCharActionCallback;
		public:
			/*!
				\brief The method polls input devices and checks up has some key been pressed

				\param[in] keyCode A key's code
			*/

			TDE2_API virtual bool IsKeyPressed(E_KEYCODES keyCode) = 0;

			/*!
				\brief The method polls input devices and checks up is some key still being pressed

				\param[in] keyCode A key's code
			*/

			TDE2_API virtual bool IsKey(E_KEYCODES keyCode) = 0;

			/*!
				\brief The method polls input devices and checks up has some key been unpressed

				\param[in] keyCode A key's code
			*/

			TDE2_API virtual bool IsKeyUnpressed(E_KEYCODES keyCode) = 0;

			/*!
				\brief The method polls input devices and checks up has some button been pressed

				\param[in] button A button's index
			*/

			TDE2_API virtual bool IsMouseButtonPressed(U8 button) = 0;

			/*!
				\brief The method polls input devices and checks up is some button still being pressed

				\param[in] button A button's index
			*/

			TDE2_API virtual bool IsMouseButton(U8 button) = 0;

			/*!
				\brief The method polls input devices and checks up has some button been unpressed

				\param[in] button A button's index
			*/

			TDE2_API virtual bool IsMouseButtonUnpressed(U8 button) = 0;

#if TDE2_EDITORS_ENABLED
			TDE2_API virtual void SetOnCharInputCallback(const TOnCharActionCallback& onEventAction) = 0;
#endif

			/*!
				\brief The method returns a position of a cursor

				\return The method returns a position of a cursor
			*/

			TDE2_API virtual TVector3 GetMousePosition() const = 0;

			/*!
				\brief The method returns normalized mouse position where the center of the screen is (0, 0)
				and each component lies in range of [-1; 1]

				\return The method returns normalized mouse position where the center of the screen is (0, 0)
				and each component lies in range of [-1; 1]
			*/

			TDE2_API virtual TVector2 GetNormalizedMousePosition() const = 0;

			/*!
				\brief The method returns a 3d vector that's composed from
				shift values of each mouse axis

				\return The method returns a 3d vector that's composed from
				shift values of each mouse axis
			*/

			TDE2_API virtual TVector3 GetMouseShiftVec() const = 0;

			/*!
				\brief The method return a pointer to IGamepad implementation

				\param[in] gamepadId An identifier of a gamepad

				\return The method return a pointer to IGamepad implementation
			*/

			TDE2_API virtual TPtr<IGamepad> GetGamepad(U8 gamepadId) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IDesktopInputContext)
	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(IInputContext);
}