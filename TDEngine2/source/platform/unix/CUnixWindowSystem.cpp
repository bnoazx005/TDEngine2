#include "../../../include/platform/unix/CUnixWindowSystem.h"
#include "../../../include/platform/unix/CUnixDLLManager.h"
#include "../../../include/platform/unix/CUnixTimer.h"
#include "../../../include/utils/CFileLogger.h"
#include "../../../include/core/IEventManager.h"
#include "../../../include/core/IImGUIContext.h"
#include "../../../include/core/IInputContext.h"
#include "../../../include/utils/CU8String.h"
#include <cstring>
#include <tuple>


#if defined (TDE2_USE_UNIXPLATFORM)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/types.h>
#include <unistd.h>


namespace TDEngine2
{
	CUnixWindowSystem::CUnixWindowSystem() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CUnixWindowSystem::Init(TPtr<IEventManager> pEventManager, const std::string& name, U32 width, U32 height, U32 flags)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEventManager)
		{
			return RC_INVALID_ARGS;
		}
		
		mWindowName = name;

		mWidth = width;
		mHeight = height;

		mSetupFlags = flags;

		mIsRunning = true;

		mpEventManager = pEventManager;

		mpEventManager->Subscribe(TOnWindowResized::GetTypeId(), this); 
		mpEventManager->Subscribe(TOnWindowMoved::GetTypeId(), this);

		mpDisplayHandler = XOpenDisplay(nullptr);

		if (!mpDisplayHandler)
		{
			return RC_FAIL;
		}

		I32 screenId = DefaultScreen(mpDisplayHandler);

		mRootWindowHandler = DefaultRootWindow(mpDisplayHandler);
		
		mInternalDataObject.mpDisplayHandler   = mpDisplayHandler;
		mInternalDataObject.mRootWindowHandler = mRootWindowHandler;
		mInternalDataObject.mScreenId          = screenId;

		E_RESULT_CODE result = RC_OK;

		LOG_MESSAGE("[UNIX Window System] The window system was successfully initialized");

		/// CUnixTimer's initialization
		mpTimer = CreateUnixTimer(result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}


	E_RESULT_CODE CUnixWindowSystem::_onFreeInternal()
	{
		XDestroyWindow(mpDisplayHandler, mWindowHandler);
		XCloseDisplay(mpDisplayHandler);

		LOG_MESSAGE("[UNIX Window System] The window system was successfully destroyed");

		return RC_OK;
	}


	static XIC CreateInternalInputContext(Display* pDisplay, Window windowHandle)
	{
		static const std::array<std::string, 2> locales
		{
			Wrench::StringUtils::GetEmptyStr(), 
			"@im=none",
		};

		XIM xim;
		
		for (auto&& currLocaleStr : locales)
		{
			xim = XOpenIM(pDisplay, 0, 0, 0);
			if (xim)
			{
				break;
			}
		}

		XIC xic = XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, windowHandle, XNFocusWindow, windowHandle, nullptr);
		XSetICFocus(xic);

		return xic;
	}


	bool CUnixWindowSystem::Run(const std::function<void()>& onFrameUpdate)
	{
		E_RESULT_CODE result = RC_OK;

		if (!mIsInitialized)
		{
			///< Late initialization of a window. This case happens only if
			/// OpenGL context wasn't created
			if ((result = _createWindow()) != RC_OK)
			{
				return result;
			}
		}

		if (result != RC_OK)
		{
			return result;
		}

		XSelectInput(mpDisplayHandler, mWindowHandler, StructureNotifyMask | KeyPressMask | KeyReleaseMask);
		XClearWindow(mpDisplayHandler, mWindowHandler);	// clear the window
		XMapWindow(mpDisplayHandler, mWindowHandler); // and display it

		mInputContext = CreateInternalInputContext(mpDisplayHandler, mWindowHandler);

		_setFullscreenMode(mSetupFlags & P_FULLSCREEN);

		if (!(mSetupFlags & P_RESIZEABLE))
		{
			_disableWindowResizing();
		}

		while (mIsRunning)
		{
			_processEvents();

			mpTimer->Tick();

			if (onFrameUpdate)
			{
				onFrameUpdate();
			}
		}

		LOG_MESSAGE("[UNIX Window System] The main loop's execution was interrupted");

		return true;
	}

	E_RESULT_CODE CUnixWindowSystem::Quit()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsRunning = false;

		LOG_MESSAGE("[UNIX Window System] The quit signal was received");

		return RC_OK;
	}

	E_RESULT_CODE CUnixWindowSystem::SetTitle(const std::string& title)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!XStoreName(mpDisplayHandler, mWindowHandler, title.c_str()))
		{
			return RC_FAIL;
		}

		//LOG_MESSAGE("[UNIX Window System] The window's title was changed. The new value is (" + title + ")");

		return RC_OK;
	}

	E_RESULT_CODE CUnixWindowSystem::SetScreenResolution(U32 width, U32 height)
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CUnixWindowSystem::SetIsFullscreenEnabled(bool state, bool borderlessMode)
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CUnixWindowSystem::ConfigureImGUIContext(IImGUIContextVisitor* pVisitor) const
	{
		return pVisitor->ConfigureForUnixPlatform(this);
	}

	TDE2_API E_RESULT_CODE CUnixWindowSystem::EnableOpenGL(XVisualInfo* pVisualInfo)
	{
		if (!pVisualInfo)
		{
			return RC_INVALID_ARGS;
		}

		XSetWindowAttributes windowAttributes;
		memset(&windowAttributes, 0, sizeof(windowAttributes));

		windowAttributes.colormap     = XCreateColormap(mpDisplayHandler, mRootWindowHandler, pVisualInfo->visual, AllocNone);
		windowAttributes.event_mask   = StructureNotifyMask;
		windowAttributes.border_pixel = 0;

		mWindowHandler = XCreateWindow(mpDisplayHandler, mRootWindowHandler, 0, 0, mWidth, mHeight, 0, pVisualInfo->depth, InputOutput, 
									   pVisualInfo->visual, CWColormap | CWBorderPixel | CWEventMask, &windowAttributes);

		mInternalDataObject.mWindowHandler = mWindowHandler;

		mIsInitialized = true;

		LOG_MESSAGE("[UNIX Window System] The window with OpenGL support was created");

		return SetTitle(mWindowName);
	}

	E_ENGINE_SUBSYSTEM_TYPE CUnixWindowSystem::GetType() const
	{
		return EST_WINDOW;
	}

	const TWindowSystemInternalData& CUnixWindowSystem::GetInternalData() const
	{
		return mInternalDataObject;
	}

	U32 CUnixWindowSystem::GetWidth() const
	{
		return mWidth;
	}

	U32 CUnixWindowSystem::GetHeight() const
	{
		return mHeight;
	}

	TPtr<ITimer> CUnixWindowSystem::GetTimer() const
	{
		return mpTimer;
	}

	const std::string& CUnixWindowSystem::GetTitle() const
	{
		return mWindowName;
	}

	U32 CUnixWindowSystem::GetFlags() const
	{
		return mSetupFlags;
	}

	TPtr<IEventManager> CUnixWindowSystem::GetEventManager() const
	{
		return mpEventManager;
	}			

	E_RESULT_CODE CUnixWindowSystem::OnEvent(const TBaseEvent* pEvent)
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

	TEventListenerId CUnixWindowSystem::GetListenerId() const
	{
		return static_cast<TEventListenerId>(GetTypeId());
	}

	TRectU32 CUnixWindowSystem::GetWindowRect() const
	{
		return { mWindowXPos, mWindowYPos, mWidth, mHeight };
	}

	TRectU32 CUnixWindowSystem::GetClientRect() const
	{
		XWindowAttributes windowAttributes;
		XGetWindowAttributes(mpDisplayHandler, mWindowHandler, &windowAttributes);

		return { static_cast<U32>(windowAttributes.x), static_cast<U32>(windowAttributes.y), windowAttributes.width, windowAttributes.height };
	}

	std::vector<TScreenResolutionInfo> CUnixWindowSystem::GetAvailableScreenResolutions() const
	{
		std::vector<TScreenResolutionInfo> output;

		Screen* pScreen = nullptr;
		
		const I32 screensCount = ScreenCount(mpDisplayHandler);
		for (I32 i = 0; i < screensCount; i++)
		{
			pScreen = ScreenOfDisplay(mpDisplayHandler, i);
			if (!pScreen)
			{
				continue;
			}

			const U32 width = static_cast<U32>(pScreen->width);
			const U32 height = static_cast<U32>(pScreen->height);
			const U16 refreshRate = static_cast<U16>(60);

			if (std::find_if(output.cbegin(), output.cend(), [width, height, refreshRate](const TScreenResolutionInfo& screenInfo)
			{
				return width == screenInfo.mWidth && height == screenInfo.mHeight && refreshRate == screenInfo.mRefreshRate;
			}) != output.cend())
			{
				continue;
			}

			output.push_back({ width, height, refreshRate });
		}

		return std::move(output);
	}


#if TDE2_EDITORS_ENABLED

	static TResult<std::tuple<std::string, I32>> ExecuteCommand(const std::string& command)
	{
		constexpr USIZE BufferSize = 256;
		C8 buffer[BufferSize];

		FILE* pPipe = popen(command.c_str(), "r");
		if (!pPipe)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		std::string output;

		while (!feof(pPipe))
		{
			// use buffer to read and add to result
			if (fgets(buffer, BufferSize, pPipe))
			{
				output += buffer;
			}
		}

		const I32 exitCode = pclose(pPipe);

		return Wrench::TOkValue<std::tuple<std::string, I32>>(std::tuple<std::string, I32>(output, exitCode));
	}


	static bool IsZenitySupported()
	{
		if (auto result = ExecuteCommand("which zenity"))
		{
			return std::get<I32>(result.Get()) == 0;
		}

		return false;
	}

	static bool IsKDialogSupported()
	{
		if (auto result = ExecuteCommand("which kdialog"))
		{
			return std::get<I32>(result.Get()) == 0;
		}

		return false;
	}


	static std::string GetFilterStr(const std::vector<std::tuple<std::string, std::string>>& filters)
	{
		std::string output;

		std::string filterName;
		std::string filterPattern;

		for (const auto& currFilter : filters)
		{
			std::tie(filterName, filterPattern) = currFilter;

			output
				.append(filterName)
				.append("/")
				.append(filterPattern)
				.append(" ");
		}

		return output;
	}


	TResult<std::string> CUnixWindowSystem::ShowOpenFileDialog(const std::vector<std::tuple<std::string, std::string>>& filters)
	{
		if (IsZenitySupported()) /// GNOME based dialog
		{
			if (auto result = ExecuteCommand("zenity --file-selection"))
			{
				return Wrench::TOkValue<std::string>(std::get<std::string>(result.Get()));
			}

			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (IsKDialogSupported()) /// KDE based dialog
		{
			/// filter in format filter (desc) | filter2 (desc) ... |
			if (auto result = ExecuteCommand("kdialog --getopenfilename . " + GetFilterStr(filters)))
			{
				return Wrench::TOkValue<std::string>(std::get<std::string>(result.Get()));
			}

			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		TDE2_UNIMPLEMENTED();
		return Wrench::TErrValue<E_RESULT_CODE>(RC_NOT_IMPLEMENTED_YET);
	}

	TResult<std::string> CUnixWindowSystem::ShowSaveFileDialog(const std::vector<std::tuple<std::string, std::string>>& filters)
	{
		if (IsZenitySupported()) /// GNOME based dialog
		{
			if (auto result = ExecuteCommand("zenity --file-selection --save"))
			{
				return Wrench::TOkValue<std::string>(std::get<std::string>(result.Get()));
			}

			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (IsKDialogSupported()) /// KDE based dialog
		{
			/// filter in format filter (desc) | filter2 (desc) ... |
			if (auto result = ExecuteCommand("kdialog --getsavefilename . " + GetFilterStr(filters)))
			{
				return Wrench::TOkValue<std::string>(std::get<std::string>(result.Get()));
			}

			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		TDE2_UNIMPLEMENTED();
		return Wrench::TErrValue<E_RESULT_CODE>(RC_NOT_IMPLEMENTED_YET);
	}

#endif

	TDE2_API E_RESULT_CODE CUnixWindowSystem::_createWindow()
	{
		mWindowHandler = XCreateSimpleWindow(mpDisplayHandler, mRootWindowHandler, 0, 0, mWidth, mHeight, 1, 
											 BlackPixel(mpDisplayHandler, mInternalDataObject.mScreenId),
											 WhitePixel(mpDisplayHandler, mInternalDataObject.mScreenId));

		mInternalDataObject.mWindowHandler = mWindowHandler;
		
		mIsInitialized = true;

		LOG_MESSAGE("[UNIX Window System] A simple window was created (OpenGL is disabled)");

		return SetTitle(mWindowName);
	}


	static void SendOnCharInputEvent(TPtr<IEventManager> pEventManager, TUtf8CodePoint codePoint)
	{
		TOnCharInputEvent onCharInputEvent;
		onCharInputEvent.mCharCode = static_cast<U32>(codePoint);

		pEventManager->Notify(&onCharInputEvent);
	}


	void CUnixWindowSystem::_processEvents()
	{
		XEvent currEvent;
		
		XConfigureEvent configureEvent;

		Status status;

		C8 tempTextBuf[255];
		I32 charactersCount = 0;

		while (XPending(mpDisplayHandler) > 0)
		{
			XNextEvent(mpDisplayHandler, &currEvent);
			
			switch (currEvent.type)
			{
				case ConfigureNotify:
					configureEvent = currEvent.xconfigure;

					_sendWindowMovedEvent(configureEvent.x, configureEvent.y);
					_sendWindowResizedEvent(configureEvent.width, configureEvent.height);
					
					break;

				case KeyPress:
					charactersCount = Xutf8LookupString(mInputContext, &currEvent.xkey, tempTextBuf, sizeof(tempTextBuf), nullptr, &status);

					if (status == XLookupChars)
					{
						SendOnCharInputEvent(mpEventManager, CU8String::StringToUTF8CodePoint(std::string(tempTextBuf, charactersCount)));
					}

					break;

				case DestroyNotify:
					mIsRunning = false;
					break;
			}
		}		
	}
	
	void CUnixWindowSystem::_sendWindowResizedEvent(U32 width, U32 height)
	{
		TOnWindowResized onResizedEvent;

		if (width != mWidth || height != mHeight)
		{
			onResizedEvent.mWidth  = width;
			onResizedEvent.mHeight = height;

			mpEventManager->Notify(&onResizedEvent);

			LOG_MESSAGE(std::string("[UNIX Window System] The window's sizes were changed (width: ").
						append(std::to_string(onResizedEvent.mWidth)).
						append(", height: ").
						append(std::to_string(onResizedEvent.mHeight)).
						append(")"));
		}
	}

	void CUnixWindowSystem::_sendWindowMovedEvent(U32 x, U32 y)
	{
		TOnWindowMoved onMovedEvent;

		if (x != mWindowXPos || y != mWindowYPos)
		{
			onMovedEvent.mX = x;
			onMovedEvent.mY = y;

			mpEventManager->Notify(&onMovedEvent);
		}
	}
	
	void CUnixWindowSystem::_setFullscreenMode(bool value)
	{
		XEvent event;

		event.xclient.type         = ClientMessage;
		event.xclient.serial       = 0;
		event.xclient.send_event   = 1;
		event.xclient.format       = 32;
		event.xclient.window       = mWindowHandler;
		event.xclient.message_type = XInternAtom(mpDisplayHandler, "_NET_WM_STATE", 0);
		event.xclient.data.l[0]    = value;
		event.xclient.data.l[1]    = XInternAtom(mpDisplayHandler, "_NET_WM_STATE_FULLSCREEN", 0);
		event.xclient.data.l[2]    = 0;
		event.xclient.data.l[3]    = 0;
		event.xclient.data.l[4]    = 0;

		long mask   = SubstructureRedirectMask | SubstructureNotifyMask;

		XSendEvent(mpDisplayHandler, mRootWindowHandler, 0, mask, &event);
		XFlush(mpDisplayHandler);
	}
	
	void CUnixWindowSystem::_disableWindowResizing()
	{
		XSizeHints sizeHints;
		memset(&sizeHints, 0, sizeof(sizeHints));

		sizeHints.flags      = AllHints;
		sizeHints.max_width  = mWidth;
		sizeHints.min_width  = mWidth;
		sizeHints.max_height = mHeight;
		sizeHints.min_height = mHeight;

		XSetWMNormalHints(mpDisplayHandler, mWindowHandler, &sizeHints);
		XFlush(mpDisplayHandler);
	}


	TDE2_API IWindowSystem* CreateUnixWindowSystem(TPtr<IEventManager> pEventManager, const std::string& name, U32 width, U32 height, U32 flags, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IWindowSystem, CUnixWindowSystem, result, pEventManager, name, width, height, flags);
	}
}

#endif