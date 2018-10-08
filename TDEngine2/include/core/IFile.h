/*!
	\file IFile.h
	\date 08.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include <string>


namespace TDEngine2
{
	class IFileSystem;


	/*!
		interface IFile

		\brief The interface represents a functionality of a file
	*/

	class IFile
	{
		public:
			/*!
				\brief The method opens specified file

				\param[in,out] pFileSystem A pointer to implementation of IFileSystem
				\param[in] filename A name of a file

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Open(IFileSystem* pFileSystem, const std::string& filename) = 0;

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
		interface IFileWriter

		\brief The interface describes a functionality of a file writer
	*/

	class IFileWriter : public virtual IFile
	{
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IFileWriter)
	};
}