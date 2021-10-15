#include "../../include/platform/CPackageFile.h"
#include "../../include/platform/MountableStorages.h"
#include "../../include/platform/IOStreams.h"
#include "../../include/core/IFile.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/utils/CFileLogger.h"
#define DEFER_IMPLEMENTATION
#include "deferOperation.hpp"
#include <algorithm>


namespace TDEngine2
{
	/*!
		\brief CPackageFileReader's definition
	*/

	CPackageFileReader::CPackageFileReader() :
		CBinaryFileReader()
	{
	}

	std::vector<U8> CPackageFileReader::ReadFileBytes(const std::string& path)
	{
		auto iter = std::find_if(mFilesTable.cbegin(), mFilesTable.cend(), [path](auto&& entry) { return entry.mFilename == path; });
		if (iter == mFilesTable.cend())
		{
			return {};
		}

		std::vector<U8> dataBuffer;
		dataBuffer.resize(static_cast<size_t>(iter->mDataBlockSize));

		IInputStream* pStream = dynamic_cast<IInputStream*>(mpStreamImpl);
		
		TSizeType prevPosition = pStream->GetPosition();
		{
			pStream->SetPosition(static_cast<TSizeType>(iter->mDataBlockOffset));
			pStream->Read(&dataBuffer[0], static_cast<TSizeType>(iter->mDataBlockSize));
		}

		pStream->SetPosition(prevPosition);

		return std::move(dataBuffer);
	}

	const TPackageFileHeader& CPackageFileReader::GetPackageHeader() const
	{
		return mCurrHeader;
	}

	const std::vector<TPackageFileEntryInfo>& CPackageFileReader::GetFilesTable() const
	{
		return mFilesTable;
	}

	E_RESULT_CODE CPackageFileReader::_onInit()
	{
		E_RESULT_CODE result = _readPackageHeader();
		result = result | _readFilesTableDescription();

		return RC_OK;
	}

	E_RESULT_CODE CPackageFileReader::_readPackageHeader()
	{
		IInputStream* pStream = dynamic_cast<IInputStream*>(mpStreamImpl);

		return pStream->Read(&mCurrHeader, sizeof(mCurrHeader));
	}

	E_RESULT_CODE CPackageFileReader::_readFilesTableDescription()
	{
		IInputStream* pStream = dynamic_cast<IInputStream*>(mpStreamImpl);

		E_RESULT_CODE result = pStream->SetPosition(static_cast<U32>(mCurrHeader.mFilesTableOffset));

		U64 filenameLength = 0;
		TPackageFileEntryInfo info;

		for (U32 i = 0; i < mCurrHeader.mEntitiesCount; ++i)
		{
			result = result | pStream->Read(&filenameLength, sizeof(filenameLength));

			info.mFilename.resize(static_cast<size_t>(filenameLength));
			result = result | pStream->Read(&info.mFilename[0], static_cast<U32>(sizeof(C8) * filenameLength));

			result = result | pStream->Read(&info.mDataBlockOffset, sizeof(info.mDataBlockOffset));
			result = result | pStream->Read(&info.mDataBlockSize, sizeof(info.mDataBlockSize));

			mFilesTable.push_back(info);
		}

		return result;
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
		return mpStreamImpl->SetPosition(sizeof(TPackageFileHeader));
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
		fileInfo.mFilename = path.empty() ? file.GetShortName() : pFileSystem->CombinePath(path, file.GetShortName());
		fileInfo.mDataBlockOffset = GetPosition();

		// Copy file's data into the package
		if (auto pStream = dynamic_cast<IInputStream*>(file.GetStream()))
		{
			E_RESULT_CODE result = RC_OK;

			fileInfo.mDataBlockSize = pStream->GetLength();

			TSizeType savedPosition = pStream->GetPosition();

			pStream->SetPosition(0);
			
			std::vector<U8> buffer;
			buffer.resize(static_cast<size_t>(fileInfo.mDataBlockSize));

			defer([&] 
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

		TSizeType prevPosition = pStream->GetPosition();

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
			result = result | pStream->Write(&currFileEntryInfo.mFilename[0], static_cast<U32>(filenameLength * sizeof(C8)));

			result = result | pStream->Write(&currFileEntryInfo.mDataBlockOffset, sizeof(currFileEntryInfo.mDataBlockOffset));
			result = result | pStream->Write(&currFileEntryInfo.mDataBlockSize, sizeof(currFileEntryInfo.mDataBlockSize));
		}

		mCurrHeader.mFilesTableSize = std::max<U64>(0, static_cast<U64>(pStream->GetPosition()) - mCurrHeader.mFilesTableOffset);
		mCurrHeader.mEntitiesCount = static_cast<U32>(mFilesTable.size());

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