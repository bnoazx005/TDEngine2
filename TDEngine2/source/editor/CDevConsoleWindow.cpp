#include "../../include/editor/CDevConsoleWindow.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/core/IInputContext.h"
#include "../../include/core/CGameUserSettings.h"
#include "../../include/utils/CFileLogger.h"
#include "stringUtils.hpp"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	static std::tuple<std::string, bool> SetConsoleVariableCommand(const std::vector<std::string>& args)
	{
		if (args.size() != 2)
		{
			return std::make_tuple(Wrench::StringUtils::Format("Expected 2 arguments <var_name> <var_value> for command set, found {0}", args.size()), true);
		}

		E_RESULT_CODE result = CreateCVarFromString(args.front(), args.back());
		if (RC_OK != result)
		{
			return std::make_tuple("Cannot create a variable", true);
		}

		return std::make_tuple(Wrench::StringUtils::Format("Variable \"{0}\" successfully updated", args.front()), false);
	}


	CDevConsoleWindow::CDevConsoleWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CDevConsoleWindow::Init(TPtr<IDesktopInputContext> pInputContext)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pInputContext)
		{
			return RC_INVALID_ARGS;
		}

		mpInputContext = pInputContext;

		// \note Register built-in commands
		RegisterCommand("clear", [this](auto&&) { ClearHistory(); return std::make_tuple(Wrench::StringUtils::GetEmptyStr(), false); });
		RegisterCommand("set", SetConsoleVariableCommand);

		mIsInitialized = true;

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

		auto&& outputInfo = iter->second(args);
		_writeToLog(std::get<std::string>(outputInfo), std::get<bool>(outputInfo));

		return RC_OK;
	}

	void CDevConsoleWindow::_onUpdate(F32 dt)
	{
		CBaseEditorWindow::_onUpdate(dt);

		if (mpInputContext->IsKey(E_KEYCODES::KC_LCONTROL) && mpInputContext->IsKeyPressed(E_KEYCODES::KC_F1))
		{
			mIsVisible = !mIsVisible;
		}

		if (!mIsVisible)
		{
			mAnimationTime = 0.0f;
		}
	}

	void CDevConsoleWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static constexpr F32 ConsoleBackgroundHeightRatio = 0.4f; // relative to display's sizes
		static constexpr F32 SeparatorSize = 20.0f; // between messages and input field
		static constexpr F32 InputFieldHeight = 20.0f;
		static constexpr F32 DropDownAnimationDuration = 0.15f;
		static constexpr TColor32F ConsoleBackgroundColor = TColor32F(0.0f, 0.0f, 0.0f, 0.95f);

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			ZeroVector2,
			ZeroVector2,
			ZeroVector2,
			false,
			true,
			false,
			true,
			true,
			false,
			1.5f
		};

		std::string message;
		bool isErrorMessage = false;

		static const TColor32F errorTextColor { 1.0f, 0.0f, 0.0f, 1.0f };
		static const TColor32F mainTextColor { 1.0f, 1.0f, 1.0f, 1.0f };

		auto processCommand = [this]()
		{
			std::vector<std::string> args;

			std::string intpuArgsStr = mCurrInputBuffer;
			while (!intpuArgsStr.empty())
			{
				USIZE escapeSequenceStartPos = intpuArgsStr.find_first_of('\"');

				auto&& tokens = Wrench::StringUtils::Split(intpuArgsStr.substr(0, escapeSequenceStartPos), " ");
				std::copy(tokens.cbegin(), tokens.cend(), std::back_inserter(args));

				USIZE escapeSequenceLastPos = intpuArgsStr.find_first_of('\"', escapeSequenceStartPos + 1);
				if (escapeSequenceStartPos != std::string::npos)
				{
					args.push_back(intpuArgsStr.substr(escapeSequenceStartPos + 1, escapeSequenceLastPos - escapeSequenceStartPos - 1));
					intpuArgsStr = intpuArgsStr.substr(escapeSequenceLastPos + 1);

					continue;
				}

				break;
			}

			if (args.empty())
			{
				_writeToLog("Can't evaluate an empty line", true);
				return;
			}

			_writeToLog(mCurrInputBuffer);
			if (ExecuteCommand(args[0], { args.cbegin() + 1, args.cend() }) != RC_OK)
			{
				_writeToLog(Wrench::StringUtils::Format("Unrecognized command: {0}", args[0]), true);
			}

			mCurrInputBuffer.clear();
		};

		if (mpImGUIContext->BeginWindow("Dev Console", isEnabled, params))
		{
			const TVector2 consoleSizes(mpImGUIContext->GetWindowWidth(), CMathUtils::Lerp(0.0f, ConsoleBackgroundHeightRatio, mAnimationTime / DropDownAnimationDuration) * mpImGUIContext->GetWindowHeight());

			mpImGUIContext->DrawRect(TRectF32(ZeroVector2, consoleSizes), ConsoleBackgroundColor);

			if (consoleSizes.y > InputFieldHeight * 2.0f)
			{
				if (mpImGUIContext->BeginChildWindow("LogRegion", consoleSizes - TVector2(0.0f, SeparatorSize + InputFieldHeight)))
				{
					for (auto iter = mLog.cbegin() + mCurrPos; iter != mLog.cend(); ++iter)
					{
						std::tie(message, isErrorMessage) = *iter;
						mpImGUIContext->Label(Wrench::StringUtils::Format("# {0} {1}", isErrorMessage ? "[Error]" : Wrench::StringUtils::GetEmptyStr(), message), isErrorMessage ? errorTextColor : mainTextColor);
					}
				}
				mpImGUIContext->EndChildWindow();

				mpImGUIContext->SetItemWidth(consoleSizes.x, [this, processCommand]
					{
						mpImGUIContext->TextField(Wrench::StringUtils::GetEmptyStr(), mCurrInputBuffer, [processCommand](auto&&) { processCommand(); }, nullptr, true);
					});
			}
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;

		mAnimationTime = CMathUtils::Clamp(0.0f, DropDownAnimationDuration, mAnimationTime + mCurrDeltaTime);
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


	TDE2_API IEditorWindow* CreateDevConsoleWindow(TPtr<IDesktopInputContext> pInputContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CDevConsoleWindow, result, pInputContext);
	}
}

#endif