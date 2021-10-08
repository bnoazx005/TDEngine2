#include "../../include/core/CProjectSettings.h"
#include "../../include/core/IFile.h"


namespace TDEngine2
{
	CProjectSettings::CProjectSettings():
		CBaseObject()
	{
	}

	E_RESULT_CODE CProjectSettings::Init(IArchiveReader* pFileReader)
	{
		if (!pFileReader)
		{
			return RC_INVALID_ARGS;
		}

		/// \todo Add initialization of project's settings here

		return RC_OK;
	}

	TDE2_API TPtr<CProjectSettings> CProjectSettings::Get()
	{
		static TPtr<CProjectSettings> pInstance = TPtr<CProjectSettings>(new (std::nothrow) CProjectSettings());
		return pInstance;
	}
}