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


namespace TDEngine2
{
	/*!
		interface IStream

		\brief The basic interface for all streams input and output that are used within in-engine's file system
	*/

	class IStream: public virtual IBaseObject
	{
		public:
			TDE2_API virtual E_RESULT_CODE SetPosition(U32 pos) = 0;
			TDE2_API virtual U32 GetPosition() const = 0;

			/*!
				\brief The method returns true if the stream is opened and ready to use
				\return The method returns true if the stream is opened and ready to use
			*/

			TDE2_API virtual bool IsValid() const = 0;

			TDE2_API virtual const std::string& GetName() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IStream)
	};


	class IInputStream: public IStream
	{
		public:
			/*!
				\brief The method reads a continuous block of data of specified size into a given buffer

				\param[out] pBuffer A buffer which will keep the read block of a file
				\paramp[in] bufferSize A size of a block that should be read

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Read(void* pBuffer, U32 bufferSize) = 0;

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


	class IOutputStream: public IStream
	{
		public:
			/*!
				\brief The method writes data of specified size into a stream

				\param[in] pBuffer A pointer to data's buffer

				\param[in] bufferSize A size in bytes of data should be written

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Write(const void* pBuffer, U32 bufferSize) = 0;

			TDE2_API virtual void Flush() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IOutputStream)
	};


	/*!
		\brief A factory function for creation objects of CFileInputStream's type

		\param[in] path A string with path to a file
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFileInputStream's implementation
	*/

	TDE2_API IInputStream* CreateFileInputStream(const std::string& path, E_RESULT_CODE& result);


	/*!
		\brief The class is an implementation of a file reading stream
	*/

	class CFileInputStream: public CBaseObject, public IInputStream
	{
		public:
			friend TDE2_API IInputStream* CreateFileInputStream(const std::string&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of a stream

				\param[in] path A string with path to a file

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const std::string& path);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method reads a continuous block of data of specified size into a given buffer

				\param[out] pBuffer A buffer which will keep the read block of a file
				\paramp[in] bufferSize A size of a block that should be read

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Read(void* pBuffer, U32 bufferSize) override;

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

			TDE2_API E_RESULT_CODE SetPosition(U32 pos) override;
			TDE2_API U32 GetPosition() const override;

			TDE2_API const std::string& GetName() const override;

			/*!
				\brief The method returns true if the stream is opened and ready to use
				\return The method returns true if the stream is opened and ready to use
			*/

			TDE2_API bool IsValid() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFileInputStream)
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

	TDE2_API IOutputStream* CreateFileOutputStream(const std::string& path, E_RESULT_CODE& result);


	/*!
		\brief The class is an implementation of a file writing stream
	*/

	class CFileOutputStream : public CBaseObject, public IOutputStream
	{
		public:
			friend TDE2_API IOutputStream* CreateFileOutputStream(const std::string&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of a stream

				\param[in] path A string with path to a file

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const std::string& path);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method writes data of specified size into a file

				\param[in] pBuffer A pointer to data's buffer

				\param[in] bufferSize A size in bytes of data should be written

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Write(const void* pBuffer, U32 bufferSize) override;

			TDE2_API void Flush() override;

			TDE2_API E_RESULT_CODE SetPosition(U32 pos) override;
			TDE2_API U32 GetPosition() const override;

			TDE2_API const std::string& GetName() const override;

			/*!
				\brief The method returns true if the stream is opened and ready to use
				\return The method returns true if the stream is opened and ready to use
			*/

			TDE2_API bool IsValid() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFileOutputStream)
		protected:
			mutable std::ofstream mInternalStream;
			std::string mPath;
	};

#if 0
	/*!
		\brief A factory function for creation objects of CMemoryInputStream's type

		\param[in] path A string with path to a file
		\param[in] pD
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CMemoryInputStream's implementation
	*/

	TDE2_API IInputStream* CreateFileInputStream(const std::string& path, E_RESULT_CODE& result);


	class CMemoryInputStream: public CBaseObject, public IInputStream
	{
		public:
			friend TDE2_API IInputStream* CreateFileInputStream(const std::string& path, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a stream

				\param[in] path A string with path to a file

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const std::string& path);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method reads a continuous block of data of specified size into a given buffer

				\param[out] pBuffer A buffer which will keep the read block of a file
				\paramp[in] bufferSize A size of a block that should be read

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Read(void* pBuffer, U32 bufferSize) override;

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

			TDE2_API E_RESULT_CODE SetPosition(U32 pos) override;
			TDE2_API U32 GetPosition() const override;

			/*!
				\brief The method returns true if the stream is opened and ready to use
				\return The method returns true if the stream is opened and ready to use
			*/

			TDE2_API bool IsValid() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CMemoryInputStream)
	};


	class CMemoryOutputStream: public CBaseObject, public IOutputStream
	{
		public:

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CMemoryOutputStream)
	};
#endif
}