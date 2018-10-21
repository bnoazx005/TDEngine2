/*!
	\file IFileSystem.h
	\date 08.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "IEngineSubsystem.h"
#include "IBaseObject.h"
#include <string>
#include <type_traits>


namespace TDEngine2
{
	class IFile;
	class ITextFileReader;
	

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
				then input /vdir/foo.txt will be replaced with c:/data/foo.txt.

				\param[in] path A virtual path's value

				\return A string that contains a physical path
			*/

			TDE2_API virtual std::string ResolveVirtualPath(const std::string& path) const = 0;

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
				\brief The method creates a new file

				\param[in] filename A string that contains a path to a file
				\param[out] result An output of a function, which contains a result of its execution

				\return A pointer to a new allocated file instance
			*/

			template <typename T>
			TDE2_API
#if _HAS_CXX17
			std::enable_if_t<std::is_base_of_v<IFile, T>, T*>
#else
			typename std::enable_if<std::is_base_of<IFile, T>::value, T*>::type
#endif
			Create(const std::string& filename, E_RESULT_CODE& result);

			/*!
				\brief The method closes specified file (shouldn't be used explicitly, call Close method of IFile instead)

				\param[in] pFile A pointer to IFile's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE CloseFile(IFile* pFile) = 0;

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
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IFileSystem)

			TDE2_API virtual IFile* _createFile(U32 typeId, const std::string& filename, E_RESULT_CODE& result) = 0;

			TDE2_API virtual E_RESULT_CODE _registerFileFactory(U32 typeId, TCreateFileCallback pCreateFileCallback) = 0;

			TDE2_API virtual E_RESULT_CODE _unregisterFileFactory(U32 typeId) = 0;
	};

	
	template <typename T>
#if _HAS_CXX17
	std::enable_if_t<std::is_base_of_v<IFile, T>, T*>
#else
	typename std::enable_if<std::is_base_of<IFile, T>::value, T*>::type
#endif
	IFileSystem::Create(const std::string& filename, E_RESULT_CODE& result)
	{
		return dynamic_cast<T*>(_createFile(T::GetTypeId(), filename, result));
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
}
