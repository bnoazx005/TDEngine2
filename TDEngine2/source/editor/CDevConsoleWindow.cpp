#include "./../../include/editor/CDevConsoleWindow.h"
#include "./../../include/core/IImGUIContext.h"
#include "./../../include/utils/CFileLogger.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CDevConsoleWindow::CDevConsoleWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CDevConsoleWindow::Init()
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		// \note Register built-in commands
		RegisterCommand("clear", [this](auto&&) { ClearHistory(); return CStringUtils::GetEmptyStr(); });

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CDevConsoleWindow::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CDevConsoleWindow::RegisterCommand(const std::string& commandName, const TCommandHandler& commandHandler)
	{
		if (commandName.empty() || !commandHandler)
		{
			return RC_INVALID_ARGS;
		}

		if (mRegisteredCommands.find(commandName) != mRegisteredCommands.cend())
		{
			return RC_FAIL;
		}

		mRegisteredCommands.emplace(commandName, commandHandler);

		return RC_OK;
	}

	E_RESULT_CODE CDevConsoleWindow::UnregisterCommand(const std::string& commandName)
	{
		auto&& iter = mRegisteredCommands.find(commandName);
		if (iter == mRegisteredCommands.cend())
		{
			return RC_FAIL;
		}

		mRegisteredCommands.erase(iter);

		return RC_OK;
	}

	E_RESULT_CODE CDevConsoleWindow::ExecuteCommand(const std::string& command, const TStringsArray& args)
	{
		auto iter = mRegisteredCommands.find(command);
		if (command.empty() || (iter == mRegisteredCommands.cend()))
		{
			return RC_FAIL;
		}

		_writeToLog(iter->second(args));
		return RC_OK;
	}

	void CDevConsoleWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(500.0f, 300.0f),
			TVector2(500.0f, 300.0f),
		};

		std::string message;
		bool isErrorMessage = false;

		static const TColor32F errorTextColor { 1.0f, 0.0f, 0.0f, 1.0f };
		static const TColor32F mainTextColor { 1.0f, 1.0f, 1.0f, 1.0f };

		auto processCommand = [this]()
		{
			auto&& tokens = CStringUtils::Split(mCurrInputBuffer, " ");
			if (tokens.empty())
			{
				_writeToLog("#[Error] Can't evaluate an empty line", true);
				return;
			}

			_writeToLog(CStringUtils::Format("#{0}", mCurrInputBuffer));
			if (ExecuteCommand(tokens[0], { tokens.cbegin() + 1, tokens.cend() }) != RC_OK)
			{
				_writeToLog(CStringUtils::Format("#[Error] Unrecognized command: {0}", tokens[0]), true);
			}

			mCurrInputBuffer.clear();
		};
		
		if (mpImGUIContext->BeginWindow("Dev Console", isEnabled, params))
		{
			if (mpImGUIContext->BeginChildWindow("LogRegion", { 480.0f, 235.0f }))
			{
				for (auto iter = mLog.cbegin() + mCurrPos; iter != mLog.cend(); ++iter)
				{
					std::tie(message, isErrorMessage) = *iter;
					mpImGUIContext->Label(message, isErrorMessage ? errorTextColor : mainTextColor);
				}
			}
			mpImGUIContext->EndChildWindow();

			if (mpImGUIContext->BeginChildWindow("Toolbar", { 480.0f, 20.0f }))
			{
				// Draw input field and send button
				mpImGUIContext->BeginHorizontal();
				{
					mpImGUIContext->TextField(CStringUtils::GetEmptyStr(), mCurrInputBuffer, processCommand);
					mpImGUIContext->Button("Run", { 100.0f, 20.0f }, processCommand);
				}
				mpImGUIContext->EndHorizontal();
			}
			mpImGUIContext->EndChildWindow();
			
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;
	}

	void CDevConsoleWindow::_writeToLog(const std::string& message, bool isError)
	{
		mLog.emplace_back(message, isError);
		LOG_MESSAGE(message);
	}

	void CDevConsoleWindow::ClearHistory()
	{
		mLog.clear();
		mCurrPos = 0;
	}


	TDE2_API IEditorWindow* CreateDevConsoleWindow(E_RESULT_CODE& result)
	{
		CDevConsoleWindow* pEditorInstance = new (std::nothrow) CDevConsoleWindow();

		if (!pEditorInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pEditorInstance->Init();

		if (result != RC_OK)
		{
			delete pEditorInstance;

			pEditorInstance = nullptr;
		}

		return dynamic_cast<IEditorWindow*>(pEditorInstance);
	}
}

#endif