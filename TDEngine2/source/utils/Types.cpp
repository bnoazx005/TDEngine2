#include "./../../include/utils/Types.h"


namespace TDEngine2
{
	const TRegisterLoaderResult TRegisterLoaderResult::mInvalidValue = { RC_FAIL, InvalidResourceLoaderId };

	const TShaderCompilerResult TShaderCompilerResult::mInvalidValue = TShaderCompilerResult(RC_FAIL);
	
	TShaderCompilerResult::TShaderCompilerResult() :
		mResultCode(RC_OK), mVSByteCode(), mPSByteCode(), mGSByteCode()
	{
	}

	TShaderCompilerResult::TShaderCompilerResult(E_RESULT_CODE errorCode):
		mResultCode(errorCode)
	{
	}
}