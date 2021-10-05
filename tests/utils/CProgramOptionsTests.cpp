#include <catch2/catch.hpp>
#include <TDEngine2.h>

using namespace TDEngine2;


TEST_CASE("CProgramOptions Tests")
{
	CProgramOptions::Get()->Reset();

	SECTION("TestParseArgs_ExecutedOnEmptyParamsSet_ReturnsRC_INVALID_ARGS")
	{
		REQUIRE(RC_INVALID_ARGS == CProgramOptions::Get()->ParseArgs({}));
	}

	SECTION("TestAddArgument_PassCorrectParamsButNoArgumentsWereProvided_ReturnsRC_OK")
	{
		static const C8* args[] =
		{
			"Arg0",
			"Arg1"
		};

		CProgramOptions::TParseArgsParams argsParams;
		argsParams.mArgsCount = 2;
		argsParams.mpArgsValues = args;
		argsParams.mpProgramUsageStr = "Empty Usage";
		argsParams.mProgramDescriptionStr = "Empty Description";

		REQUIRE(RC_OK == CProgramOptions::Get()->ParseArgs(argsParams));
	}

	SECTION("TestAddArgument_PassCorrectParams_ReturnsRC_OK")
	{
		static const C8* args[] = 
		{
			"Arg0",
			"Arg1"
		};

		CProgramOptions::TParseArgsParams argsParams;
		argsParams.mArgsCount = 2;
		argsParams.mpArgsValues = args;
		argsParams.mpProgramUsageStr = "Empty Usage";
		argsParams.mProgramDescriptionStr = "Empty Description";

		CProgramOptions::Get()->AddArgument({ 'V', "version", Wrench::StringUtils::GetEmptyStr() });

		REQUIRE(RC_OK == CProgramOptions::Get()->ParseArgs(argsParams));
	}

	SECTION("TestAddArgument_InvokesMethodAfterParseArgs_ReturnsRC_FAIL")
	{
		static const C8* args[] =
		{
			"Arg0",
			"Arg1"
		};

		CProgramOptions::TParseArgsParams argsParams;
		argsParams.mArgsCount = 2;
		argsParams.mpArgsValues = args;
		argsParams.mpProgramUsageStr = "Empty Usage";
		argsParams.mProgramDescriptionStr = "Empty Description";

		REQUIRE(RC_OK == CProgramOptions::Get()->ParseArgs(argsParams));
		REQUIRE(RC_FAIL == CProgramOptions::Get()->AddArgument({})); /// \note RC_FAIL doesn't depent on input if AddArgument is invoked after ParseArgs
	}

	SECTION("TestAddArgument_PassDuplicatedCommand_ReturnsRC_INVALID_ARGS")
	{
		REQUIRE(RC_OK == CProgramOptions::Get()->AddArgument({ 'V', "version", Wrench::StringUtils::GetEmptyStr() }));
		REQUIRE(RC_INVALID_ARGS == CProgramOptions::Get()->AddArgument({ 'V', "variable", Wrench::StringUtils::GetEmptyStr() }));
		REQUIRE(RC_INVALID_ARGS == CProgramOptions::Get()->AddArgument({ '0', "version", Wrench::StringUtils::GetEmptyStr() }));
	}

	SECTION("TestGetValue_PassArgumentThatDoesntExist_ReturnsRC_FAIL")
	{
		REQUIRE(RC_FAIL == CProgramOptions::Get()->GetValue<I32>("test").GetError());
	}

	SECTION("TestGetValue_PassExistingArgumentId_ReturnsItsValue")
	{
		static const C8* args[] =
		{
			"Program Path",
			"--value=42"
		};

		CProgramOptions::TParseArgsParams argsParams;
		argsParams.mArgsCount = 2;
		argsParams.mpArgsValues = args;

		static const std::string argumentId = "value";

		CProgramOptions::Get()->AddArgument({ 'V', argumentId, Wrench::StringUtils::GetEmptyStr() });

		REQUIRE(RC_OK == CProgramOptions::Get()->ParseArgs(argsParams));

		auto&& valueResult = CProgramOptions::Get()->GetValue<I32>(argumentId);
		REQUIRE((valueResult.IsOk() && valueResult.Get() == 42));
	}

	SECTION("TestGetValueOrDefault_PassArgumentThatDoesntExist_ReturnsDefaultValue")
	{
		const std::string expectedValue = "TestString";
		REQUIRE((CProgramOptions::Get()->GetValueOrDefault<std::string>("test", expectedValue) == expectedValue));
	}
}