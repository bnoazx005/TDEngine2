#include "../../include/core/CGameModesManager.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/math/MathUtils.h"
#include "../../include/scene/ISceneManager.h"
#include "../../include/scene/IScene.h"
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


	E_RESULT_CODE CGameModesManager::PushMode(TPtr<IGameMode> pNewMode)
	{
		if (!pNewMode)
		{
			return RC_INVALID_ARGS;
		}

		mModesContext.emplace(pNewMode);
		mModesContext.top()->OnEnter();

		return RC_OK;
	}

	E_RESULT_CODE CGameModesManager::PopMode()
	{
		auto pCurrActiveMode = _getCurrentGameMode();
		if (!pCurrActiveMode)
		{
			return RC_FAIL;
		}

		pCurrActiveMode->OnExit();
		mModesContext.pop();

		/// \note Extract the latter mode if it exists
		pCurrActiveMode = _getCurrentGameMode();

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

	CBaseGameMode::CBaseGameMode(const std::string& name) :
		CBaseObject(), mName(name)
	{
	}

	CBaseGameMode::CBaseGameMode() :
		CBaseObject(), mName("Default Mode")
	{
	}

	E_RESULT_CODE CBaseGameMode::Init(IGameModesManager* pOwner)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pOwner)
		{
			return RC_INVALID_ARGS;
		}

		mpOwner = pOwner;

		mIsInitialized = true;

		return RC_OK;
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


	TDE2_API IGameMode* CreateBaseGameMode(IGameModesManager* pOwner, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGameMode, CBaseGameMode, result, pOwner);
	}


	/*!
		\brief CSplashScreenGameMode's definition
	*/

	CSplashScreenGameMode::CSplashScreenGameMode():
		CBaseGameMode("SplashScreenMode"), mSplashScreenSceneHandle(TSceneId::Invalid)
	{
	}

	void CSplashScreenGameMode::OnEnter()
	{
		LOG_MESSAGE(Wrench::StringUtils::Format("[BaseGameMode] Invoke OnEnter, mode: \"{0}\"", mName));
		mCurrTime = 0.0f;

		/// \todo Load splash screen's scene
		if (auto loadSceneResult = mpSceneManager->LoadScene("ProjectResources/Scenes/SplashScreen.scene"))
		{
			mSplashScreenSceneHandle = loadSceneResult.Get();
		}
	}

	void CSplashScreenGameMode::OnExit()
	{
		LOG_MESSAGE(Wrench::StringUtils::Format("[BaseGameMode] Invoke OnExit, mode: \"{0}\"", mName));
		
		E_RESULT_CODE result = mpSceneManager->UnloadScene(mSplashScreenSceneHandle);
		TDE2_ASSERT(RC_OK == result);
	}

	void CSplashScreenGameMode::Update(F32 dt)
	{
		mCurrTime += dt;

		if (CMathUtils::IsGreatOrEqual(mCurrTime, mShowDuration))
		{
			E_RESULT_CODE result = mpOwner->PopMode();
			TDE2_ASSERT(RC_OK == result);

			return;
		}
	}


	TDE2_API IGameMode* CreateSplashScreenGameMode(IGameModesManager* pOwner, TPtr<ISceneManager> pSceneManager, const TSplashScreenModeParams& params, E_RESULT_CODE& result)
	{
		if (auto pMode = CREATE_IMPL(CSplashScreenGameMode, CSplashScreenGameMode, result, pOwner))
		{
			pMode->mpSceneManager = pSceneManager;
			pMode->mShowDuration = params.mShowDuration;

			return dynamic_cast<IGameMode*>(pMode);
		}

		return nullptr;
	}
}