/*!
	\file CBaseFile.h
	\date 21.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/IFile.h"
#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../core/IFileSystem.h"


namespace TDEngine2
{
	/*!
		class CBaseFile

		\brief The class implements a common functionality of IFile interface.
		All implementations should inherit this one to provide a proper type counting.
	*/

	template <typename T>
	class CBaseFile: public virtual IFile
	{
		public:
			/*!
				\brief The method opens specified file

				\param[in,out] pFileSystem A pointer to implementation of IFileSystem
				\param[in] filename A name of a file

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Open(IFileSystem* pFileSystem, const std::string& filename) override;

			/*!
				\brief The method closes current opened file

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Close() override;

			/*!
				\brief The method returns a name of a file

				\return The method returns a name of a file
			*/

			TDE2_API std::string GetFilename() const override;

			/*!
				\brief The method returns true if a file is opened

				\return The method returns true if a file is opened
			*/

			TDE2_API bool IsOpen() const override;

			/*!
				\brief The static method returns the type's identifier

				\return The static method returns the type's identifier
			*/

			TDE2_API static U32 GetTypeId();
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseFile)

			TDE2_API virtual E_RESULT_CODE _onFree() = 0;
		protected:
			static U32    mTypeId;					///< The value of mTypeId is same for all resources of T type

			std::ifstream mFile;

			std::string   mName;

			IFileSystem*  mpFileSystemInstance;
	};


	template <typename T>
	CBaseFile<T>::CBaseFile()
	{
	}

	template <typename T>
	E_RESULT_CODE CBaseFile<T>::Open(IFileSystem* pFileSystem, const std::string& filename)
	{
		if (!pFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		if (mFile.is_open())
		{
			return RC_FAIL;
		}

		mFile.open(filename, std::ios::in);

		if (!mFile.is_open())
		{
			return RC_FILE_NOT_FOUND;
		}

		mName = filename;

		mpFileSystemInstance = pFileSystem;

		return RC_OK;
	}

	template <typename T>
	E_RESULT_CODE CBaseFile<T>::Close()
	{
		if (!mFile.is_open())
		{
			return RC_FAIL;
		}

		mFile.close();

		E_RESULT_CODE result = mpFileSystemInstance->CloseFile(this);

		if (result != RC_OK)
		{
			return result;
		}

		if ((result = _onFree()) != RC_OK)
		{
			return result;
		}

		delete this;

		return RC_OK;
	}

	template <typename T>
	std::string CBaseFile<T>::GetFilename() const
	{
		return mName;
	}

	template <typename T>
	bool CBaseFile<T>::IsOpen() const
	{
		return mFile.is_open();
	}

	template <typename T>
	U32 CBaseFile<T>::GetTypeId()
	{
		return mTypeId;
	}

	template <typename T>
	U32 CBaseFile<T>::mTypeId = TTypeRegistry<T>::GetTypeId(); ///< Compile-time generation of a type's id
}
