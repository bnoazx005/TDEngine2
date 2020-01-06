#include "./../../../include/platform/unix/CUnixWindowSystem.h"
#include "./../../../include/platform/unix/CUnixDLLManager.h"
#include "./../../../include/platform/unix/CUnixTimer.h"
#include "./../../../include/utils/CFileLogger.h"
#include "./../../../include/core/IEventManager.h"
#include "./../../../include/core/IImGUIContext.h"
#include <cstring>


#if defined (TDE2_USE_UNIXPLATFORM)

namespace TDEngine2
{
	CUnixWindowSystem::CUnixWindowSystem() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CUnixWindowSystem::Init(IEventManager* pEventManager, const std::string& name, U32 width, U32 height, U32 flags)
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

		/// CUnix2DLLManager's initialization
		mpDLLManager = CreateUnixDLLManager(result);

		if (result != RC_OK)
		{
			return result;
		}
		
		return RC_OK;
	}


	E_RESULT_CODE CUnixWindowSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_OK;
		}

		XDestroyWindow(mpDisplayHandler, mWindowHandler);

		XCloseDisplay(mpDisplayHandler);

		mIsInitialized = false;

		delete this;

		LOG_MESSAGE("[UNIX Window System] The window system was successfully destroyed");

		return RC_OK;
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

		_setFullscreenMode(mSetupFlags & P_FULLSCREEN);

		if (!(mSetupFlags & P_RESIZEABLE))
		{
			_disableWindowResizing();
		}

		while (mIsRunning)
		{
			_processEvents();

			onFrameUpdate();

			mpTimer->Tick();
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

	ITimer* CUnixWindowSystem::GetTimer() const
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

	IDLLManager* CUnixWindowSystem::GetDLLManagerInstance() const
	{
		return mpDLLManager;
	}

	IEventManager* CUnixWindowSystem::GetEventManager() const
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
		return GetTypeId();
	}

	TRectU32 CUnixWindowSystem::GetWindowRect() const
	{
		return { mWindowXPos, mWindowYPos, mWidth, mHeight };
	}

	TRectU32 CUnixWindowSystem::GetClientRect() const
	{
		TDE2_UNIMPLEMENTED();
		return { 0, 0, 0, 0 };
	}

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

	void CUnixWindowSystem::_processEvents()
	{
		XEvent currEvent;
		
		XConfigureEvent configureEvent;

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


	TDE2_API IWindowSystem* CreateUnixWindowSystem(IEventManager* pEventManager, const std::string& name, U32 width, U32 height, U32 flags, E_RESULT_CODE& result)
	{
		CUnixWindowSystem* pWindowSystemInstance = new (std::nothrow) CUnixWindowSystem();

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