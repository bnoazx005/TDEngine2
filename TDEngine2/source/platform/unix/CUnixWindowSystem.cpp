#include "./../../../include/platform/unix/CUnixWindowSystem.h"


#if defined (TDE2_USE_UNIXPLATFORM)

namespace TDEngine2
{
	CUnixWindowSystem::CUnixWindowSystem() :
		mIsInitialized(false)
	{
	}

	CUnixWindowSystem::~CUnixWindowSystem()
	{
	}

	E_RESULT_CODE CUnixWindowSystem::Init(const std::string& name, U32 width, U32 height, U32 flags)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}
		
		mWindowName = name;

		mWidth = width;
		mHeight = height;

		mSetupFlags = flags;

		mpDisplayHandler = XOpenDisplay(nullptr);

		if (!mpDisplayHandler)
		{
			return RC_FAIL;
		}

		I32 screenId = DefaultScreen(mpDisplayHandler);

		mRootWindowHandler = DefaultRootWindow(mpDisplayHandler);

		mWindowHandler = XCreateSimpleWindow(mpDisplayHandler, mRootWindowHandler, 0, 0, width, height, 1, BlackPixel(mpDisplayHandler, screenId), 
											 WhitePixel(mpDisplayHandler, screenId));

		XClearWindow(mpDisplayHandler, mWindowHandler);	// clear the window

		XMapWindow(mpDisplayHandler, mWindowHandler); // and display it

		mIsInitialized = true;

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

		return RC_OK;
	}

	bool CUnixWindowSystem::Run(const std::function<void()>& onFrameUpdate)
	{
		if (!mIsInitialized)
		{
			return false;
		}

		XEvent currEvent;

		while (mIsRunning)
		{
			XNextEvent(mpDisplayHandler, &currEvent);
		}

		return true;
	}

	E_RESULT_CODE CUnixWindowSystem::Quit()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsRunning = false;

		return RC_OK;
	}

	E_RESULT_CODE CUnixWindowSystem::SetTitle(const std::string& title)
	{
		return RC_NOT_IMPLEMENTED_YET;
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
	

	TDE2_API IWindowSystem* CreateUnixWindowSystem(const std::string& name, U32 width, U32 height, U32 flags, E_RESULT_CODE& result)
	{
		CUnixWindowSystem* pWindowSystemInstance = new (std::nothrow) CUnixWindowSystem();

		if (!pWindowSystemInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pWindowSystemInstance->Init(name, width, height, flags);

		if (result != RC_OK)
		{
			delete pWindowSystemInstance;

			pWindowSystemInstance = nullptr;
		}

		return dynamic_cast<IWindowSystem*>(pWindowSystemInstance);
	}
}

#endif