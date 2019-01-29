/*!
	\file CUnixWindowSystem.h
	\date 12.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../../utils/Config.h"
#include "./../../utils/Utils.h"
#include "./../../core/Event.h"

#if defined(TDE2_USE_UNIXPLATFORM)


#include "./../../core/IWindowSystem.h"
#include <X11/Xutil.h>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CUnixWindowSystem type

		\return A pointer to CUnixWindowSystem's implementation
	*/

	TDE2_API IWindowSystem* CreateUnixWindowSystem(IEventManager* pEventManager, const std::string& name, U32 width, U32 height, U32 flags, E_RESULT_CODE& result);

	/*!
		class CUnixWindowSystem

		\brief The class provides methods for a window system, which works under a UNIX platform
	*/

	class CUnixWindowSystem : public IWindowSystem, public IEventHandler
	{
		public:
			/*!
				\brief A factory function for creation objects of CUnixWindowSystem type

				\return A pointer to CUnixWindowSystem's implementation
			*/

			friend TDE2_API IWindowSystem* CreateUnixWindowSystem(IEventManager* pEventManager, const std::string& name, U32 width, U32 height, U32 flags, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CUnixWindowSystem)

			/*!
				\brief The method initializes a main window

				\param[in, out] pEventManager A pointer to IEventManager implementation

				\param[in] name A name of a main window

				\param[in] width A window's width

				\param[in] height A window's height

				\param[in] flags An additional flags (bitwise value) lets configure

				additional settings of a window is created

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IEventManager* pEventManager, const std::string& name, U32 width, U32 height, U32 flags = 0x0) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method processes a window's update (executing messages, redraw content)

				\param[in] onFrameUpdate A callback that is invoked every time, when the window doesn't process a message

				\return The method returns false, if Quit method was invoked or the window was closed
				in other possible way, true in other cases
			*/

			TDE2_API bool Run(const std::function<void()>& onFrameUpdate) override;

			/*!
				\brief The method stops the engine's main loop

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Quit() override;

			/*!
				\brief The method changes a window's title

				\param[in] title A new title's value

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetTitle(const std::string& title) override;

			/*!
				\brief The method proceeds creation of a window if OpenGL is needed

				\param[in] pVisualInfo An information about needed parameters for a window

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE EnableOpenGL(XVisualInfo* pVisualInfo);

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

			/*!
				\brief The method returns an object that contains internal handlers that are used by the system.

				The structure of the object and its members can vary on different platforms.

				return The method returns an object that contains internal handlers that are used by the system
			*/

			TDE2_API const TWindowSystemInternalData& GetInternalData() const override;

			/*!
				\brief The method returns a width of a window

				\return The method returns a width of a window
			*/

			TDE2_API U32 GetWidth() const override;

			/*!
				\brief The method returns a height of a window

				\return The method returns a height of a window
			*/

			TDE2_API U32 GetHeight() const override;

			/*!
				\brief The method returns a pointer to a in-engine timer

				\returns The method returns a pointer to a in-engine timer
			*/

			TDE2_API ITimer* GetTimer() const override;

			/*!
				\brief The method returns a window's title

				\returns The method returns a window's title
			*/

			TDE2_API const std::string& GetTitle() const override;

			/*!
				\brief The method returns current set flags

				\return The method returns current set flags
			*/

			TDE2_API U32 GetFlags() const override;

			/*!
				\brief The method returns a pointer to IDLLManager's implementation

				\return The method returns a pointer to IDLLManager's implementation
			*/

			TDE2_API IDLLManager* GetDLLManagerInstance() const override;

			/*!
				\brief The method returns a pointer to IEventManager implementation

				\return The method returns a pointer to IEventManager implementation
			*/

			TDE2_API IEventManager* GetEventManager() const override;

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
				\brief The method returns an object of TRect type which contains
				information about sizes of the window and its position

				\return The method returns an object of TRect type which contains
				information about sizes of the window and its position
			*/

			TDE2_API TRectU32 GetWindowRect() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CUnixWindowSystem)

			TDE2_API E_RESULT_CODE _createWindow();

			TDE2_API void _processEvents();

			TDE2_API void _sendWindowResizedEvent(U32 width, U32 height);

			TDE2_API void _sendWindowMovedEvent(U32 x, U32 y);

			TDE2_API void _setFullscreenMode(bool value);

			TDE2_API void _disableWindowResizing();
		protected:
			Display*                  mpDisplayHandler;

			Window                    mRootWindowHandler;

			Window                    mWindowHandler;

			U32                       mWidth;
			
			U32                       mHeight;

			U32                       mSetupFlags;

			U32                       mWindowXPos;

			U32                       mWindowYPos;

			std::string               mWindowName;

			bool                      mIsInitialized;
			
			TWindowSystemInternalData mInternalDataObject;
			
			ITimer*                   mpTimer;

			IDLLManager*              mpDLLManager;

			volatile bool             mIsRunning;

			IEventManager*            mpEventManager;
	};
}

#endif