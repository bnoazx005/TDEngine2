/*!
	\file CTextFileReader.h
	\date 08.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseFile.h"
#include <fstream>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CTextFileReader's type

		\return A pointer to CTextFileReader's implementation
	*/

	TDE2_API IFile* CreateTextFileReader(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result);


	/*!
		class CTextFileReader

		\brief The class represents a base reader of text files
	*/

	class CTextFileReader: public ITextFileReader, public CBaseFile<CTextFileReader>
	{
		public:
			friend TDE2_API IFile* CreateTextFileReader(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result);
		public:
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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTextFileReader)

			TDE2_API E_RESULT_CODE _onFree() override;
	};
}
