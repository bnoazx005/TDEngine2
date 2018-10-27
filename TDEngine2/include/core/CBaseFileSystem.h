/*!
	\file CBaseFileSystem.h
	\date 13.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseObject.h"
#include "IFileSystem.h"
#include <unordered_map>
#include <vector>
#include <list>


namespace TDEngine2
{
	class IFile;


	/*!
		class CBaseFileSystem

		\brief The class is a cross-platform implementation of IFileSystem interface
	*/

	class CBaseFileSystem : public CBaseObject, public IFileSystem
	{
		protected:
			typedef U32                                                       TFileHandler;

			typedef std::unordered_map<std::string, std::string>              TVirtualPathsMap;

			typedef std::unordered_map<std::string, TFileHandler>             TFilesHashMap;

			typedef std::vector<IFile*>                                       TFilesArray;

			typedef std::unordered_map<TypeId, U32>                           TFileFactoriesRegistry;

			typedef std::vector<TCreateFileCallback>                          TFileFactories;

			typedef std::list<U32>                                            TFreeEntitiesRegistry;
		public:
			/*!
				\brief The method initializes a file system's object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method mounts a path to a real existing one

				\param[in] path A path within a real file system
				\param[in] aliasPath A virtual file system path, with which specific physical one will
				be associated

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Mount(const std::string& path, const std::string& aliasPath) override;

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

				\return A string that contains a physical path
			*/

			TDE2_API virtual std::string ResolveVirtualPath(const std::string& path) const = 0;

			/*!
				\brief The method closes specified file

				\param[in] pFile A pointer to IFile's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE CloseFile(IFile* pFile) override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

			/*!
				\brief The method closes all opened files

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE CloseAllFiles() override;

			/*!
				\brief The method returns true if specified file exists

				\param[in] filename A path to a file

				\return The method returns true if specified file exists
			*/

			TDE2_API bool FileExists(const std::string& filename) const override;

			/*!
				\brief The method returns a current working directory

				\return The method returns a string with current working directory
			*/

			TDE2_API std::string GetCurrDirectory() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseFileSystem)

			TDE2_API E_RESULT_CODE _registerFileEntry(IFile* pFileEntry);

			TDE2_API virtual std::string _unifyPathView(const std::string& path, bool isVirtualPath = false) const = 0;

			TDE2_API virtual bool _isPathValid(const std::string& path, bool isVirtualPath = false) const = 0;

			TDE2_API virtual E_RESULT_CODE _onInit() = 0;

			TDE2_API IFile* _createFile(U32 typeId, const std::string& filename, E_RESULT_CODE& result) override;

			TDE2_API E_RESULT_CODE _registerFileFactory(U32 typeId, TCreateFileCallback pCreateFileCallback) override;

			TDE2_API E_RESULT_CODE _unregisterFileFactory(U32 typeId) override;
		protected:
			TFilesArray            mActiveFiles;

			TVirtualPathsMap       mVirtualPathsMap;

			TFilesHashMap          mFilesMap;

			TFileFactoriesRegistry mFileFactoriesMap;

			TFileFactories         mFileFactories;

			TFreeEntitiesRegistry  mFileFactoriesFreeSlots;

			static std::string     mInvalidPath;
	};
}