#include "../../../include/platform/win32/CWin32WindowSystem.h"
#include "../../../include/platform/win32/CWin32Timer.h"
#include "../../../include/platform/win32/CWin32DLLManager.h"
#include "../../../include/utils/CFileLogger.h"
#include "../../../include/core/IEventManager.h"
#include "../../../include/core/IImGUIContext.h"
#include "../../../include/core/IInputContext.h"
#include <string>


#if defined(TDE2_USE_WINPLATFORM)


namespace TDEngine2
{
	static const C8 AppWinProcParamName[] = "WindowObjectPtr";	/// the proerty's name is used in WNDPROC function to retrieve window's object


	LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		IWindowSystem* pWinSystem = static_cast<IWindowSystem*>(GetProp(hWnd, AppWinProcParamName));

		if (!pWinSystem)
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		static bool hasWindowBeenMaximized = pWinSystem->GetFlags() & P_FULLSCREEN;

		static U32 lastWindowWidth = 0, lastWindowHeight = 0;

		TPtr<IEventManager> pEventManager = pWinSystem->GetEventManager();

		TOnWindowMoved onMovedEvent;

		auto onSendResizeWindowEvent = [&pEventManager](U32 width, U32 height)
		{
			if (width < 1 || height < 1)
			{
				return;
			}

			TOnWindowResized onResizedEvent;

			onResizedEvent.mWidth = width;
			onResizedEvent.mHeight = height;

			pEventManager->Notify(&onResizedEvent);

			LOG_MESSAGE(std::string("[Win32 Window System] The window's sizes were changed (width: ").
				append(std::to_string(onResizedEvent.mWidth)).
				append(", height: ").
				append(std::to_string(onResizedEvent.mHeight)).
				append(")"));

		};

		TOnCharInputEvent onCharInputEvent;

		switch (uMsg)
		{
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			case WM_SIZE:
				lastWindowWidth  = lParam & (0x0000FFFF);
				lastWindowHeight = (lParam & (0xFFFF0000)) >> 16;

				if (wParam == SIZE_MAXIMIZED)
				{
					onSendResizeWindowEvent(lastWindowWidth, lastWindowHeight);
					hasWindowBeenMaximized = true;
				}
				else
				{
					if (hasWindowBeenMaximized)
					{
						onSendResizeWindowEvent(lastWindowWidth, lastWindowHeight);
					}
					hasWindowBeenMaximized = false;
				}
				break;
			case WM_EXITSIZEMOVE:
				onSendResizeWindowEvent(lastWindowWidth, lastWindowHeight);
				break;
			case WM_MOVE:
				onMovedEvent.mX = lParam & (0x0000FFFF);
				onMovedEvent.mY = (lParam & (0xFFFF0000)) >> 16;

				pEventManager->Notify(&onMovedEvent);
				break;
			case WM_CHAR:
				onCharInputEvent.mCharCode = static_cast<U32>(wParam);
				pEventManager->Notify(&onCharInputEvent);
				break;
			default:
				return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		return 0;
	}


	CWin32WindowSystem::CWin32WindowSystem():
		CBaseObject()
	{
	}

	E_RESULT_CODE CWin32WindowSystem::Init(TPtr<IEventManager> pEventManager, const std::string& name, U32 width, U32 height, U32 flags)
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
		wc.lpfnWndProc   = (WNDPROC)WndProc;
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

		DWORD style = _getStyleByParams(flags);
		
		RECT outsideWindowRect = { 0, 0, static_cast<LONG>(mWidth), static_cast<LONG>(mHeight) };
		AdjustWindowRect(&outsideWindowRect, style, false);

		width  = outsideWindowRect.right - outsideWindowRect.left;
		height = outsideWindowRect.bottom - outsideWindowRect.top;

		if (flags & P_FULLSCREEN)
		{
			style = WS_POPUPWINDOW;

			mWidth = GetSystemMetrics(SM_CXSCREEN);
			mHeight = GetSystemMetrics(SM_CYSCREEN);

			width  = mWidth;
			height = mHeight;
		}

		mWindowXPos = CW_USEDEFAULT;
		mWindowYPos = CW_USEDEFAULT;

		mWindowHandler = CreateWindow(mWindowClassName.c_str(), mWindowName.c_str(), style, mWindowXPos, mWindowYPos, 
									  width, height, nullptr, nullptr, mInstanceHandler, nullptr);

		if (!mWindowHandler)
		{
			return RC_FAIL;
		}

		ShowWindow(mWindowHandler, SW_SHOW);
		UpdateWindow(mWindowHandler);
		
		mInternalDataObject.mWindowHandler         = mWindowHandler;
		mInternalDataObject.mWindowInstanceHandler = mInstanceHandler;

		SetProp(mWindowHandler, AppWinProcParamName, this); //attach the window's object as the window's parameter

		/// try to get a device context's handler
		mInternalDataObject.mDeviceContextHandler = GetDC(mWindowHandler);

		/// \todo add invokation of user's OnInit method here
		
		LOG_MESSAGE("[Win32 Window System] The window system was successfully initialized");

		E_RESULT_CODE result = RC_OK;

		/// CWin32Timer's initialization

		mpTimer = TPtr<ITimer>(CreateWin32Timer(result));

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;
		
		return RC_OK;
	}


	E_RESULT_CODE CWin32WindowSystem::_onFreeInternal()
	{
		E_RESULT_CODE result = DestroyWindow(mWindowHandler) ? RC_FAIL : RC_OK;
		result = result | (!UnregisterClass(mWindowClassName.c_str(), mInstanceHandler) ? RC_FAIL : RC_OK);

		RemoveProp(mWindowHandler, AppWinProcParamName);

		/// \todo add invokation of OnFree user's method here

		LOG_MESSAGE("[Win32 Window System] The window system was successfully destroyed");

		return result;
	}

	bool CWin32WindowSystem::Run(const std::function<void()>& onFrameUpdate)
	{
		if (!mIsInitialized)
		{
			return false;
		}

		MSG currMessage;

		memset(&currMessage, 0, sizeof(currMessage));

		bool isRunning = true;

		while (isRunning)
		{
			while (PeekMessage(&currMessage, 0, 0, 0, PM_REMOVE))
			{
				if (WM_QUIT == currMessage.message)
				{
					isRunning = false;
				}

				TranslateMessage(&currMessage);
				DispatchMessage(&currMessage);
			}

			mpTimer->Tick();

			if (onFrameUpdate)
			{
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

	E_RESULT_CODE CWin32WindowSystem::ConfigureImGUIContext(IImGUIContextVisitor* pVisitor) const
	{
		return pVisitor->ConfigureForWin32Platform(this);
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

	TPtr<ITimer> CWin32WindowSystem::GetTimer() const
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

	TPtr<IEventManager> CWin32WindowSystem::GetEventManager() const
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
		return TEventListenerId(GetTypeId());
	}

	TRectU32 CWin32WindowSystem::GetWindowRect() const
	{
		RECT windowRect;
		::GetWindowRect(mWindowHandler, &windowRect);

		return { mWindowXPos, mWindowYPos, static_cast<U32>(windowRect.right - windowRect.left), static_cast<U32>(windowRect.bottom - windowRect.top) };
	}

	TRectU32 CWin32WindowSystem::GetClientRect() const
	{
		return { mWindowXPos, mWindowYPos, mWidth, mHeight };
	}

#if TDE2_EDITORS_ENABLED

	std::string GetFilterStr(const std::vector<std::tuple<std::string, std::string>>& filters)
	{
		std::string output;

		std::string filterName;
		std::string filterPattern;

		for (const auto& currFilter : filters)
		{
			std::tie(filterName, filterPattern) = currFilter;

			output
				.append(filterName)
				.append("-");

			output.back() = '\0';

			output
				.append(filterPattern)
				.append("-");

			output.back() = '\0';
		}

		return output;
	}

	TResult<std::string> CWin32WindowSystem::ShowOpenFileDialog(const std::vector<std::tuple<std::string, std::string>>& filters)
	{
		OPENFILENAME openFileDialogDesc;       // common dialog box structure
		
		std::array<C8, 255> outputFilepath { 0 };

		// \note Initialize OPENFILENAME
		ZeroMemory(&openFileDialogDesc, sizeof(openFileDialogDesc));
		openFileDialogDesc.lStructSize = sizeof(openFileDialogDesc);

		openFileDialogDesc.hwndOwner = mWindowHandler;
		openFileDialogDesc.lpstrFile = &outputFilepath.front();
		openFileDialogDesc.nMaxFile = static_cast<DWORD>(outputFilepath.size());

		auto filtersFormattedStr = GetFilterStr(filters);
		openFileDialogDesc.lpstrFilter = filtersFormattedStr.c_str();

		openFileDialogDesc.nFilterIndex = 1;
		openFileDialogDesc.lpstrFileTitle = NULL;
		openFileDialogDesc.nMaxFileTitle = 0;
		openFileDialogDesc.lpstrInitialDir = NULL;
		openFileDialogDesc.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileName(&openFileDialogDesc) == TRUE)
		{
			return Wrench::TOkValue<std::string>(std::string(&outputFilepath.front()));
		}

		return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
	}

	TResult<std::string> CWin32WindowSystem::ShowSaveFileDialog(const std::vector<std::tuple<std::string, std::string>>& filters)
	{
		OPENFILENAME saveFileDialogDesc;       // common dialog box structure

		std::array<C8, 255> outputFilepath{ 0 };

		// \note Initialize OPENFILENAME
		ZeroMemory(&saveFileDialogDesc, sizeof(saveFileDialogDesc));
		saveFileDialogDesc.lStructSize = sizeof(saveFileDialogDesc);

		saveFileDialogDesc.hwndOwner = mWindowHandler;
		saveFileDialogDesc.lpstrFile = &outputFilepath.front();
		saveFileDialogDesc.nMaxFile = static_cast<DWORD>(outputFilepath.size());

		auto filtersFormattedStr = GetFilterStr(filters);
		saveFileDialogDesc.lpstrFilter = filtersFormattedStr.c_str();

		saveFileDialogDesc.nFilterIndex = 1;
		saveFileDialogDesc.lpstrFileTitle = NULL;
		saveFileDialogDesc.nMaxFileTitle = 0;
		saveFileDialogDesc.lpstrInitialDir = NULL;
		saveFileDialogDesc.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

		if (GetSaveFileName(&saveFileDialogDesc) == TRUE)
		{
			return Wrench::TOkValue<std::string>(std::string(&outputFilepath.front()));
		}

		return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
	}

#endif

	U32 CWin32WindowSystem::_getStyleByParams(U32 flags) const
	{
		return (flags & P_RESIZEABLE) ? WS_OVERLAPPEDWINDOW : ((flags & P_FULLSCREEN) ? WS_POPUPWINDOW : WS_EX_TOOLWINDOW);
	}


	TDE2_API IWindowSystem* CreateWin32WindowSystem(TPtr<IEventManager> pEventManager, const std::string& name, U32 width, U32 height, U32 flags, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IWindowSystem, CWin32WindowSystem, result, pEventManager, name, width, height, flags);
	}
}

#endif