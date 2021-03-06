/*!
	\file IFileSystem.h
	\date 08.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../utils/CResult.h"
#include "IEngineSubsystem.h"
#include "IBaseObject.h"
#include <string>
#include <type_traits>


namespace TDEngine2
{
	class IFile;
	class IJobManager;
	

	/*!
		interface IFileSystem

		\brief The interface describes a functionality of a virtual file system
		that the engine uses
	*/

	class IFileSystem : public virtual IBaseObject, public IEngineSubsystem
	{
		public:
			typedef IFile* (*TCreateFileCallback)(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes a file system's object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method mounts a virtual path to a real existing one

				\param[in] path A path within a real file system
				\param[in] aliasPath A virtual file system path, with which specific physical one will
				be associated

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Mount(const std::string& path, const std::string& aliasPath) = 0;

			/*!
				\brief The method unmounts a virtual path from a real existing one if it was binded earlier

				\param[in] aliasPath A virtual file system path, with which specific physical one will
				be associated

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Unmount(const std::string& aliasPath) = 0;

			/*!
				\brief The method converts virtual path into a real one. For example, if
				a following virtual directory /vdir/ exists and is binded to c:/data/,
				then input vfs://vdir/foo.txt will be replaced with c:/data/foo.txt.

				\param[in] path A virtual path's value
				\param[in] isDirectory A flat that tells should be given path processed as a directory or its a path to some file

				\return A string that contains a physical path
			*/

			TDE2_API virtual std::string ResolveVirtualPath(const std::string& path, bool isDirectory = true) const = 0;

			/*!
				\brief The method registers a file factory and binds it with a specified type

				\param[in] pCreateFileCallback A pointer to a function, which is a file factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			template <typename T>
			TDE2_API 
#if _HAS_CXX17
			std::enable_if_t<std::is_base_of_v<IFile, T>, E_RESULT_CODE>
#else
			typename std::enable_if<std::is_base_of<IFile, T>::value, E_RESULT_CODE>::type
#endif
			RegisterFileFactory(TCreateFileCallback pCreateFileCallback);

			/*!
				\brief The method unregisters a file factory, which is attached to a specified type

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			template <typename T>
			TDE2_API
#if _HAS_CXX17
			std::enable_if_t<std::is_base_of_v<IFile, T>, E_RESULT_CODE>
#else
			typename std::enable_if<std::is_base_of<IFile, T>::value, E_RESULT_CODE>::type
#endif
			UnregisterFileFactory();

			/*!
				\brief The method opens a file. If 'createIfDoesntExist' argument equals to true the method
				will create a new file in a file system if latter doesn't exist. If the parameter
				equals to false an error will be returned

				\param[in] filename A string that contains a path to a file
				\param[in] createIfDoesntExist If this argument equals to true the method
				will create a new file in a file system if latter doesn't exist. If the parameter
				equals to false an error will be returned

				\return Either a handle of opened file or an error code
			*/

			template <typename T>
			TDE2_API
#if _HAS_CXX17
			std::enable_if_t<std::is_base_of_v<IFile, T>, TResult<TFileEntryId>>
#else
			typename std::enable_if<std::is_base_of<IFile, T>::value, TResult<TFileEntryId>>::type
#endif
			Open(const std::string& filename, bool createIfDoesntExist = false);
			
			/*!
				\brief The method closes a file with a given filename

				\param[in] filename A path to a file

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE CloseFile(const std::string& filename) = 0;

			/*!
				\brief The method closes a file with a given file's identifier

				\param[in] fileId An identifier of a file

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE CloseFile(TFileEntryId fileId) = 0;

			/*!
				\brief The method closes all opened files
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE CloseAllFiles() = 0;

			/*!
				\brief The method returns true if specified file exists

				\param[in] filename A path to a file

				\return The method returns true if specified file exists
			*/

			TDE2_API virtual bool FileExists(const std::string& filename) const = 0;

			/*!
				\brief The method returns a current working directory

				\return The method returns a string with current working directory
			*/

			TDE2_API virtual std::string GetCurrDirectory() const = 0;

			/*!
				\brief The method returns a pointer of a file by its handle

				\param[in] fileId An identifier of a file

				\return A pointer to IFile implementation
			*/

			template <typename T>
			TDE2_API
#if _HAS_CXX17
			std::enable_if_t<std::is_base_of_v<IFile, T>, T*>
#else
			typename std::enable_if<std::is_base_of<IFile, T>::value, T*>::type
#endif
			Get(TFileEntryId fileId);

			/*!
				\brief The method sets up a pointer to IJobManager implementation within
				the file manager.

				\param[in] pJobManager A pointer to IJobManager implementation
			*/

			TDE2_API virtual void SetJobManager(IJobManager* pJobManager) = 0;

			/*!
				\brief The method returns a pointer to IJobManager

				\return The method returns a pointer to IJobManager
			*/

			TDE2_API virtual IJobManager* GetJobManager() const = 0;

			/*!
				\brief The method tells whether asynchronous file input / output operations
				are available for a user or not

				\return The method returns true if asynchronous file I/O is available, false in
				other cases
			*/

			TDE2_API virtual bool IsStreamingEnabled() const = 0;

			/*!
				\brief The method extracts extension from a given path 

				\param[in] path A string, which represents a valid file system path

				\return The method extracts extension from a given path
			*/

			TDE2_API virtual std::string GetExtension(const std::string& path) const = 0;

			/*!
				\brief The method returns a path separator charater
				\return The method returns a path separator charater
			*/

			TDE2_API virtual const C8& GetPathSeparatorChar() const = 0;

			/*!
				\brief The method returns an alternative version of a path separator charater
				\return The method returns an alternative version of a path separator charater
			*/

			TDE2_API virtual const C8& GetAltPathSeparatorChar() const = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_FILE_SYSTEM; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IFileSystem)

			TDE2_API virtual TResult<TFileEntryId> _openFile(U32 typeId, const std::string& filename, bool createIfDoesntExist) = 0;

			TDE2_API virtual E_RESULT_CODE _registerFileFactory(U32 typeId, TCreateFileCallback pCreateFileCallback) = 0;

			TDE2_API virtual E_RESULT_CODE _unregisterFileFactory(U32 typeId) = 0;

			TDE2_API virtual IFile* _getFile(TFileEntryId fileId) = 0;
	};

	
	template <typename T>
#if _HAS_CXX17
	std::enable_if_t<std::is_base_of_v<IFile, T>, TResult<TFileEntryId>>
#else
	typename std::enable_if<std::is_base_of<IFile, T>::value, TResult<TFileEntryId>>::type
#endif
	IFileSystem::Open(const std::string& filename, bool createIfDoesntExist)
	{
		return _openFile(T::GetTypeId(), filename, createIfDoesntExist);
	}

	template <typename T>
#if _HAS_CXX17
	std::enable_if_t<std::is_base_of_v<IFile, T>, E_RESULT_CODE>
#else
	typename std::enable_if<std::is_base_of<IFile, T>::value, E_RESULT_CODE>::type
#endif
	IFileSystem::RegisterFileFactory(TCreateFileCallback pCreateFileCallback)
	{
		return _registerFileFactory(T::GetTypeId(), pCreateFileCallback);
	}

	template <typename T>
#if _HAS_CXX17
	std::enable_if_t<std::is_base_of_v<IFile, T>, E_RESULT_CODE>
#else
	typename std::enable_if<std::is_base_of<IFile, T>::value, E_RESULT_CODE>::type
#endif
	IFileSystem::UnregisterFileFactory()
	{
		return _unregisterFileFactory(T::GetTypeId());
	}

	template <typename T>
	TDE2_API
#if _HAS_CXX17
	std::enable_if_t<std::is_base_of_v<IFile, T>, T*>
#else
	typename std::enable_if<std::is_base_of<IFile, T>::value, T*>::type
#endif
	IFileSystem::Get(TFileEntryId fileId)
	{
		return dynamic_cast<T*>(_getFile(fileId));
	}
}
