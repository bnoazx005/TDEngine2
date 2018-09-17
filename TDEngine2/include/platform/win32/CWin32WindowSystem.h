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

	class TDE2_API CWin32WindowSystem : public IWindowSystem
	{
		public:
			/*!
				\brief A factory function for creation objects of CWin32WindowSystem type

				\return A pointer to CWin32WindowSystem's implementation
			*/

			friend TDE2_API IWindowSystem* CreateWin32WindowSystem(const std::string& name, U32 width, U32 height, U32 flags, E_RESULT_CODE& result);

		public:
			virtual ~CWin32WindowSystem();

			/*!
				\brief The method initializes a main window

				\param[in] name A name of a main window
				\param[in] width A window's width
				\param[in] height A window's height
				\param[in] flags An additional flags (bitwise value) lets configure
				additional settings of a window is created

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Init(const std::string& name, U32 width, U32 height, U32 flags = 0x0) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Free() override;
			
			/*!
				\brief The method processes a window's update (executing messages, redraw content)

				\return The method returns false, if Quit method was invoked or the window was closed
				in other possible way, true in other cases
			*/

			bool Run() override;

			/*!
				\brief The method stops the engine's main loop

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Quit() override;


			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			E_ENGINE_SUBSYSTEM_TYPE GetType() const override;
		protected:
			CWin32WindowSystem();

			CWin32WindowSystem(const CWin32WindowSystem& windowSystem) = delete;
			virtual CWin32WindowSystem& operator= (CWin32WindowSystem& windowSystem) = delete;

			static LRESULT CALLBACK _wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		protected:
			HWND        mWindowHandler;
			HINSTANCE   mInstanceHandler;
			U32         mWidth;
			U32         mHeight;
			U32         mSetupFlags;
			std::string mWindowName;
			std::string mWindowClassName;
			bool        mIsInitialized;
			static C8   mAppWinProcParamName[];
	};


	/*!
		\brief A factory function for creation objects of CWin32WindowSystem type

		\return A pointer to CWin32WindowSystem's implementation
	*/

	TDE2_API IWindowSystem* CreateWin32WindowSystem(const std::string& name, U32 width, U32 height, U32 flags, E_RESULT_CODE& result);
}

#endif