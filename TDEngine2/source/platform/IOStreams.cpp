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
		mPath = path;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CFileInputStream::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		--mRefCounter;

		if (!mRefCounter)
		{
			mInternalStream.close();

			delete this;
		}

		return RC_OK;
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
		mInternalStream.ignore((std::numeric_limits<std::streamsize>::max)());

		std::streamsize length = mInternalStream.gcount();

		mInternalStream.clear();   // \note Since ignore will have set eof.
		mInternalStream.seekg(0, std::ios_base::beg);

		return static_cast<U64>(length);
	}


	IStream* CreateFileInputStream(const std::string& path, E_RESULT_CODE& result)
	{
		CFileInputStream* pStreamInstance = new (std::nothrow) CFileInputStream();

		if (!pStreamInstance)
		{
			result = RC_OUT_OF_MEMORY;
			return nullptr;
		}

		result = pStreamInstance->Init(path);

		if (result != RC_OK)
		{
			delete pStreamInstance;
			pStreamInstance = nullptr;
		}

		return dynamic_cast<IStream*>(pStreamInstance);
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

	E_RESULT_CODE CFileOutputStream::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		--mRefCounter;

		if (!mRefCounter)
		{
			mInternalStream.close();

			delete this;
		}

		return RC_OK;
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
		CFileOutputStream* pStreamInstance = new (std::nothrow) CFileOutputStream();

		if (!pStreamInstance)
		{
			result = RC_OUT_OF_MEMORY;
			return nullptr;
		}

		result = pStreamInstance->Init(path);

		if (result != RC_OK)
		{
			delete pStreamInstance;
			pStreamInstance = nullptr;
		}

		return dynamic_cast<IStream*>(pStreamInstance);
	}

	
	/*!
		\brief CMemoryInputStream's definition
	*/



	/*!
		\brief CMemoryOutputStream's definition
	*/

	
}