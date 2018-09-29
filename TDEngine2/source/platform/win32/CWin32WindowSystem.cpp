#include "./../../../include/platform/win32/CWin32WindowSystem.h"
#include "./../../../include/platform/win32/CWin32Timer.h"


#if defined(TDE2_USE_WIN32PLATFORM)


namespace TDEngine2
{
	C8 CWin32WindowSystem::mAppWinProcParamName[] = "WindowObjectPtr";	/// the proerty's name is used in WNDPROC function to retrieve window's object


	CWin32WindowSystem::CWin32WindowSystem():
		mIsInitialized(false)
	{
	}

	CWin32WindowSystem::~CWin32WindowSystem()
	{
	}

	E_RESULT_CODE CWin32WindowSystem::Init(const std::string& name, U32 width, U32 height, U32 flags)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		mIsInitialized = true;

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

		mWindowHandler = CreateWindow(mWindowClassName.c_str(), mWindowName.c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT, 
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

		E_RESULT_CODE result = RC_OK;

		/// CWin32Timer's initialization

		mpTimer = CreateWin32Timer(result);

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

		if (!DestroyWindow(mWindowHandler))
		{
			return RC_FAIL;
		}

		if (!UnregisterClass(mWindowClassName.c_str(), mInstanceHandler))
		{
			return RC_FAIL;
		}

		RemoveProp(mWindowHandler, mAppWinProcParamName);

		/// \todo add invokation of OnFree user's method here

		E_RESULT_CODE result = RC_OK;

		/// CWin32Timer's destruction
		if ((result = mpTimer->Free()) != RC_OK)
		{
			return result;
		}

		mIsInitialized = false;

		delete this;

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

		return true;
	}

	E_RESULT_CODE CWin32WindowSystem::Quit()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		PostQuitMessage(0);

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


	LRESULT CALLBACK CWin32WindowSystem::_wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		IWindowSystem* pWinSystem = static_cast<IWindowSystem*>(GetProp(hWnd, CWin32WindowSystem::mAppWinProcParamName));

		if (!pWinSystem)
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
		
		switch (uMsg)
		{
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			default:
				return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		return 0;
	}


	TDE2_API IWindowSystem* CreateWin32WindowSystem(const std::string& name, U32 width, U32 height, U32 flags, E_RESULT_CODE& result)
	{
		IWindowSystem* pWindowSystemInstance = new (std::nothrow) CWin32WindowSystem();

		if (!pWindowSystemInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pWindowSystemInstance->Init(name, width, height, flags);

		return pWindowSystemInstance;
	}
}

#endif