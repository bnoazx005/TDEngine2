#include "../../include/platform/IOStreams.h"
#include "../../deps/Wrench/source/stringUtils.hpp"

namespace TDEngine2
{
	/*!
		\brief CFileInputStream's definition
	*/

	CFileInputStream::CFileInputStream() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CFileInputStream::Init(const std::string& path)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mInternalStream.open(path);
		if (!mInternalStream.is_open())
		{
			return RC_FILE_NOT_FOUND;
		}

		mPath = path;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CFileInputStream::_onFreeInternal()
	{
		mInternalStream.close();
		return RC_OK;
	}

	E_RESULT_CODE CFileInputStream::Reset(bool isBinaryMode)
	{
		if (mInternalStream.is_open())
		{
			mInternalStream.close();
		}

		if (isBinaryMode)
		{
			mInternalStream.open(mPath, std::ios::binary);
		}
		else
		{
			mInternalStream.open(mPath);
		}

		if (!mInternalStream.is_open())
		{
			return RC_FILE_NOT_FOUND;
		}

		return IsValid() ? RC_OK : RC_FAIL;
	}

	E_RESULT_CODE CFileInputStream::Read(void* pBuffer, U32 bufferSize)
	{
		if (!pBuffer || !bufferSize)
		{
			return RC_INVALID_ARGS;
		}

		mInternalStream.read(static_cast<char*>(pBuffer), bufferSize);

		return RC_OK;
	}

	std::string CFileInputStream::ReadLine()
	{
		if (!mInternalStream.is_open())
		{
			return Wrench::StringUtils::GetEmptyStr();
		}

		std::string currReadLine;
		std::getline(mInternalStream, currReadLine);

		return currReadLine;
	}

	std::string CFileInputStream::ReadToEnd()
	{
		if (!mInternalStream.is_open())
		{
			return Wrench::StringUtils::GetEmptyStr();
		}

		std::stringstream strBuffer;
		strBuffer << mInternalStream.rdbuf();

		return strBuffer.str();
	}

	E_RESULT_CODE CFileInputStream::SetPosition(U32 pos)
	{
		mInternalStream.seekg(pos);
		return RC_OK;
	}

	const std::string& CFileInputStream::GetName() const
	{
		return mPath;
	}

	U32 CFileInputStream::GetPosition() const
	{
		return static_cast<U32>(mInternalStream.tellg());
	}

	bool CFileInputStream::IsValid() const
	{
		return !mInternalStream.bad() && !mInternalStream.fail();
	}

	bool CFileInputStream::IsEndOfStream() const
	{
		return mInternalStream.eof();
	}

	U64 CFileInputStream::GetLength() const
	{
		if (!mInternalStream.is_open() || !IsValid())
		{
			return 0;
		}

		mInternalStream.seekg(0, std::ios::end);
		U64 length = mInternalStream.tellg();

		mInternalStream.clear();   // \note Since ignore will have set eof.
		mInternalStream.seekg(0, std::ios_base::beg);

		return static_cast<U64>(length);
	}


	IStream* CreateFileInputStream(const std::string& path, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IStream, CFileInputStream, result, path);
	}


	/*!
		\brief CFileOutputStream's definition
	*/

	CFileOutputStream::CFileOutputStream() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CFileOutputStream::Init(const std::string& path)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mInternalStream.open(path);
		mPath = path;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CFileOutputStream::_onFreeInternal()
	{
		mInternalStream.close();
		return RC_OK;
	}

	E_RESULT_CODE CFileOutputStream::Reset(bool isBinaryMode)
	{
		if (mInternalStream.is_open())
		{
			mInternalStream.close();
		}

		if (isBinaryMode)
		{
			mInternalStream.open(mPath, std::ios::binary);
		}
		else
		{
			mInternalStream.open(mPath);
		}

		if (!mInternalStream.is_open())
		{
			return RC_FILE_NOT_FOUND;
		}

		return IsValid() ? RC_OK : RC_FAIL;
	}

	E_RESULT_CODE CFileOutputStream::Write(const void* pBuffer, U32 bufferSize)
	{
		if (!pBuffer || !bufferSize)
		{
			return RC_INVALID_ARGS;
		}

		mInternalStream.write(static_cast<const char*>(pBuffer), bufferSize);
		return RC_OK;
	}

	void CFileOutputStream::Flush()
	{
		mInternalStream.flush();
	}

	E_RESULT_CODE CFileOutputStream::SetPosition(U32 pos)
	{
		mInternalStream.seekp(pos);
		return RC_OK;
	}

	U32 CFileOutputStream::GetPosition() const
	{
		return static_cast<U32>(mInternalStream.tellp());
	}

	const std::string& CFileOutputStream::GetName() const
	{
		return mPath;
	}

	bool CFileOutputStream::IsValid() const
	{
		return !mInternalStream.bad() && !mInternalStream.fail();
	}

	bool CFileOutputStream::IsEndOfStream() const
	{
		return mInternalStream.eof();
	}

	U64 CFileOutputStream::GetLength() const
	{
		return static_cast<U64>(GetPosition());
	}


	IStream* CreateFileOutputStream(const std::string& path, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IStream, CFileOutputStream, result, path);
	}

	
	/*!
		\brief CMemoryIOStream's definition
	*/

	CMemoryIOStream::CMemoryIOStream() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CMemoryIOStream::Init(const std::string& path, const std::vector<U8>& data)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}
		
		std::copy(data.begin(), data.end(), std::back_inserter(mData));

		mPath = path;
		mPointer = 0;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CMemoryIOStream::Reset(bool isBinaryMode)
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CMemoryIOStream::Read(void* pBuffer, U32 bufferSize)
	{
		if (!pBuffer || !bufferSize)
		{
			return RC_INVALID_ARGS;
		}

		U32 size = std::min<U32>(bufferSize, std::max<U32>(mData.size() - mPointer, 0));

		memcpy(pBuffer, static_cast<void*>(&mData[mPointer]), size);
		mPointer += size;

		return RC_OK;
	}

	std::string CMemoryIOStream::ReadLine()
	{
		if (!IsValid())
		{
			return Wrench::StringUtils::GetEmptyStr();
		}
		
		U32 start = mPointer;

		while ((mPointer < mData.size()) && (mData[mPointer] != '\n'))
		{
			++mPointer;
		}

		return std::string(mData.cbegin() + start, mData.cbegin() + start + mPointer - 1);
	}

	std::string CMemoryIOStream::ReadToEnd()
	{
		if (!IsValid())
		{
			return Wrench::StringUtils::GetEmptyStr();
		}

		mPointer = mData.size();

		return std::string(mData.cbegin(), mData.cend());
	}

	E_RESULT_CODE CMemoryIOStream::Write(const void* pBuffer, U32 bufferSize)
	{
		return RC_OK;
	}

	void CMemoryIOStream::Flush()
	{
	}

	E_RESULT_CODE CMemoryIOStream::SetPosition(U32 pos)
	{
		if (pos > mData.size())
		{
			return RC_FAIL;
		}

		mPointer = pos;

		return RC_OK;
	}

	const std::string& CMemoryIOStream::GetName() const
	{
		return mPath;
	}

	U32 CMemoryIOStream::GetPosition() const
	{
		return mPointer;
	}

	bool CMemoryIOStream::IsValid() const
	{
		return !mData.empty();
	}

	bool CMemoryIOStream::IsEndOfStream() const
	{
		return mPointer >= mData.size();
	}

	U64 CMemoryIOStream::GetLength() const
	{		
		return static_cast<U64>(mData.size());
	}


	IStream* CreateMemoryIOStream(const std::string& path, const std::vector<U8>& data, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IStream, CMemoryIOStream, result, path, data);
	}	
}