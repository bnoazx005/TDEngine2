/*!
	\file MountableStorages.h
	\date 31.10.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../utils/CResourceContainer.h"
#include "../core/CBaseObject.h"
#include <unordered_map>
#include <string>
#include <tuple>


namespace TDEngine2
{
	class IFile;
	class IFileSystem;
	class IStream;


	enum class E_FILE_FACTORY_TYPE : U8;


	/*!
		\brief Than more value than lower the priority of a storage, native file system has the highest priority among other FSs
	*/

	enum class E_MOUNTABLE_STORAGES_PRIORITIES : U16
	{
		NATIVE = 0,
		PACKAGE = 500,
		UNKNOWN = (std::numeric_limits<U16>::max)()
	};


	/*!
		interface IMountableStorage

		\brief The interface describes a functionality of any mountable point within the engine
	*/

	class IMountableStorage: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an internal state

				\param[in, out] pFileSystem A pointer to IFileSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IFileSystem* pFileSystem, const std::string& path) = 0;

			/*!
				\brief The method is called when the storage has been mounted to the main file system

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE OnMounted(const std::string& aliasPath) = 0;

			TDE2_API virtual TResult<TFileEntryId> OpenFile(const TypeId& typeId, const std::string& path, bool createIfDoesntExist = false) = 0;
			TDE2_API virtual E_RESULT_CODE CloseFile(TFileEntryId handle) = 0;
			TDE2_API virtual E_RESULT_CODE CloseFile(const std::string& path) = 0;

			TDE2_API virtual TResult<IFile*> GetFile(TFileEntryId fileId) const = 0;

			TDE2_API virtual bool FileExists(const std::string& path) const = 0;

			TDE2_API virtual IFileSystem* GetFileSystem() const = 0;

			TDE2_API virtual const std::string& GetBasePath() const = 0;

			TDE2_API virtual U16 GetPriority() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IMountableStorage)
	};


	class CBaseMountableStorage : public CBaseObject, public IMountableStorage
	{
		public:
			typedef std::unordered_map<std::string, std::string> TVirtualPathsMap;
			typedef std::unordered_map<std::string, TFileEntryId> TFilesHashMap;
			typedef std::unordered_map<TFileEntryId, U32> TStorageEntriesInfo;

			typedef CResourceContainer<IFile*> TFilesContainer;

		public:
			/*!
				\brief The method initializes an internal state

				\param[in, out] pFileSystem A pointer to IFileSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IFileSystem* pFileSystem, const std::string& path) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;
			
			TDE2_API TResult<TFileEntryId> OpenFile(const TypeId& typeId, const std::string& path, bool createIfDoesntExist) override;
			TDE2_API E_RESULT_CODE CloseFile(TFileEntryId handle) override;
			TDE2_API E_RESULT_CODE CloseFile(const std::string& path) override;

			TDE2_API TResult<IFile*> GetFile(TFileEntryId fileId) const override;

			TDE2_API IFileSystem* GetFileSystem() const override;

			TDE2_API const std::string& GetBasePath() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseMountableStorage)

			TDE2_API TFileEntryId _registerFileEntry(IFile* pFileEntry);

			TDE2_API virtual void _createNewFile(const std::string& path) = 0;
			TDE2_API virtual TResult<IStream*> _createStream(const std::string& path, E_FILE_FACTORY_TYPE type) const = 0;
		protected:
			std::string mPhysicalPath;

			IFileSystem* mpFileSystem;

			TFilesContainer mActiveFiles;

			TVirtualPathsMap mVirtualPathsMap;

			TFilesHashMap mFilesMap;

			TStorageEntriesInfo mLocalConversionMap;
	};

	/*!
		\brief A factory function for creation objects of CPhysicalFilesStorage's type

		\return A pointer to CPhysicalFilesStorage's implementation
	*/

	TDE2_API IMountableStorage* CreatePhysicalFilesStorage(IFileSystem* pFileSystem, const std::string& path, E_RESULT_CODE& result);


	class CPhysicalFilesStorage : public CBaseMountableStorage
	{
		public:
			friend TDE2_API IMountableStorage* CreatePhysicalFilesStorage(IFileSystem*, const std::string&, E_RESULT_CODE&);
		
		public:
			/*!
				\brief The method is called when the storage has been mounted to the main file system

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE OnMounted(const std::string& aliasPath) override;

			TDE2_API bool FileExists(const std::string& path) const override;

			TDE2_API U16 GetPriority() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPhysicalFilesStorage)

			TDE2_API void _createNewFile(const std::string& path) override;
			TDE2_API TResult<IStream*> _createStream(const std::string& path, E_FILE_FACTORY_TYPE type) const override;
		protected:
	};


	/*!
		\brief A factory function for creation objects of CPhysicalFilesStorage's type

		\return A pointer to CPhysicalFilesStorage's implementation
	*/

	TDE2_API IMountableStorage* CreatePackageFilesStorage(IFileSystem* pFileSystem, const std::string& path, E_RESULT_CODE& result);


	class CPackageFilesStorage : public CBaseMountableStorage
	{
		public:
			friend TDE2_API IMountableStorage* CreatePackageFilesStorage(IFileSystem*, const std::string&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method is called when the storage has been mounted to the main file system

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE OnMounted(const std::string& aliasPath) override;

			TDE2_API bool FileExists(const std::string& path) const override;

			TDE2_API U16 GetPriority() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPackageFilesStorage)

			TDE2_API void _createNewFile(const std::string& path) override;
			TDE2_API TResult<IStream*> _createStream(const std::string& path, E_FILE_FACTORY_TYPE type) const override;
		protected:
			TFileEntryId mPackageFileHandle;

			std::unordered_map<std::string, std::tuple<U64, U64>> mPackageFilesInfoTable; ///< The first U64 - is offset of the file within the package, the second is size of data

			std::string mAliasPath;
	};
}