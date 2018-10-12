/*!
	\file CUnixFileSystem.h
	\date 12.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../../core/CBaseObject.h"
#include "./../../core/IFileSystem.h"
#include <unordered_map>
#include <vector>


namespace TDEngine2
{
	class IFile;


	/*!
		\brief A factory function for creation objects of CUnixFileSystem's type

		\return A pointer to CUnixFileSystem's implementation
	*/

	TDE2_API IFileSystem* CreateUnixFileSystem(E_RESULT_CODE& result);


	/*!
		class CUnixFileSystem

		\brief The class is an implementation of IFileSystem interface for
		UNIX platform
	*/

	class CUnixFileSystem : public CBaseObject, public IFileSystem
	{
		public:
			friend TDE2_API IFileSystem* CreateUnixFileSystem(E_RESULT_CODE& result);
		protected:
			typedef U32                                                       TFileHandler;

			typedef std::unordered_map<std::string, std::string>              TVirtualPathsMap;

			typedef std::unordered_map<std::string, TFileHandler>             TFilesHashMap;

			typedef std::vector<IFile*>                                       TFilesArray;
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

			TDE2_API std::string ResolveVirtualPath(const std::string& path) const override;

			/*!
				\brief The method creates a new reader for a text files

				\param[in] filename A string that contains a path to a file
				\param[out] result An output of a function, which contains a result of its execution

				\return A pointer to a new allocated ITextFileReader's instance
			*/

			TDE2_API ITextFileReader* CreateTextFileReader(const std::string& filename, E_RESULT_CODE& result) override;

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CUnixFileSystem)

			TDE2_API E_RESULT_CODE _registerFileEntry(IFile* pFileEntry);

			TDE2_API std::string _unifyPathView(const std::string& path, bool isVirtualPath = false) const;

			TDE2_API bool _isPathValid(const std::string& path, bool isVirtualPath = false) const;
		protected:
			TFilesArray        mActiveFiles;

			TVirtualPathsMap   mVirtualPathsMap;

			TFilesHashMap      mFilesMap;

			static C8          mPathSeparator;

			static C8          mAltPathSeparator;

			static std::string mInvalidPath;

			static C8          mForbiddenChars[];
	};
}