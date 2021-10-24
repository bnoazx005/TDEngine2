/*!
	\file FileStreams.h
	\date 31.10.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../core/CBaseObject.h"
#include <string>
#include <fstream>
#include <vector>


namespace TDEngine2
{
	/*!
		interface IStream

		\brief The basic interface for all streams input and output that are used within in-engine's file system
	*/

	class IStream: public virtual IBaseObject
	{
		public:
			typedef USIZE TSizeType;
		public:
			TDE2_API virtual E_RESULT_CODE SetPosition(TSizeType pos) = 0;
			TDE2_API virtual TSizeType GetPosition() const = 0;

			/*!
				\brief The method reopens current stream and resets its state

				\param[in] isBinaryMode The flag defines whether the stream is binary or not

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Reset(bool isBinaryMode = false) = 0;

			/*!
				\brief The method returns true if the stream is opened and ready to use
				\return The method returns true if the stream is opened and ready to use
			*/

			TDE2_API virtual bool IsValid() const = 0;

			TDE2_API virtual const std::string& GetName() const = 0;

			TDE2_API virtual bool IsEndOfStream() const = 0;

			TDE2_API virtual TSizeType GetLength() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IStream)
	};


	TDE2_DECLARE_SCOPED_PTR(IStream)


	class IInputStream: public virtual IStream
	{
		public:
			/*!
				\brief The method reads a continuous block of data of specified size into a given buffer

				\param[out] pBuffer A buffer which will keep the read block of a file
				\paramp[in] bufferSize A size of a block that should be read

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Read(void* pBuffer, TSizeType bufferSize) = 0;

			/*!
				\brief The method reads a current line of the file and returns its data as string

				\return A string, which contains file's data
			*/

			TDE2_API virtual std::string ReadLine() = 0;

			/*!
				\brief The method reads a whole file and returns its data as string

				\return A string, which contains file's data
			*/

			TDE2_API virtual std::string ReadToEnd() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IInputStream)
	};


	TDE2_DECLARE_SCOPED_PTR(IInputStream)


	class IOutputStream: public virtual IStream
	{
		public:
			/*!
				\brief The method writes data of specified size into a stream

				\param[in] pBuffer A pointer to data's buffer

				\param[in] bufferSize A size in bytes of data should be written

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Write(const void* pBuffer, TSizeType bufferSize) = 0;

			TDE2_API virtual void Flush() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IOutputStream)
	};


	TDE2_DECLARE_SCOPED_PTR(IOutputStream)


	/*!
		\brief A factory function for creation objects of CFileInputStream's type

		\param[in] path A string with path to a file
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFileInputStream's implementation
	*/

	TDE2_API IStream* CreateFileInputStream(const std::string& path, E_RESULT_CODE& result);


	/*!
		\brief The class is an implementation of a file reading stream
	*/

	class CFileInputStream: public CBaseObject, public IInputStream
	{
		public:
			friend TDE2_API IStream* CreateFileInputStream(const std::string&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of a stream

				\param[in] path A string with path to a file

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const std::string& path);
			
			/*!
				\brief The method reopens current stream and resets its state

				\param[in] isBinaryMode The flag defines whether the stream is binary or not

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset(bool isBinaryMode = false) override;

			/*!
				\brief The method reads a continuous block of data of specified size into a given buffer

				\param[out] pBuffer A buffer which will keep the read block of a file
				\paramp[in] bufferSize A size of a block that should be read

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Read(void* pBuffer, TSizeType bufferSize) override;

			/*!
				\brief The method reads a whole file and returns its data as string

				\return A string, which contains file's data
			*/

			TDE2_API std::string ReadLine() override;

			/*!
				\brief The method reads a whole file and returns its data as string

				\return A string, which contains file's data
			*/

			TDE2_API std::string ReadToEnd() override;

			TDE2_API E_RESULT_CODE SetPosition(TSizeType pos) override;
			TDE2_API TSizeType GetPosition() const override;

			TDE2_API const std::string& GetName() const override;

			/*!
				\brief The method returns true if the stream is opened and ready to use
				\return The method returns true if the stream is opened and ready to use
			*/

			TDE2_API bool IsValid() const override;

			TDE2_API bool IsEndOfStream() const override;

			TDE2_API TSizeType GetLength() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFileInputStream)

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			mutable std::ifstream mInternalStream;
			std::string mPath;
	};


	/*!
		\brief A factory function for creation objects of CFileOutputStream's type

		\param[in] path A string with path to a file
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFileOutputStream's implementation
	*/

	TDE2_API IStream* CreateFileOutputStream(const std::string& path, E_RESULT_CODE& result);


	/*!
		\brief The class is an implementation of a file writing stream
	*/

	class CFileOutputStream : public CBaseObject, public IOutputStream
	{
		public:
			friend TDE2_API IStream* CreateFileOutputStream(const std::string&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of a stream

				\param[in] path A string with path to a file

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const std::string& path);

			/*!
				\brief The method reopens current stream and resets its state

				\param[in] isBinaryMode The flag defines whether the stream is binary or not

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset(bool isBinaryMode = false) override;

			/*!
				\brief The method writes data of specified size into a file

				\param[in] pBuffer A pointer to data's buffer

				\param[in] bufferSize A size in bytes of data should be written

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Write(const void* pBuffer, TSizeType bufferSize) override;

			TDE2_API void Flush() override;

			TDE2_API E_RESULT_CODE SetPosition(TSizeType pos) override;
			TDE2_API TSizeType GetPosition() const override;

			TDE2_API const std::string& GetName() const override;

			/*!
				\brief The method returns true if the stream is opened and ready to use
				\return The method returns true if the stream is opened and ready to use
			*/

			TDE2_API bool IsValid() const override;

			TDE2_API bool IsEndOfStream() const override;

			TDE2_API TSizeType GetLength() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFileOutputStream)

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			mutable std::ofstream mInternalStream;
			std::string mPath;
	};


	/*!
		\brief A factory function for creation objects of CMemoryIOStream's type

		\param[in] path A string with path to a file
		\param[in] pD
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CMemoryIOStream's implementation
	*/

	TDE2_API IStream* CreateMemoryIOStream(const std::string& path, const std::vector<U8>& data, E_RESULT_CODE& result);


	class CMemoryIOStream: public CBaseObject, public IInputStream, public IOutputStream
	{
		public:
			friend TDE2_API IStream* CreateMemoryIOStream(const std::string&, const std::vector<U8>&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of a stream

				\param[in] path A string with path to a file

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const std::string& path, const std::vector<U8>& data);

			/*!
				\brief The method reopens current stream and resets its state

				\param[in] isBinaryMode The flag defines whether the stream is binary or not

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset(bool isBinaryMode = false) override;

			/*!
				\brief The method reads a continuous block of data of specified size into a given buffer

				\param[out] pBuffer A buffer which will keep the read block of a file
				\paramp[in] bufferSize A size of a block that should be read

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Read(void* pBuffer, TSizeType bufferSize) override;

			/*!
				\brief The method reads a whole file and returns its data as string

				\return A string, which contains file's data
			*/

			TDE2_API std::string ReadLine() override;

			/*!
				\brief The method reads a whole file and returns its data as string

				\return A string, which contains file's data
			*/

			TDE2_API std::string ReadToEnd() override;

			/*!
				\brief The method writes data of specified size into a file

				\param[in] pBuffer A pointer to data's buffer

				\param[in] bufferSize A size in bytes of data should be written

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Write(const void* pBuffer, TSizeType bufferSize) override;

			TDE2_API void Flush() override;

			TDE2_API E_RESULT_CODE SetPosition(TSizeType pos) override;
			TDE2_API TSizeType GetPosition() const override;

			TDE2_API const std::string& GetName() const override;

			/*!
				\brief The method returns true if the stream is opened and ready to use
				\return The method returns true if the stream is opened and ready to use
			*/

			TDE2_API bool IsValid() const override;

			TDE2_API bool IsEndOfStream() const override;

			TDE2_API TSizeType GetLength() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CMemoryIOStream)
		protected:
			std::string mPath;

			std::vector<U8> mData;
			TSizeType mPointer;
	};
}