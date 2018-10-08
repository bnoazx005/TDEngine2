/*!
	\file CTextFileReader.h
	\date 08.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/IFile.h"
#include <fstream>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CTextFileReader's type

		\return A pointer to CTextFileReader's implementation
	*/

	TDE2_API IFileReader* CreateTextFileReader(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result);


	/*!
		class CTextFileReader

		\brief The class represents a base reader of text files
	*/

	class CTextFileReader: public ITextFileReader
	{
		public:
			friend TDE2_API IFileReader* CreateTextFileReader(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result);
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
				\brief The method reads a single line within a text file and moves a carret to
				next one

				\return A string, which contains a read line
			*/

			TDE2_API std::string ReadLine() override;

			/*!
				\brief The method reads a whole file and returns its data as string

				\return A string, which contains file's data
			*/

			TDE2_API std::string ReadToEnd() override;

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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTextFileReader)
		protected:
			std::ifstream mFile;

			std::string   mName;

			IFileSystem*  mpFileSystemInstance;
	};
}
