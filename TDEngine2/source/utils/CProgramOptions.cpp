#include "../../include/utils/CProgramOptions.h"
#include "argparse.h"
#include "stringUtils.hpp"
#include <algorithm>


namespace TDEngine2
{
	CProgramOptions::CProgramOptions():
		CBaseObject(), mIsInternalStateInitialized(false)
	{
	}


	static argparse_option ConvertArgumentToOption(CProgramOptions::TArgumentParams& arg)
	{
		auto& value = arg.mValue;

		if (value.Is<I32>())
		{
			return OPT_INTEGER(arg.mSingleCharCommand, arg.mCommand.c_str(), &value.As<I32>(), arg.mCommandInfo.c_str());
		}

		if (value.Is<F32>())
		{
			return OPT_FLOAT(arg.mSingleCharCommand, arg.mCommand.c_str(), &value.As<F32>(), arg.mCommandInfo.c_str());
		}

		if (value.Is<bool>())
		{
			return OPT_BOOLEAN(arg.mSingleCharCommand, arg.mCommand.c_str(), &value.As<bool>(), arg.mCommandInfo.c_str());
		}

		if (value.Is<std::string>())
		{
			return OPT_STRING(arg.mSingleCharCommand, arg.mCommand.c_str(), &value.As<std::string>(), arg.mCommandInfo.c_str());
		}

		return OPT_END();
	}


	E_RESULT_CODE CProgramOptions::ParseArgs(const TParseArgsParams& params)
	{
		if (params.mArgsCount < 1 || !params.mpArgsValues)
		{
			return RC_INVALID_ARGS;
		}

		argparse argparse;

		const C8* pUsage[] =
		{
			params.mpProgramUsageStr.c_str(), nullptr
		};

		std::vector<argparse_option> internalOptions;
		internalOptions.push_back(OPT_HELP());

		std::transform(mArgumentsInfo.begin(), mArgumentsInfo.end(), std::back_inserter(internalOptions), ConvertArgumentToOption);

		internalOptions.push_back(OPT_END());

		argparse_init(&argparse, &internalOptions.front(), pUsage, 0);
		argparse_describe(&argparse, params.mProgramDescriptionStr.c_str(), nullptr);
		I32 argc = argparse_parse(&argparse, params.mArgsCount, params.mpArgsValues);

		mIsInternalStateInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CProgramOptions::AddArgument(const TArgumentParams& params)
	{
		if (mIsInternalStateInitialized)
		{
			return RC_FAIL;
		}

		std::string commandName = Wrench::StringUtils::RemoveAllWhitespaces(params.mCommand);
		if (Wrench::StringUtils::StartsWith(commandName, "--"))
		{
			commandName = commandName.substr(2);
		}

		auto it = std::find_if(mArgumentsInfo.cbegin(), mArgumentsInfo.cend(), [&params](const TArgumentParams& arg)
		{
			return (arg.mCommand == params.mCommand) || (arg.mSingleCharCommand == params.mSingleCharCommand);
		});

		if (it != mArgumentsInfo.cend())
		{
			return RC_INVALID_ARGS;
		}

		mArgumentsInfo.push_back(params);
		return RC_OK;
	}

	void CProgramOptions::Reset()
	{
		mIsInternalStateInitialized = false;
		mArgumentsInfo.clear();
	}

	TPtr<CProgramOptions> CProgramOptions::Get()
	{
		static TPtr<CProgramOptions> pInstance = TPtr<CProgramOptions>(new (std::nothrow) CProgramOptions());
		return pInstance;
	}
}