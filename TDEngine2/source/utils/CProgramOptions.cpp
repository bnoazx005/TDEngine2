#include "../../include/utils/CProgramOptions.h"
#include "stringUtils.hpp"
#include <algorithm>
#include <cstdlib>


namespace TDEngine2
{
	CProgramOptions::CProgramOptions():
		CBaseObject(), mIsInternalStateInitialized(false)
	{
	}


	constexpr I32 SUCCESS_EXIT_CODE = 0;

	static const std::string SINGLE_CHAR_COMMAND_ID_PREFIX = "-";
	static const std::string LONG_COMMAND_ID_PREFIX = "--";
	static const std::string ASSIGNMENT_TOKEN = "=";


	static void PrintArgumentsInfo(const CProgramOptions::TParseArgsParams& params, const std::vector<CProgramOptions::TArgumentParams>& argumentsInfo)
	{
		std::cout << params.mProgramDescriptionStr << std::endl
			<< "Usage:\nt" << params.mpProgramUsageStr << std::endl
			<< std::endl << "Options: " << std::endl;

		for (auto&& currArgInfo : argumentsInfo)
		{
			std::cout << '\t';

			if (currArgInfo.mSingleCharCommand)
			{
				std::cout << SINGLE_CHAR_COMMAND_ID_PREFIX << currArgInfo.mSingleCharCommand << ", ";
			}

			if (!currArgInfo.mCommand.empty())
			{
				std::cout << LONG_COMMAND_ID_PREFIX << currArgInfo.mCommand << "\t\t\t";
			}

			std::cout << currArgInfo.mCommandInfo;
		}
	}


	static CProgramOptions::TArgumentParams::E_VALUE_TYPE GetStringValueType(const std::string& id)
	{
		if (strtol(id.c_str(), nullptr, 10))
		{
			return CProgramOptions::TArgumentParams::E_VALUE_TYPE::INTEGER;
		}

		if (strtof(id.c_str(), nullptr))
		{
			return CProgramOptions::TArgumentParams::E_VALUE_TYPE::FLOAT;
		}

		return CProgramOptions::TArgumentParams::E_VALUE_TYPE::STRING;
	}


	static void PackArgValueAsType(const std::string& value, CProgramOptions::TArgumentParams& param)
	{
		switch (param.mValueType)
		{
			case CProgramOptions::TArgumentParams::E_VALUE_TYPE::INTEGER:
				param.mValue = std::stoi(value);
				break;

			case CProgramOptions::TArgumentParams::E_VALUE_TYPE::FLOAT:
				param.mValue = std::stof(value);
				break;

			case CProgramOptions::TArgumentParams::E_VALUE_TYPE::STRING:
				param.mValue = value;
				break;
		}
	}


	static void InitArguments(std::vector<CProgramOptions::TArgumentParams>& argumentsInfo)
	{
		for (auto&& currArg : argumentsInfo)
		{
			switch (currArg.mValueType)
			{
				case CProgramOptions::TArgumentParams::E_VALUE_TYPE::INTEGER:
					currArg.mValue = 0;
					break;

				case CProgramOptions::TArgumentParams::E_VALUE_TYPE::FLOAT:
					currArg.mValue = 0.0f;
					break;

				case CProgramOptions::TArgumentParams::E_VALUE_TYPE::STRING:
					currArg.mValue = Wrench::StringUtils::GetEmptyStr();
					break;

				case CProgramOptions::TArgumentParams::E_VALUE_TYPE::BOOLEAN:
					currArg.mValue = false;
					break;
			}
		}
	}


	static void StoreArgumentValue(CProgramOptions::TArgumentParams& param, const std::string& valueStr)
	{
		if (GetStringValueType(valueStr) != param.mValueType)
		{
			return;
		}

		PackArgValueAsType(valueStr, param);
	}


	E_RESULT_CODE CProgramOptions::ParseArgs(const TParseArgsParams& params)
	{
		if (params.mArgsCount < 1 || !params.mpArgsValues)
		{
			return RC_INVALID_ARGS;
		}

		std::vector<std::string> args(params.mpArgsValues, params.mpArgsValues + params.mArgsCount);

		if (std::find(args.cbegin(), args.cend(), "--help") != args.cend() || std::find(args.cbegin(), args.cend(), "-h") != args.cend())
		{
			PrintArgumentsInfo(params, mArgumentsInfo);
			exit(SUCCESS_EXIT_CODE);
		}

		InitArguments(mArgumentsInfo);
		mPositionalArguments.clear();

		for (USIZE i = 0; i < args.size();)
		{
			if (Wrench::StringUtils::StartsWith(args[i], LONG_COMMAND_ID_PREFIX) || Wrench::StringUtils::StartsWith(args[i], SINGLE_CHAR_COMMAND_ID_PREFIX))
			{
				auto it = std::find_if(mArgumentsInfo.begin(), mArgumentsInfo.end(), [argName = args[i]](const TArgumentParams& param) 
				{
					return argName.find(param.mCommand, LONG_COMMAND_ID_PREFIX.size()) != std::string::npos || (argName.size() > 1 && argName[1] == param.mSingleCharCommand);
				});

				if (it == mArgumentsInfo.end())
				{
					i += 2; // skip current argument and its value, because they are not recognized
					continue;
				}

				if (it->mValueType == CProgramOptions::TArgumentParams::E_VALUE_TYPE::BOOLEAN)
				{
					it->mValue = true;
					i++;

					continue;
				}

				// check if there is '=' sign
				const USIZE pos = args[i].find(ASSIGNMENT_TOKEN);
				if (pos != std::string::npos)
				{
					StoreArgumentValue(*it, args[i++].substr(pos + 1));
					continue;
				}

				if (i + 1 >= args.size())
				{
					return RC_FAIL;
				}

				StoreArgumentValue(*it, args[++i]);
				continue;
			}

			mPositionalArguments.emplace_back(args[i++]);
		}

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
			return (arg.mCommand == params.mCommand) || (arg.mSingleCharCommand == params.mSingleCharCommand && arg.mSingleCharCommand != '\0');
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

	const std::string& CProgramOptions::GetPositionalArgValue(U32 index) const
	{
		return (index >= static_cast<U32>(mPositionalArguments.size())) ? Wrench::StringUtils::GetEmptyStr() : mPositionalArguments[index];
	}

	TPtr<CProgramOptions> CProgramOptions::Get()
	{
		static TPtr<CProgramOptions> pInstance = TPtr<CProgramOptions>(new (std::nothrow) CProgramOptions());
		return pInstance;
	}
}