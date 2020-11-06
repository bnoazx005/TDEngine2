#include "../../include/platform/CPackageFile.h"
#include "../../include/core/IFile.h"
#include "../../include/utils/CFileLogger.h"


namespace TDEngine2
{
	/*!
		\brief CPackageFileReader's definition
	*/

	CPackageFileReader::CPackageFileReader() :
		CBinaryFileReader()
	{
	}

	IFile* CreatePackageFileReader(IMountableStorage* pStorage, IStream* pStream, E_RESULT_CODE& result)
	{
		CPackageFileReader* pFileInstance = new (std::nothrow) CPackageFileReader();

		if (!pFileInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pFileInstance->Open(pStorage, pStream);

		if (result != RC_OK)
		{
			delete pFileInstance;

			pFileInstance = nullptr;
		}

		return dynamic_cast<IFile*>(pFileInstance);
	}


	/*!
		\brief CPackageFileWriter's definition
	*/

	CPackageFileWriter::CPackageFileWriter() :
		CBinaryFileWriter()
	{
	}

	E_RESULT_CODE CPackageFileWriter::_writeFileInternal(TypeId fileTypeId, const std::string& path, const IFile& file)
	{
		if (this == &file)
		{
			LOG_WARNING("[Package File Writer] Circular dependencies within the packages are not allowed");
			return RC_FAIL;
		}

		return RC_NOT_IMPLEMENTED_YET;
	}


	IFile* CreatePackageFileWriter(IMountableStorage* pStorage, IStream* pStream, E_RESULT_CODE& result)
	{
		CPackageFileWriter* pFileInstance = new (std::nothrow) CPackageFileWriter();

		if (!pFileInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pFileInstance->Open(pStorage, pStream);

		if (result != RC_OK)
		{
			delete pFileInstance;

			pFileInstance = nullptr;
		}

		return dynamic_cast<IFile*>(pFileInstance);
	}

}