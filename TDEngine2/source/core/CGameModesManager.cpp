#include "../../include/core/CGameModesManager.h"
#include "../../include/utils/CFileLogger.h"
#include "stringUtils.hpp"


namespace TDEngine2
{
	CGameModesManager::CGameModesManager():
		CBaseObject()
	{
	}

	E_RESULT_CODE CGameModesManager::Init() 
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CGameModesManager::SwitchMode(TPtr<IGameMode> pNewMode)
	{
		if (!pNewMode)
		{
			return RC_INVALID_ARGS;
		}

		if (auto pPrevMode = _getCurrentGameMode())
		{
			pPrevMode->OnExit();
			mModesContext.pop();
		}

		mModesContext.emplace(pNewMode);
		mModesContext.top()->OnEnter();

		return RC_OK;
	}

	void CGameModesManager::Update(F32 dt)
	{
		if (auto pCurrMode = _getCurrentGameMode())
		{
			pCurrMode->Update(dt);
		}
	}
	
	E_ENGINE_SUBSYSTEM_TYPE CGameModesManager::GetType() const
	{
		return GetTypeID();
	}

	TPtr<IGameMode> CGameModesManager::_getCurrentGameMode()
	{
		return mModesContext.empty() ? nullptr : mModesContext.top();
	}


	TDE2_API IGameModesManager* CreateGameModesManager(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGameModesManager, CGameModesManager, result);
	}


	/*!
		\brief CBaseGameMode's definition
	*/

	TPtr<IGameMode> CBaseGameMode::mDefaultGameMode = TPtr<IGameMode>(new CBaseGameMode);

	CBaseGameMode::CBaseGameMode(const std::string& name) :
		CBaseObject(), mName(name)
	{
		mIsInitialized = true;
	}

	CBaseGameMode::CBaseGameMode() :
		CBaseObject(), mName("Default Mode")
	{
		mIsInitialized = true;
	}

	void CBaseGameMode::OnEnter() 
	{
		LOG_MESSAGE(Wrench::StringUtils::Format("[BaseGameMode] Invoke OnEnter, mode: \"{0}\"", mName));
	}
	
	void CBaseGameMode::OnExit()
	{
		LOG_MESSAGE(Wrench::StringUtils::Format("[BaseGameMode] Invoke OnExit, mode: \"{0}\"", mName));
	}

	void CBaseGameMode::Update(F32 dt)
	{

	}


	/*!
		\brief CSplashScreenGameMode's definition
	*/

	TPtr<IGameMode> CSplashScreenGameMode::Instance()
	{
		static TPtr<IGameMode> pInstance = TPtr<IGameMode>(new CSplashScreenGameMode);
		return pInstance;
	}

	CSplashScreenGameMode::CSplashScreenGameMode():
		CBaseGameMode("SplashScreenMode")
	{
	}

	void CSplashScreenGameMode::OnEnter()
	{
		LOG_MESSAGE(Wrench::StringUtils::Format("[BaseGameMode] Invoke OnEnter, mode: \"{0}\"", mName));
	}

	void CSplashScreenGameMode::OnExit()
	{
		LOG_MESSAGE(Wrench::StringUtils::Format("[BaseGameMode] Invoke OnExit, mode: \"{0}\"", mName));
	}

	void CSplashScreenGameMode::Update(F32 dt)
	{

	}
}