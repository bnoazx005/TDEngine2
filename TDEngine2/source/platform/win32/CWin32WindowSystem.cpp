#include "./../../../include/platform/win32/CWin32WindowSystem.h"
#include "./../../../include/platform/win32/CWin32Timer.h"
#include "./../../../include/platform/win32/CWin32DLLManager.h"
#include "./../../../include/utils/CFileLogger.h"
#include "./../../../include/core/IEventManager.h"
#include <string>


#if defined(TDE2_USE_WIN32PLATFORM)


namespace TDEngine2
{
	C8 CWin32WindowSystem::mAppWinProcParamName[] = "WindowObjectPtr";	/// the proerty's name is used in WNDPROC function to retrieve window's object


	CWin32WindowSystem::CWin32WindowSystem():
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CWin32WindowSystem::Init(IEventManager* pEventManager, const std::string& name, U32 width, U32 height, U32 flags)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEventManager)
		{
			return RC_INVALID_ARGS;
		}
		
		mpEventManager = pEventManager;
		
		mpEventManager->Subscribe(TOnWindowResized::GetTypeId(), this); /// the window also listens to its own events to properly update internal data outside of _wndProc
		mpEventManager->Subscribe(TOnWindowMoved::GetTypeId(), this);

		mWindowName      = name;
		mWindowClassName = mWindowName + "Class";

		mWidth  = width;
		mHeight = height;
		
		mSetupFlags  = flags;

		mInstanceHandler = GetModuleHandle(nullptr);

		memset(&mInternalDataObject, 0, sizeof(mInternalDataObject));

		WNDCLASSEX wc;
		memset(&wc, 0, sizeof(wc));

		wc.cbSize        = sizeof(WNDCLASSEX);
		wc.lpfnWndProc   = (WNDPROC)_wndProc;
		wc.hInstance     = mInstanceHandler;
		wc.lpszClassName = mWindowClassName.c_str();
		wc.cbWndExtra    = NULL;
		wc.cbClsExtra    = NULL;
		wc.hIcon         = NULL;
		wc.lpszMenuName  = NULL;
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
		wc.style         = CS_HREDRAW | CS_VREDRAW;
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

		if (!RegisterClassEx(&wc))
		{
			return RC_FAIL;
		}

		DWORD style = (flags & P_RESIZEABLE) ? WS_OVERLAPPEDWINDOW : WS_EX_TOOLWINDOW;

		if (flags & P_FULLSCREEN)
		{
			style = WS_POPUPWINDOW;

			mWidth = GetSystemMetrics(SM_CXSCREEN);
			mHeight = GetSystemMetrics(SM_CYSCREEN);
		}

		mWindowXPos = CW_USEDEFAULT;
		mWindowYPos = CW_USEDEFAULT;

		mWindowHandler = CreateWindow(mWindowClassName.c_str(), mWindowName.c_str(), style, mWindowXPos, mWindowYPos, 
									  mWidth, mHeight, nullptr, nullptr, mInstanceHandler, nullptr);

		if (!mWindowHandler)
		{
			return RC_FAIL;
		}

		ShowWindow(mWindowHandler, SW_SHOW);
		UpdateWindow(mWindowHandler);
		
		mInternalDataObject.mWindowHandler         = mWindowHandler;
		mInternalDataObject.mWindowInstanceHandler = mInstanceHandler;

		SetProp(mWindowHandler, mAppWinProcParamName, this); //attach the window's object as the window's parameter

		/// try to get a device context's handler
		mInternalDataObject.mDeviceContextHandler = GetDC(mWindowHandler);

		/// \todo add invokation of user's OnInit method here
		
		LOG_MESSAGE("[Win32 Window System] The window system was successfully initialized");

		E_RESULT_CODE result = RC_OK;

		/// CWin32Timer's initialization

		mpTimer = CreateWin32Timer(result);

		if (result != RC_OK)
		{
			return result;
		}

		/// CWin32DLLManager's initialization

		mpDLLManager = CreateWin32DLLManager(result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;
		
		return RC_OK;
	}


	E_RESULT_CODE CWin32WindowSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_OK;
		}

		E_RESULT_CODE result = DestroyWindow(mWindowHandler) ? RC_FAIL : RC_OK;
		result = result | (!UnregisterClass(mWindowClassName.c_str(), mInstanceHandler) ? RC_FAIL : RC_OK);

		RemoveProp(mWindowHandler, mAppWinProcParamName);

		/// \todo add invokation of OnFree user's method here
		
		result = result | mpTimer->Free();
		result = result | mpDLLManager->Free();

		mIsInitialized = false;

		delete this;

		LOG_MESSAGE("[Win32 Window System] The window system was successfully destroyed");

		return RC_OK;
	}

	bool CWin32WindowSystem::Run(const std::function<void()>& onFrameUpdate)
	{
		if (!mIsInitialized)
		{
			return false;
		}

		MSG currMessage;

		memset(&currMessage, 0, sizeof(currMessage));

		while (currMessage.message != WM_QUIT)
		{
			if (PeekMessage(&currMessage, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&currMessage);
				DispatchMessage(&currMessage);
			}
			else
			{
				mpTimer->Tick();

				onFrameUpdate();
			}
		}

		LOG_MESSAGE("[Win32 Window System] The main loop's execution was interrupted");

		return true;
	}

	E_RESULT_CODE CWin32WindowSystem::Quit()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		PostQuitMessage(0);

		LOG_MESSAGE("[Win32 Window System] The quit signal was received");

		return RC_OK;
	}

	E_RESULT_CODE CWin32WindowSystem::SetTitle(const std::string& title)
	{
		mWindowName = title;

		if (!SetWindowText(mWindowHandler, mWindowName.c_str()))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_ENGINE_SUBSYSTEM_TYPE CWin32WindowSystem::GetType() const
	{
		return EST_WINDOW;
	}

	const TWindowSystemInternalData& CWin32WindowSystem::GetInternalData() const
	{
		return mInternalDataObject;
	}

	U32 CWin32WindowSystem::GetWidth() const
	{
		return mWidth;
	}

	U32 CWin32WindowSystem::GetHeight() const
	{
		return mHeight;
	}

	ITimer* CWin32WindowSystem::GetTimer() const
	{
		return mpTimer;
	}

	const std::string& CWin32WindowSystem::GetTitle() const
	{
		return mWindowName;
	}

	U32 CWin32WindowSystem::GetFlags() const
	{
		return mSetupFlags;
	}

	IDLLManager* CWin32WindowSystem::GetDLLManagerInstance() const
	{
		return mpDLLManager;
	}

	IEventManager* CWin32WindowSystem::GetEventManager() const
	{
		return mpEventManager;
	}

	E_RESULT_CODE CWin32WindowSystem::OnEvent(const TBaseEvent* pEvent)
	{
		TypeId eventTypeId = pEvent->GetEventType();

		if (eventTypeId == TOnWindowResized::GetTypeId())
		{
			const TOnWindowResized* pOnWindowResizedEvent = dynamic_cast<const TOnWindowResized*>(pEvent);

			mWidth  = pOnWindowResizedEvent->mWidth;
			mHeight = pOnWindowResizedEvent->mHeight;

			return RC_OK;
		}

		if (eventTypeId == TOnWindowMoved::GetTypeId())
		{
			const TOnWindowMoved* pOnWindowMovedEvent = dynamic_cast<const TOnWindowMoved*>(pEvent);

			mWindowXPos = pOnWindowMovedEvent->mX;
			mWindowYPos = pOnWindowMovedEvent->mY;

			return RC_OK;
		}
		
		return RC_OK;
	}

	TEventListenerId CWin32WindowSystem::GetListenerId() const
	{
		return GetTypeId();
	}

	TRectU32 CWin32WindowSystem::GetWindowRect() const
	{
		return { mWindowXPos, mWindowYPos, mWidth, mHeight };
	}


	LRESULT CALLBACK CWin32WindowSystem::_wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		IWindowSystem* pWinSystem = static_cast<IWindowSystem*>(GetProp(hWnd, CWin32WindowSystem::mAppWinProcParamName));

		if (!pWinSystem)
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		IEventManager* pEventManager = pWinSystem->GetEventManager();
		
		TOnWindowResized onResizedEvent;

		TOnWindowMoved onMovedEvent;

		switch (uMsg)
		{
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			case WM_SIZE:
				onResizedEvent.mWidth  = lParam & (0x0000FFFF);
				onResizedEvent.mHeight = (lParam & (0xFFFF0000)) >> 16;

				pEventManager->Notify(&onResizedEvent);

				LOG_MESSAGE(std::string("[Win32 Window System] The window's sizes were changed (width: ").
									append(std::to_string(onResizedEvent.mWidth)).
									append(", height: ").
									append(std::to_string(onResizedEvent.mHeight)).
									append(")"));

				break;
			case WM_MOVE:
				onMovedEvent.mX = lParam & (0x0000FFFF);
				onMovedEvent.mY = (lParam & (0xFFFF0000)) >> 16;

				pEventManager->Notify(&onMovedEvent);
				break;
			default:
				return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		return 0;
	}


	TDE2_API IWindowSystem* CreateWin32WindowSystem(IEventManager* pEventManager, const std::string& name, U32 width, U32 height, U32 flags, E_RESULT_CODE& result)
	{
		CWin32WindowSystem* pWindowSystemInstance = new (std::nothrow) CWin32WindowSystem();

		if (!pWindowSystemInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pWindowSystemInstance->Init(pEventManager, name, width, height, flags);

		if (result != RC_OK)
		{
			delete pWindowSystemInstance;

			pWindowSystemInstance = nullptr;
		}

		return dynamic_cast<IWindowSystem*>(pWindowSystemInstance);
	}
}

#endif