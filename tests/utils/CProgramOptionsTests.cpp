#include <catch2/catch_test_macros.hpp>
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
		REQUIRE(RC_FAIL == CProgramOptions::Get()->GetValue<bool>("flag").GetError());
	}

	SECTION("TestGetValue_PassExistingArgumentId_ReturnsItsValue")
	{
		static const C8* args[] =
		{
			"Program Path",
			"--value=42",
			"--flag"
		};

		CProgramOptions::TParseArgsParams argsParams;
		argsParams.mArgsCount = 3;
		argsParams.mpArgsValues = args;

		static const std::string argumentId = "value";

		CProgramOptions::Get()->AddArgument({ 'V', argumentId, Wrench::StringUtils::GetEmptyStr() });
		CProgramOptions::Get()->AddArgument({ '\0', "flag", Wrench::StringUtils::GetEmptyStr(), CProgramOptions::TArgumentParams::E_VALUE_TYPE::BOOLEAN });

		REQUIRE(RC_OK == CProgramOptions::Get()->ParseArgs(argsParams));

		auto&& valueResult = CProgramOptions::Get()->GetValue<I32>(argumentId);
		REQUIRE(valueResult.IsOk()); 
		REQUIRE(valueResult.Get() == 42);

		auto&& boolValueResult = CProgramOptions::Get()->GetValue<bool>("flag");
		REQUIRE(boolValueResult.IsOk());
		REQUIRE(boolValueResult.Get());
	}

	SECTION("TestGetValueOrDefault_PassArgumentThatDoesntExist_ReturnsDefaultValue")
	{
		const std::string expectedValue = "TestString";
		REQUIRE((CProgramOptions::Get()->GetValueOrDefault<std::string>("test", expectedValue) == expectedValue));
	}

	SECTION("TestParseArgs_PassArgumentWithValueAsSingleString_ReturnsRC_FAIL")
	{
		static const C8* args[] =
		{
			"Program Path",
			"-F 4"
		};

		CProgramOptions::TParseArgsParams argsParams;
		argsParams.mArgsCount = 2;
		argsParams.mpArgsValues = args;

		static const std::string argumentId = "F";

		CProgramOptions::Get()->AddArgument({ 'F', Wrench::StringUtils::GetEmptyStr(), Wrench::StringUtils::GetEmptyStr() });

		REQUIRE(RC_FAIL == CProgramOptions::Get()->ParseArgs(argsParams));
	}

	SECTION("TestGetValue_PassExistingArgumentIdBySingleCharId_ReturnsItsValue")
	{
		static const C8* args[] =
		{
			"Program Path",
			"-F",
			"4"
		};

		CProgramOptions::TParseArgsParams argsParams;
		argsParams.mArgsCount = 3;
		argsParams.mpArgsValues = args;

		static const std::string argumentId = "F";

		CProgramOptions::Get()->AddArgument({ 'F', Wrench::StringUtils::GetEmptyStr(), Wrench::StringUtils::GetEmptyStr() });

		REQUIRE(RC_OK == CProgramOptions::Get()->ParseArgs(argsParams));

		auto&& valueResult = CProgramOptions::Get()->GetValue<I32>(argumentId);
		REQUIRE((valueResult.IsOk() && valueResult.Get() == 4));
	}

	SECTION("TestGetValueOrDefault_TryGetInexistingArgumentId_ReturnsDefaultValue")
	{
		static const C8* args[] =
		{
			""
		};

		CProgramOptions::TParseArgsParams argsParams;
		argsParams.mArgsCount = 1;
		argsParams.mpArgsValues = args;

		CProgramOptions::Get()->AddArgument({ 'V', "version", Wrench::StringUtils::GetEmptyStr() });
		CProgramOptions::Get()->AddArgument({ '\0', "missing_arg", Wrench::StringUtils::GetEmptyStr(), TDEngine2::CProgramOptions::TArgumentParams::E_VALUE_TYPE::STRING });
		REQUIRE(RC_OK == CProgramOptions::Get()->ParseArgs(argsParams));

		auto&& valueResult = CProgramOptions::Get()->GetValueOrDefault<std::string>("missing_arg", "error");
		REQUIRE(valueResult == "error");
	}
}