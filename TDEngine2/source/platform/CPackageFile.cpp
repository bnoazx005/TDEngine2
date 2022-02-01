#include "../../include/platform/CPackageFile.h"
#include "../../include/platform/MountableStorages.h"
#include "../../include/platform/IOStreams.h"
#include "../../include/core/IFile.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/utils/CFileLogger.h"
#define DEFER_IMPLEMENTATION
#include "deferOperation.hpp"
#include <algorithm>
#include "stringUtils.hpp"
#include "zlib.h"


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
		dataBuffer.resize(iter->mIsCompressed ? static_cast<USIZE>(iter->mCompressedBlockSize) : static_cast<USIZE>(iter->mDataBlockSize));

		TPtr<IInputStream> pStream = DynamicPtrCast<IInputStream>(mpStreamImpl);
		
		TSizeType prevPosition = pStream->GetPosition();
		{
			pStream->SetPosition(static_cast<TSizeType>(iter->mDataBlockOffset));
			pStream->Read(&dataBuffer[0], dataBuffer.size() * sizeof(U8));
		}

		pStream->SetPosition(prevPosition);

		/// \note Make decompression if the file was archived previously
		if (iter->mIsCompressed)
		{
			std::vector<U8> decompressedBufferBlock;
			decompressedBufferBlock.resize(static_cast<USIZE>(iter->mDataBlockSize));

			uLongf decompressedDataSize = static_cast<uLongf>(iter->mDataBlockSize);

			if (Z_OK != uncompress(&decompressedBufferBlock.front(), &decompressedDataSize, &dataBuffer.front(), static_cast<uLong>(iter->mCompressedBlockSize)))
			{
				return {};
			}

			return std::move(decompressedBufferBlock);
		}

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
		TPtr<IInputStream> pStream = DynamicPtrCast<IInputStream>(mpStreamImpl);

		E_RESULT_CODE result = RC_OK;

		C8 tag[4];
		U16 version, padding;

		result = result | pStream->Read(&tag, sizeof(tag));
		result = result | pStream->Read(&version, sizeof(version));
		result = result | pStream->Read(&padding, sizeof(padding));

		version = SwapBytes(version);

		if (strcmp(tag, TPackageFileHeader::mTag) != 0 || version != TPackageFileHeader::mVersion)
		{
			LOG_ERROR(Wrench::StringUtils::Format("[CPackageFileReader] Invalid package was found at ({0}", pStream->GetName()));
			TDE2_ASSERT(false);

			return RC_INVALID_FILE;
		}

		result = result | pStream->Read(&mCurrHeader.mEntitiesCount, sizeof(mCurrHeader.mEntitiesCount));
		mCurrHeader.mEntitiesCount = SwapBytes(mCurrHeader.mEntitiesCount);

		result = result | pStream->Read(&mCurrHeader.mFilesTableOffset, sizeof(mCurrHeader.mFilesTableOffset));
		mCurrHeader.mFilesTableOffset = SwapBytes(mCurrHeader.mFilesTableOffset);

		result = result | pStream->Read(&mCurrHeader.mFilesTableSize, sizeof(mCurrHeader.mFilesTableSize));
		mCurrHeader.mFilesTableSize = SwapBytes(mCurrHeader.mFilesTableSize);

		return result;
	}

	E_RESULT_CODE CPackageFileReader::_readFilesTableDescription()
	{
		TPtr<IInputStream> pStream = DynamicPtrCast<IInputStream>(mpStreamImpl);

		E_RESULT_CODE result = pStream->SetPosition(static_cast<U32>(mCurrHeader.mFilesTableOffset));

		U64 filenameLength = 0;
		TPackageFileEntryInfo info;

		for (U32 i = 0; i < mCurrHeader.mEntitiesCount; ++i)
		{
			result = result | pStream->Read(&filenameLength, sizeof(filenameLength));

			info.mFilename.resize(static_cast<size_t>(SwapBytes(filenameLength)));
			result = result | pStream->Read(&info.mFilename[0], static_cast<U32>(sizeof(C8) * filenameLength));

			result = result | pStream->Read(&info.mDataBlockOffset, sizeof(info.mDataBlockOffset));
			info.mDataBlockOffset = SwapBytes(info.mDataBlockOffset);

			result = result | pStream->Read(&info.mDataBlockSize, sizeof(info.mDataBlockSize));
			info.mDataBlockSize = SwapBytes(info.mDataBlockSize);

			result = result | pStream->Read(&info.mCompressedBlockSize, sizeof(info.mCompressedBlockSize));
			info.mCompressedBlockSize = SwapBytes(info.mCompressedBlockSize);

			result = result | pStream->Read(&info.mIsCompressed, sizeof(info.mIsCompressed));

			mFilesTable.push_back(info);
		}

		return result;
	}


	IFile* CreatePackageFileReader(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result)
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

	E_RESULT_CODE CPackageFileWriter::_writeFileInternal(TypeId fileTypeId, const std::string& path, const IFileReader& file, bool useCompression)
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
		if (auto pStream = DynamicPtrCast<IInputStream>(file.GetStream()))
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

			/// \note Compresss data if useCompression equals to true
			if (useCompression)
			{
				uLong compressedDataBlockSize = compressBound(static_cast<uLong>(buffer.size()));

				std::vector<U8> compressedDataBuffer;
				compressedDataBuffer.resize(static_cast<USIZE>(compressedDataBlockSize));
				
				if (Z_OK != compress(&compressedDataBuffer.front(), &compressedDataBlockSize, &buffer.front(), static_cast<uLong>(fileInfo.mDataBlockSize)))
				{
					return RC_FAIL;
				}

				buffer.resize(static_cast<USIZE>(compressedDataBlockSize));
				memcpy(&buffer.front(), &compressedDataBuffer.front(), sizeof(U8) * compressedDataBlockSize);

				fileInfo.mCompressedBlockSize = static_cast<U64>(compressedDataBlockSize);
				fileInfo.mIsCompressed = true;
			}

			// Write into the package
			if (auto pPackageStream = DynamicPtrCast<IOutputStream>(mpStreamImpl))
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
		auto pStream = DynamicPtrCast<IOutputStream>(mpStreamImpl);

		TSizeType prevPosition = pStream->GetPosition();

		E_RESULT_CODE result = pStream->SetPosition(0);
		result = result | pStream->Write(&mCurrHeader.mTag, sizeof(mCurrHeader.mTag));

		const U16 version = SwapBytes(mCurrHeader.mVersion);
		result = result | pStream->Write(&version, sizeof(version));

		result = result | pStream->Write(&mCurrHeader.mPadding, sizeof(mCurrHeader.mPadding));

		mCurrHeader.mEntitiesCount = SwapBytes(mCurrHeader.mEntitiesCount);
		result = result | pStream->Write(&mCurrHeader.mEntitiesCount, sizeof(mCurrHeader.mEntitiesCount));

		mCurrHeader.mFilesTableOffset = SwapBytes(mCurrHeader.mFilesTableOffset);
		result = result | pStream->Write(&mCurrHeader.mFilesTableOffset, sizeof(mCurrHeader.mFilesTableOffset));

		mCurrHeader.mFilesTableSize = SwapBytes(mCurrHeader.mFilesTableSize);
		result = result | pStream->Write(&mCurrHeader.mFilesTableSize, sizeof(mCurrHeader.mFilesTableSize));

		pStream->SetPosition(prevPosition);

		return result;
	}

	E_RESULT_CODE CPackageFileWriter::_writeFilesTableDescription()
	{
		auto pStream = DynamicPtrCast<IOutputStream>(mpStreamImpl);

		mCurrHeader.mFilesTableOffset = pStream->GetPosition();

		E_RESULT_CODE result = RC_OK;

		for (auto&& currFileEntryInfo : mFilesTable)
		{
			U64 filenameLength = SwapBytes(currFileEntryInfo.mFilename.length());

			result = result | pStream->Write(&filenameLength, sizeof(filenameLength));
			result = result | pStream->Write(&currFileEntryInfo.mFilename[0], static_cast<U32>(filenameLength * sizeof(C8)));

			currFileEntryInfo.mDataBlockOffset = SwapBytes(currFileEntryInfo.mDataBlockOffset);
			result = result | pStream->Write(&currFileEntryInfo.mDataBlockOffset, sizeof(currFileEntryInfo.mDataBlockOffset));
			
			currFileEntryInfo.mDataBlockSize = SwapBytes(currFileEntryInfo.mDataBlockSize);
			result = result | pStream->Write(&currFileEntryInfo.mDataBlockSize, sizeof(currFileEntryInfo.mDataBlockSize));

			currFileEntryInfo.mCompressedBlockSize = SwapBytes(currFileEntryInfo.mCompressedBlockSize);
			result = result | pStream->Write(&currFileEntryInfo.mCompressedBlockSize, sizeof(currFileEntryInfo.mCompressedBlockSize));

			result = result | pStream->Write(&currFileEntryInfo.mIsCompressed, sizeof(currFileEntryInfo.mIsCompressed));
		}

		mCurrHeader.mFilesTableSize = std::max<U64>(0, static_cast<U64>(pStream->GetPosition()) - mCurrHeader.mFilesTableOffset);
		mCurrHeader.mEntitiesCount = static_cast<U32>(mFilesTable.size());

		return result;
	}


	IFile* CreatePackageFileWriter(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result)
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