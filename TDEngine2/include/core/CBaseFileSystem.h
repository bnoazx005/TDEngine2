/*!
	\file CBaseFileSystem.h
	\date 13.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseObject.h"
#include "IFileSystem.h"
#include "../utils/CResourceContainer.h"
#include <unordered_map>
#include <vector>
#include <mutex>


namespace TDEngine2
{
	class IFile;
	class IMountableStorage;


	TDE2_DECLARE_SCOPED_PTR(IMountableStorage)


	/*!
		class CBaseFileSystem

		\brief The class is a cross-platform implementation of IFileSystem interface
	*/

	class CBaseFileSystem : public CBaseObject, public IFileSystem
	{
		protected:
			struct TMountedStorageInfo
			{
				TPtr<IMountableStorage> mpStorage;
				std::string mAliasPath;
			};

			typedef std::vector<TMountedStorageInfo> TMountingPoints;
			typedef std::unordered_map<TypeId, U32> TFileFactoriesRegistry;
			
			typedef CResourceContainer<TFileFactory> TFileFactoriesContainer;
		public:
			/*!
				\brief The method initializes a file system's object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method mounts a path to a real existing one

				\param[in] path A path within a real file system
				\param[in] aliasPath A virtual file system path, with which specific physical one will
				be associated

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE MountPhysicalPath(const std::string& path, const std::string& aliasPath, U16 relativePriority = 0) override;

			/*!
				\brief The method mounts a package to the alias to provide implicit work with its files structure

				\param[in] path A path of the package
				\param[in] aliasPath A virtual file system path, with which specific physical one will
				be associated

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE MountPackage(const std::string& path, const std::string& aliasPath) override;

			/*!
				\brief The method unmounts a path from a real existing one if it was binded earlier

				\param[in] aliasPath A virtual file system path, with which specific physical one will
				be associated

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Unmount(const std::string& aliasPath) override;

			/*!
				\brief The method converts virtual path into a real one. For example, if
				a following virtual directory /vdir/ exists and is binded to c:/data/,
				then input /vdir/foo.txt will be replaced with c:/data/foo.txt.

				\param[in] path A virtual path's value
				\param[in] isDirectory A flat that tells should be given path processed as a directory or its a path to some file

				\return A string that contains a physical path
			*/

			TDE2_API std::string ResolveVirtualPath(const std::string& path, bool isDirectory = true) const override;

			/*!
				\brief The method extrats filename with an extension from a valid given path

				\param[in] path Valid and non-empty path string

				\return A filename which stands after last separator within the path
			*/

			TDE2_API std::string ExtractFilename(const std::string& path) const;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

			/*!
				\brief The method returns true if specified file exists

				\param[in] filename A path to a file

				\return The method returns true if specified file exists
			*/

			TDE2_API bool FileExists(const std::string& filename) const override;

			/*!
				\brief The method removes specified file from the file system
			*/

			TDE2_API E_RESULT_CODE RemoveFile(const std::string& filePath) override;

			/*!
				\brief The method returns a list of file's path which are exists at given directory
			*/

			TDE2_API std::vector<std::string> GetFilesListAtDirectory(const std::string& path) const override;

			/*!
				\brief The method returns a current working directory

				\return The method returns a string with current working directory
			*/

			TDE2_API std::string GetCurrDirectory() const override;

			/*!
				\brief The method sets up a pointer to IJobManager implementation within
				the file manager.

				\param[in] pJobManager A pointer to IJobManager implementation
			*/

			TDE2_API void SetJobManager(IJobManager* pJobManager) override;

			/*!
				\brief The method returns a pointer to IJobManager

				\return The method returns a pointer to IJobManager
			*/

			TDE2_API IJobManager* GetJobManager() const override;

			/*!
				\brief The method returns a function pointer to a file factory which is registered for the given type

				\param[in] typeId A type of a file

				\return Returns a function pointer to a file factory
			*/

			TDE2_API TResult<TFileFactory> GetFileFactory(TypeId typeId) override;

			/*!
				\brief The method tells whether asynchronous file input / output operations
				are available for a user or not

				\return The method returns true if asynchronous file I/O is available, false in
				other cases
			*/

			TDE2_API bool IsStreamingEnabled() const override;

			/*!
				\brief The method extracts extension from a given path

				\param[in] path A string, which represents a valid file system path

				\return The method extracts extension from a given path
			*/

			TDE2_API std::string GetExtension(const std::string& path) const override;

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

			TDE2_API std::string CombinePath(const std::string& left, const std::string& right) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseFileSystem)

			TDE2_API E_RESULT_CODE _mountInternal(const std::string& aliasPath, IMountableStorage* pStorage, U16 relativePriority);

			TDE2_API std::string _normalizePathView(const std::string& path, bool isDirectory = true) const;

			TDE2_API virtual E_RESULT_CODE _onInit() = 0;

			TDE2_API TResult<TFileEntryId> _openFile(const TypeId& typeId, const std::string& filename, bool createIfDoesntExist) override;

			TDE2_API E_RESULT_CODE _registerFileFactory(const TypeId& typeId, const TFileFactory& fileFactory) override;

			TDE2_API E_RESULT_CODE _unregisterFileFactory(const TypeId& typeId) override;

			TDE2_API IFile* _getFile(TFileEntryId fileId) override;
			TDE2_API IFile* _getFileUnsafe(TFileEntryId fileId) override;

			TDE2_API void _createNewFile(const std::string& filename);

			/*!
				\brief The method converts virtual path into a real one. For example, if
				a following virtual directory /vdir/ exists and is binded to c:/data/,
				then input /vdir/foo.txt will be replaced with c:/data/foo.txt.

				\return A string that contains a physical path
			*/

			TDE2_API std::string _resolveVirtualPathInternal(const TMountedStorageInfo& mountInfo, const std::string& path, bool isDirectory) const;
			
			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			mutable std::mutex mMutex;

			TFileFactoriesRegistry mFileFactoriesMap;
						
			TFileFactoriesContainer mFileFactories;
			
			IJobManager* mpJobManager;

			TMountingPoints mMountedStorages;
	};
}