/*!
	\file CWin32WindowSystem.h
	\date 17.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../../utils/Config.h"

#if defined(TDE2_USE_WIN32PLATFORM) /// use


#include "./../../core/IWindowSystem.h"
#include <Windows.h>


namespace TDEngine2
{
	/*!
		class CWin32WindowSystem

		\brief The class provides methods for a window system, which works under Windows
	*/

	class CWin32WindowSystem : public IWindowSystem
	{
		public:
			/*!
				\brief A factory function for creation objects of CWin32WindowSystem type

				\return A pointer to CWin32WindowSystem's implementation
			*/

			friend TDE2_API IWindowSystem* CreateWin32WindowSystem(const std::string& name, U32 width, U32 height, U32 flags, E_RESULT_CODE& result);

		public:
			TDE2_API virtual ~CWin32WindowSystem();

			/*!
				\brief The method initializes a main window

				\param[in] name A name of a main window
				\param[in] width A window's width
				\param[in] height A window's height
				\param[in] flags An additional flags (bitwise value) lets configure
				additional settings of a window is created

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(const std::string& name, U32 width, U32 height, U32 flags = 0x0) override;

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
		protected:
			TDE2_API CWin32WindowSystem();

			TDE2_API CWin32WindowSystem(const CWin32WindowSystem& windowSystem) = delete;
			TDE2_API virtual CWin32WindowSystem& operator= (CWin32WindowSystem& windowSystem) = delete;

			TDE2_API static LRESULT CALLBACK _wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		protected:
			HWND                     mWindowHandler;
			HINSTANCE                mInstanceHandler;
			U32                      mWidth;
			U32                      mHeight;
			U32                      mSetupFlags;
			std::string              mWindowName;
			std::string              mWindowClassName;
			bool                     mIsInitialized;
			static C8                mAppWinProcParamName[];
			TWin32InternalWindowData mInternalDataObject;
			ITimer*                  mpTimer;
	};


	/*!
		\brief A factory function for creation objects of CWin32WindowSystem type

		\return A pointer to CWin32WindowSystem's implementation
	*/

	TDE2_API IWindowSystem* CreateWin32WindowSystem(const std::string& name, U32 width, U32 height, U32 flags, E_RESULT_CODE& result);
}

#endif