/*!
	\file IFile.h
	\date 08.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "Serialization.h"
#include <string>


/// \note forward declaration for mini-yaml library
namespace Yaml
{
	class Node;
}

namespace TDEngine2
{
	class IFileSystem;
	class IMountableStorage;
	class IStream;


	TDE2_DECLARE_SCOPED_PTR(IStream)


	/*!
		interface IFile

		\brief The interface represents a functionality of a file
	*/

	class IFile
	{
		public:
			/*!
				\brief The method opens specified file

				\param[in,out] pStorage A pointer to implementation of IMountableStorage
				\param[in,out] pStream A pointer to IStream implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Open(IMountableStorage* pStorage, TPtr<IStream> pStream) = 0;

			/*!
				\brief The method increment a value of the internal reference counter. You can ignore 
				this method if you work in single-threaded manner. If you want to use an object from
				another thread you should increment its counter to prevent non thread-safe deletion by
				some of running threads.
			*/

			TDE2_API virtual void AddRef() = 0;

			/*!
				\brief The method closes current opened file
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Close() = 0;

			/*!
				\brief The method returns a name of a file

				\return The method returns a name of a file
			*/

			TDE2_API virtual std::string GetFilename() const = 0;

			/*!
				\brief The method returns only name of a file with extension without its base path

				\return The method returns only name of a file with extension without its base path. E.g. "file.png"
			*/

			TDE2_API virtual std::string GetShortName() const = 0;

			/*!
				\brief The method returns true if a file is opened

				\return The method returns true if a file is opened
			*/

			TDE2_API virtual bool IsOpen() const = 0;

			/*!
				\brief The method returns true if the file is used within
				the same thread in which it was created

				\return The method returns true if the file is used within
				the same thread in which it was created
			*/

			TDE2_API virtual bool IsParentThread() const = 0;

			TDE2_API virtual TPtr<IStream> GetStream() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IFile)
	};


	/*!
		interface IFileReader

		\brief The interface describes a functionality of a file reader
	*/

	class IFileReader : public virtual IFile
	{
		public:
			typedef USIZE TSizeType;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IFileReader)
	};


	/*!
		interface ITextFileReader

		\brief The interface describes a functionality of a text file's reader
	*/

	class ITextFileReader : public virtual IFileReader
	{
		public:
			TDE2_REGISTER_TYPE(ITextFileReader)

			/*!
				\brief The method reads a single line within a text file and moves a carret to
				next one

				\return A string, which contains a read line
			*/

			TDE2_API virtual std::string ReadLine() = 0;

			/*!
				\brief The method reads a whole file and returns its data as string

				\return A string, which contains file's data
			*/

			TDE2_API virtual std::string ReadToEnd() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITextFileReader)
	};


	/*!
		interface ICsvFileReader

		\brief The interface describes a functionality of a reader of CSV files 
	*/

	class ICsvFileReader : public virtual IFileReader
	{
		public:
			TDE2_REGISTER_TYPE(ICsvFileReader)

			TDE2_API virtual std::string GetCellValue(const std::string& colId, U32 rowId) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ICsvFileReader)
	};


	/*!
		interface IConfigFileReader

		\brief The interface describes a functionality of a config file's reader.
	*/

	class IConfigFileReader : public virtual IFileReader
	{
		public:
			TDE2_REGISTER_TYPE(IConfigFileReader)

			/*!
				\brief The method try to read integer value with specified name inside a group. Otherwise, it
				will return a default value

				\param[in] group A group's name

				\param[in] paramName A parameter's name

				\param[in] defaultValue A default value, which will be returned if the specified parameter doesn't exists

				\return The method try to read integer value with specified name inside a group. Otherwise, it
				will return a default value
			*/

			TDE2_API virtual I32 GetInt(const std::string& group, const std::string& paramName, I32 defaultValue = 0) = 0;

			/*!
				\brief The method try to read integer value with specified name inside a group. Otherwise, it
				will return a default value

				\param[in] group A group's name

				\param[in] paramName A parameter's name

				\param[in] defaultValue A default value, which will be returned if the specified parameter doesn't exists

				\return The method try to read integer value with specified name inside a group. Otherwise, it
				will return a default value
			*/

			TDE2_API virtual F32 GetFloat(const std::string& group, const std::string& paramName, F32 defaultValue = 0.0f) = 0;

			/*!
				\brief The method try to read integer value with specified name inside a group. Otherwise, it
				will return a default value

				\param[in] group A group's name

				\param[in] paramName A parameter's name

				\param[in] defaultValue A default value, which will be returned if the specified parameter doesn't exists

				\return The method try to read integer value with specified name inside a group. Otherwise, it
				will return a default value
			*/

			TDE2_API virtual bool GetBool(const std::string& group, const std::string& paramName, bool defaultValue = false) = 0;

			/*!
				\brief The method try to read integer value with specified name inside a group. Otherwise, it
				will return a default value

				\param[in] group A group's name

				\param[in] paramName A parameter's name

				\param[in] defaultValue A default value, which will be returned if the specified parameter doesn't exists

				\return The method try to read integer value with specified name inside a group. Otherwise, it
				will return a default value
			*/

			TDE2_API virtual std::string GetString(const std::string& group, const std::string& paramName, std::string defaultValue = "") = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IConfigFileReader)
	};

	/*!
		interface ITextFileReader

		\brief The interface describes a functionality of a binary file's reader
	*/

	class IBinaryFileReader : public virtual IFileReader
	{
		public:
			typedef void (*TSuccessReadCallback)(void* pBuffer);

			typedef void (*TErrorReadCallback)(E_RESULT_CODE);

		public:
			TDE2_REGISTER_TYPE(IBinaryFileReader)

			TDE2_API virtual U8 ReadUInt8() = 0;
			TDE2_API virtual U16 ReadUInt16() = 0;
			TDE2_API virtual U32 ReadUInt32() = 0;
			TDE2_API virtual U64 ReadUInt64() = 0;

			TDE2_API virtual I8 ReadInt8() = 0;
			TDE2_API virtual I16 ReadInt16() = 0;
			TDE2_API virtual I32 ReadInt32() = 0;
			TDE2_API virtual I64 ReadInt64() = 0;

			TDE2_API virtual F32 ReadFloat() = 0;
			TDE2_API virtual F64 ReadDouble() = 0;

			TDE2_API virtual bool ReadBool() = 0;

			/*!
				\brief The method reads a continuous block of a file of specified size
				into a given buffer. Read doesn't take into account endianness of current machine
				to solve the problem use ReadX() methods

				\param[out] pBuffer A buffer which will keep the read block of a file

				\param[in] bufferSize A size of a block that should be read
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Read(void* pBuffer, TSizeType bufferSize) = 0;

			/*!
				\brief The method asynchronously reads a continuous block of a file of specified size
				into a given buffer

				\param[in] size A size of a block that should be read

				\param[in] successCallback A callback that will be called if everything went fine

				\param[in] errorCallback A callback will be called if something went wrong
			*/

			TDE2_API virtual void ReadAsync(TSizeType size, const TSuccessReadCallback& successCallback, const TErrorReadCallback& errorCallback) = 0;

			/*!
				\brief The method sets up a position of a file pointer

				\param[in] pos A new position of a file pointer

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetPosition(TSizeType pos) = 0;

			/*!
				\brief The method is a predicate that returns true if the end of a file has reached
				
				\return The method is a predicate that returns true if the end of a file has reached
			*/

			TDE2_API virtual bool IsEOF() const = 0;

			/*!
				\brief The method returns a current position of a file pointer
				
				\return The method returns a current position of a file pointer
			*/

			TDE2_API virtual TSizeType GetPosition() const = 0;

			/*!
				\brief The method returns a total size of a file in bytes

				\return The method returns a total size of a file in bytes
			*/

			TDE2_API virtual TSizeType GetFileLength() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IBinaryFileReader)
	};


	/*!
		interface IFileWriter

		\brief The interface describes a functionality of a file writer
	*/

	class IFileWriter: public virtual IFile
	{
		public:
			typedef USIZE TSizeType;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IFileWriter)
	};


	/*!
		interface ITextFileWriter

		\brief The interface describes a functionality of a text file's writer
	*/

	class ITextFileWriter : public virtual IFileWriter
	{
		public:
			TDE2_REGISTER_TYPE(ITextFileWriter)

			/*!
				\brief The method writes a given line and moves the file cursor to a new line

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE WriteLine(const std::string& value) = 0;

		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITextFileWriter)
	};


	/*!
		interface IConfigFileWriter

		\brief The interface describes a functionality of a config file's writer.
	*/

	class IConfigFileWriter : public virtual IFileWriter
	{
		public:
			TDE2_REGISTER_TYPE(IConfigFileWriter)

			TDE2_API virtual E_RESULT_CODE SetInt(const std::string& group, const std::string& paramName, I32 value = 0) = 0;
			TDE2_API virtual E_RESULT_CODE SetFloat(const std::string& group, const std::string& paramName, F32 value = 0.0f) = 0;
			TDE2_API virtual E_RESULT_CODE SetBool(const std::string& group, const std::string& paramName, bool value = false) = 0;
			TDE2_API virtual E_RESULT_CODE SetString(const std::string& group, const std::string& paramName, const std::string& value = "") = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IConfigFileWriter)
	};


	/*!
		interface IBinaryFileWriter

		\brief The interface represents a functionality of binary file writer
	*/

	class IBinaryFileWriter: public IFileWriter
	{
		public:
			typedef void(*TSuccessWriteCallback)();

			typedef void(*TErrorWriteCallback)(E_RESULT_CODE);

		public:
			TDE2_REGISTER_TYPE(IBinaryFileWriter)

			TDE2_API virtual E_RESULT_CODE WriteUInt8(U8 value) = 0;
			TDE2_API virtual E_RESULT_CODE WriteUInt16(U16 value) = 0;
			TDE2_API virtual E_RESULT_CODE WriteUInt32(U32 value) = 0;
			TDE2_API virtual E_RESULT_CODE WriteUInt64(U64 value) = 0;

			TDE2_API virtual E_RESULT_CODE WriteInt8(I8 value) = 0;
			TDE2_API virtual E_RESULT_CODE WriteInt16(I16 value) = 0;
			TDE2_API virtual E_RESULT_CODE WriteInt32(I32 value) = 0;
			TDE2_API virtual E_RESULT_CODE WriteInt64(I64 value) = 0;

			TDE2_API virtual E_RESULT_CODE WriteFloat(F32 value) = 0;
			TDE2_API virtual E_RESULT_CODE WriteDouble(F64 value) = 0;

			TDE2_API virtual E_RESULT_CODE WriteBool(bool value) = 0;

			/*!
				\brief The method writes data of specified size into a file. If you need endian independent 
				writings use WriteInt8, WriteUint16, etc

				\param[in] pBuffer A pointer to data's buffer

				\param[in] bufferSize A size in bytes of data should be written
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Write(const void* pBuffer, TSizeType bufferSize) = 0;

			/*!
				\brief The method writes data in asynchronous manner into a given file

				\param[in] pBuffer A pointer to data's buffer

				\param[in] bufferSize A size in bytes of data should be written

				\param[in] successCallback A callback that should be called when data was successfully written into the file

				\param[in] errorCallback A callback that will be called when some error's happened during the process
			*/

			TDE2_API virtual void WriteAsync(const void* pBuffer, TSizeType bufferSize, const TSuccessWriteCallback& successCallback,
											 const TErrorWriteCallback& errorCallback) = 0;

			/*!
				\brief The method writes all unsaved buffer data into the destination file

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Flush() = 0;

			/*!
				\brief The method sets up a position of a file pointer

				\param[in] pos A new position of a file pointer

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetPosition(TSizeType pos) = 0;

			/*!
				\brief The method is a predicate that returns true if the end of a file has reached

				\return The method is a predicate that returns true if the end of a file has reached
			*/

			TDE2_API virtual bool IsEOF() const = 0;

			/*!
				\brief The method returns a current position of a file pointer

				\return The method returns a current position of a file pointer
			*/

			TDE2_API virtual TSizeType GetPosition() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IBinaryFileWriter)
	};


	/*!
		interface IImageFileWriter

		\brief The interface represents a functionality of image file writer
	*/

	class IImageFileWriter : public IFileWriter
	{
		public:
			TDE2_REGISTER_TYPE(IImageFileWriter)

			/*!
				\brief The method writes image data into a file

				\param[in, out] pTexture A pointer to ITexture2D implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Write(class ITexture2D* pTexture) = 0;

			TDE2_API virtual E_RESULT_CODE Write(I32 width, I32 height, I8 numOfChannels, const std::vector<U8>& imageData) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IImageFileWriter)
	};


	/*!
		interface IYAMLFileWriter

		\brief The interface represents a functionality of YAML file writer
	*/

	class IYAMLFileWriter : public IArchiveWriter, public IFileWriter
	{
		public:
			TDE2_REGISTER_TYPE(IYAMLFileWriter)

			TDE2_API virtual E_RESULT_CODE Serialize(Yaml::Node& object) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IYAMLFileWriter)
	};


	/*!
		interface IYAMLFileReader

		\brief The interface describes a functionality of YAML file reader
	*/

	class IYAMLFileReader : public IArchiveReader, public IFileReader
	{
		public:
			TDE2_REGISTER_TYPE(IYAMLFileReader)

			TDE2_API virtual E_RESULT_CODE Deserialize(Yaml::Node& outputObject) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IYAMLFileReader)
	};



	class IMesh;
	class IStaticMesh;
	class ISkinnedMesh;


	/*!
		\brief The interface describes a functionality of a reader
		of mesh files which correspond to internal binary meshes
	*/

	class IBinaryMeshFileReader : public virtual IBinaryFileReader
	{
		public:
			TDE2_REGISTER_TYPE(IBinaryMeshFileReader)

			/*!
				\brief The method reads binary mesh file into the given memory

				\param[out] pDestMesh A pointer to IMesh implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE LoadMesh(IMesh*& pDestMesh) = 0;

			TDE2_API virtual E_RESULT_CODE LoadStaticMesh(IStaticMesh* const& pMesh) = 0;
			TDE2_API virtual E_RESULT_CODE LoadSkinnedMesh(ISkinnedMesh* const& pMesh) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IBinaryMeshFileReader)
	};


	typedef struct TPackageFileEntryInfo
	{
		std::string mFilename;

		U64  mDataBlockOffset = 0;
		U64  mDataBlockSize = 0;
		U64  mCompressedBlockSize = 0;
		bool mIsCompressed = false;
	} TPackageFileEntryInfo, *TPackageFileEntryInfoPtr;


	/*!
		\brief The interface represents a functionality of a packages file reader
	*/

	class IPackageFileReader : public virtual IBinaryFileReader
	{
		public:
			TDE2_REGISTER_TYPE(IPackageFileReader)

			/*!
				\brief The method reads file's data no matter on whether it's compressed or not. It the data was
				stored in compressed way automatically decompresses it 

				\return The array of decompressed bytes that represent data
			*/

			TDE2_API virtual std::vector<U8> ReadFileBytes(const std::string& path) = 0;

			TDE2_API virtual const struct TPackageFileHeader& GetPackageHeader() const = 0;
			TDE2_API virtual const std::vector<TPackageFileEntryInfo>& GetFilesTable() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPackageFileReader)
	};


	/*!
		\brief The interface represents a functionality of a packages file writer
	*/

	class IPackageFileWriter : public virtual IBinaryFileWriter
	{
		public:
			TDE2_REGISTER_TYPE(IPackageFileWriter)

			template <typename T>
			TDE2_API
#if _HAS_CXX17
			std::enable_if_t<std::is_base_of_v<IFileReader, T>, E_RESULT_CODE>
#else
			typename std::enable_if<std::is_base_of<IFileReader, T>::value, E_RESULT_CODE>::type
#endif
			WriteFile(const std::string& path, const T& file, bool useCompression = false)
			{
				return _writeFileInternal(T::GetTypeId(), path, dynamic_cast<const IFileReader&>(file), useCompression);
			}
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPackageFileWriter)

			TDE2_API virtual E_RESULT_CODE _writeFileInternal(TypeId fileTypeId, const std::string& path, const IFileReader& file, bool useCompression) = 0;
	};


	/*!
		interface IBinaryArchiveWriter

		\brief The interface represents a functionality of a binary archive writer
	*/

	class IBinaryArchiveWriter : public IArchiveWriter, public IFileWriter
	{
		public:
			TDE2_REGISTER_TYPE(IBinaryArchiveWriter)
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IBinaryArchiveWriter)
	};


	/*!
		interface IBinaryArchiveReader

		\brief The interface describes a functionality of a binary archive reader
	*/

	class IBinaryArchiveReader : public IArchiveReader, public IFileReader
	{
		public:
			TDE2_REGISTER_TYPE(IBinaryArchiveReader)
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IBinaryArchiveReader)
	};
}