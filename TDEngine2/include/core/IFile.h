/*!
	\file IFile.h
	\date 08.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
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

			TDE2_API virtual E_RESULT_CODE Open(IMountableStorage* pStorage, IStream* pStream) = 0;

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
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IFile)
	};


	/*!
		interface IFileReader

		\brief The interface describes a functionality of a file reader
	*/

	class IFileReader : public virtual IFile
	{
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IFileReader)
	};


	/*!
		interface ITextFileReader

		\brief The interface describes a functionality of a text file's reader

		\todo UTF-8 and UTF-16 support is needed
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
		interface IConfigFileReader

		\brief The interface describes a functionality of a config file's reader.

		\todo UTF-8 and UTF-16 support is needed
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

			/*!
				\brief The method reads a continuous block of a file of specified size
				into a given buffer

				\param[out] pBuffer A buffer which will keep the read block of a file

				\param[in] bufferSize A size of a block that should be read
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Read(void* pBuffer, U32 bufferSize) = 0;

			/*!
				\brief The method asynchronously reads a continuous block of a file of specified size
				into a given buffer

				\param[in] size A size of a block that should be read

				\param[in] successCallback A callback that will be called if everything went fine

				\param[in] errorCallback A callback will be called if something went wrong
			*/

			TDE2_API virtual void ReadAsync(U32 size, const TSuccessReadCallback& successCallback, const TErrorReadCallback& errorCallback) = 0;

			/*!
				\brief The method sets up a position of a file pointer

				\param[in] pos A new position of a file pointer

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetPosition(U32 pos) = 0;

			/*!
				\brief The method is a predicate that returns true if the end of a file has reached
				
				\return The method is a predicate that returns true if the end of a file has reached
			*/

			TDE2_API virtual bool IsEOF() const = 0;

			/*!
				\brief The method returns a current position of a file pointer
				
				\return The method returns a current position of a file pointer
			*/

			TDE2_API virtual U32 GetPosition() const = 0;

			/*!
				\brief The method returns a total size of a file in bytes

				\return The method returns a total size of a file in bytes
			*/

			TDE2_API virtual U64 GetFileLength() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IBinaryFileReader)
	};


	/*!
		interface IFileWriter

		\brief The interface describes a functionality of a file writer
	*/

	class IFileWriter: public virtual IFile
	{
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IFileWriter)
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

			/*!
				\brief The method writes data of specified size into a file

				\param[in] pBuffer A pointer to data's buffer

				\param[in] bufferSize A size in bytes of data should be written
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Write(const void* pBuffer, U32 bufferSize) = 0;

			/*!
				\brief The method writes data in asynchronous manner into a given file

				\param[in] pBuffer A pointer to data's buffer

				\param[in] bufferSize A size in bytes of data should be written

				\param[in] successCallback A callback that should be called when data was successfully written into the file

				\param[in] errorCallback A callback that will be called when some error's happened during the process
			*/

			TDE2_API virtual void WriteAsync(const void* pBuffer, U32 bufferSize, const TSuccessWriteCallback& successCallback,
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

			TDE2_API virtual E_RESULT_CODE SetPosition(U32 pos) = 0;

			/*!
				\brief The method is a predicate that returns true if the end of a file has reached

				\return The method is a predicate that returns true if the end of a file has reached
			*/

			TDE2_API virtual bool IsEOF() const = 0;

			/*!
				\brief The method returns a current position of a file pointer

				\return The method returns a current position of a file pointer
			*/

			TDE2_API virtual U32 GetPosition() const = 0;
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
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IBinaryMeshFileReader)
	};


	/*!
		\brief The interface represents a functionality of a packages file reader
	*/

	class IPackageFileReader : public virtual IBinaryFileReader
	{
		public:
			TDE2_REGISTER_TYPE(IPackageFileReader)

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
			std::enable_if_t<std::is_base_of_v<IFile, T>, E_RESULT_CODE>
#else
			typename std::enable_if<std::is_base_of<IFile, T>::value, E_RESULT_CODE>::type
#endif
			WriteFile(const std::string& path, const T& file)
			{
				return _writeFileInternal(T::GetTypeId(), path, dynamic_cast<const IFile&>(file));
			}
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPackageFileWriter)

			TDE2_API virtual E_RESULT_CODE _writeFileInternal(TypeId fileTypeId, const std::string& path, const IFile& file) = 0;
	};
}