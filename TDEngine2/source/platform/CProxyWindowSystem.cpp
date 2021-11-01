#include "../../include/platform/CProxyWindowSystem.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/utils/ITimer.h"
#if defined (TDE2_USE_WINPLATFORM)		
#include "../../include/platform/win32/CWin32Timer.h"
#elif defined (TDE2_USE_UNIXPLATFORM)
#include "../../include/platform/unix/CUnixTimer.h"
#else
#endif

namespace TDEngine2
{
	CProxyWindowSystem::CProxyWindowSystem() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CProxyWindowSystem::Init(TPtr<IEventManager> pEventManager, const std::string& name, U32 width, U32 height, U32 flags)
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

		mWidth = width;
		mHeight = height;
		mFlags = flags;

		E_RESULT_CODE result = RC_OK;

#if defined (TDE2_USE_WINPLATFORM)																/// Win32 Platform
		mpTimer = TPtr<ITimer>(CreateWin32Timer(result));
#elif defined (TDE2_USE_UNIXPLATFORM)
		mpTimer = TPtr<ITimer>(CreateUnixTimer(result));
#else
#endif
		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	bool CProxyWindowSystem::Run(const std::function<void()>& onFrameUpdate)
	{
		if (!mIsInitialized)
		{
			return false;
		}

		while (!mQuitFlag)
		{
			mpTimer->Tick();
			onFrameUpdate();
		}

		LOG_MESSAGE("[Proxy Window System] The main loop's execution was interrupted");

		return true;
	}

	E_RESULT_CODE CProxyWindowSystem::Quit()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mQuitFlag = true;
		LOG_MESSAGE("[Proxy Window System] The quit signal was received");

		return RC_OK;
	}

	E_RESULT_CODE CProxyWindowSystem::SetTitle(const std::string& title)
	{
		return RC_OK;
	}

	E_RESULT_CODE CProxyWindowSystem::ConfigureImGUIContext(IImGUIContextVisitor* pVisitor) const
	{
		return RC_FAIL;
	}

	E_ENGINE_SUBSYSTEM_TYPE CProxyWindowSystem::GetType() const
	{
		return EST_WINDOW;
	}

	const TWindowSystemInternalData& CProxyWindowSystem::GetInternalData() const
	{
		static TWindowSystemInternalData data{};
		return data;
	}

	U32 CProxyWindowSystem::GetWidth() const
	{
		return mWidth;
	}

	U32 CProxyWindowSystem::GetHeight() const
	{
		return mHeight;
	}

	TPtr<ITimer> CProxyWindowSystem::GetTimer() const
	{
		return mpTimer;
	}

	const std::string& CProxyWindowSystem::GetTitle() const
	{
		return Wrench::StringUtils::GetEmptyStr();
	}

	U32 CProxyWindowSystem::GetFlags() const
	{
		return mFlags;
	}

	TPtr<IEventManager> CProxyWindowSystem::GetEventManager() const
	{
		return mpEventManager;
	}

	TRectU32 CProxyWindowSystem::GetWindowRect() const
	{
		return {};
	}

	TRectU32 CProxyWindowSystem::GetClientRect() const
	{
		return {};
	}

#if TDE2_EDITORS_ENABLED

	TResult<std::string> CProxyWindowSystem::ShowOpenFileDialog(const std::vector<std::tuple<std::string, std::string>>& filters)
	{
		return Wrench::TErrValue<E_RESULT_CODE>(RC_OK);
	}

	TResult<std::string> CProxyWindowSystem::ShowSaveFileDialog(const std::vector<std::tuple<std::string, std::string>>& filters)
	{
		return Wrench::TErrValue<E_RESULT_CODE>(RC_OK);
	}

#endif


	TDE2_API IWindowSystem* CreateProxyWindowSystem(TPtr<IEventManager> pEventManager, const std::string& name, U32 width, U32 height, U32 flags, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IWindowSystem, CProxyWindowSystem, result, pEventManager, name, width, height, flags);
	}
}