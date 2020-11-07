#include "../../include/platform/CPackageFile.h"
#include "../../include/platform/MountableStorages.h"
#include "../../include/platform/IOStreams.h"
#include "../../include/core/IFile.h"
#include "../../include/core/IFileSystem.h"
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
		CBinaryFileWriter(), mCurrHeader(), mFilesTable()
	{
	}

	E_RESULT_CODE CPackageFileWriter::_onInit()
	{
		return SetPosition(sizeof(TPackageFileHeader));
	}

	E_RESULT_CODE CPackageFileWriter::_onFree()
	{
		E_RESULT_CODE result = _writeFilesTableDescription();
		result = result | _writePackageHeader();

		return result;
	}

	E_RESULT_CODE CPackageFileWriter::_writeFileInternal(TypeId fileTypeId, const std::string& path, const IFileReader& file)
	{
		if (CPackageFileWriter::GetTypeId() == fileTypeId || CPackageFileReader::GetTypeId() == fileTypeId)
		{
			LOG_WARNING("[Package File Writer] For now we don't support nested packages");
			return RC_FAIL;
		}

		IFileSystem* pFileSystem = mpStorage->GetFileSystem(); // \note Not the best idea to ask it in this way

		TPackageFileEntryInfo fileInfo;
		fileInfo.mFilename = pFileSystem->CombinePath(path, file.GetShortName());
		fileInfo.mDataBlockOffset = GetPosition();

		// Copy file's data into the package
		if (auto pStream = dynamic_cast<IInputStream*>(file.GetStream()))
		{
			E_RESULT_CODE result = RC_OK;

			fileInfo.mDataBlockSize = pStream->GetLength();

			U32 savedPosition = pStream->GetPosition();

			pStream->SetPosition(0);
			
			std::vector<U8> buffer;
			buffer.resize(fileInfo.mDataBlockSize);

			CDeferOperation _([&] 
			{
				pStream->SetPosition(savedPosition); // Restore previous position
			});

			if (RC_OK != (result = pStream->Read(&buffer[0], buffer.size())))
			{
				return result;
			}

			// Write into the package
			if (auto pPackageStream = dynamic_cast<IOutputStream*>(mpStreamImpl))
			{
				if (RC_OK != (result = pPackageStream->Write(static_cast<const void*>(&buffer[0]), buffer.size())))
				{
					return result;
				}

				buffer.clear();
			}
		}

		mFilesTable.push_back(fileInfo);
		
		return RC_OK;
	}

	E_RESULT_CODE CPackageFileWriter::_writePackageHeader()
	{
		IOutputStream* pStream = dynamic_cast<IOutputStream*>(mpStreamImpl);

		U32 prevPosition = pStream->GetPosition();

		E_RESULT_CODE result = pStream->SetPosition(0);
		result = result | pStream->Write(&mCurrHeader, sizeof(mCurrHeader));

		pStream->SetPosition(prevPosition);

		return result;
	}

	E_RESULT_CODE CPackageFileWriter::_writeFilesTableDescription()
	{
		IOutputStream* pStream = dynamic_cast<IOutputStream*>(mpStreamImpl);

		mCurrHeader.mFilesTableOffset = pStream->GetPosition();

		E_RESULT_CODE result = RC_OK;

		for (auto&& currFileEntryInfo : mFilesTable)
		{
			U64 filenameLength = currFileEntryInfo.mFilename.length();

			result = result | pStream->Write(&filenameLength, sizeof(filenameLength));
			result = result | pStream->Write(&currFileEntryInfo.mFilename[0], filenameLength * sizeof(C8));

			result = result | pStream->Write(&currFileEntryInfo.mDataBlockOffset, sizeof(currFileEntryInfo.mDataBlockOffset));
			result = result | pStream->Write(&currFileEntryInfo.mDataBlockSize, sizeof(currFileEntryInfo.mDataBlockSize));
		}

		mCurrHeader.mFilesTableSize = std::max<U64>(0, static_cast<U64>(pStream->GetPosition()) - mCurrHeader.mFilesTableOffset);
		mCurrHeader.mEntitiesCount = mFilesTable.size();

		return result;
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