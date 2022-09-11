/*!
	\file CTextFileWriter.h
	\date 11.09.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseFile.h"
#include <fstream>


namespace TDEngine2
{
	class IOutputStream;


	/*!
		\brief A factory function for creation objects of CTextFileWriter's type

		\return A pointer to CTextFileWriter's implementation
	*/

	TDE2_API IFile* CreateTextFileWriter(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result);


	/*!
		class CTextFileWriter

		\brief The class represents a base writer of text files
	*/

	class CTextFileWriter: public ITextFileWriter, public CBaseFile
	{
		public:
			friend TDE2_API IFile* CreateTextFileWriter(IMountableStorage*, TPtr<IStream>, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CTextFileWriter)

			/*!
				\brief The method writes a given line and moves the file cursor to a new line

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE WriteLine(const std::string& value) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTextFileWriter)

			TDE2_API E_RESULT_CODE _onFree() override;

			TDE2_API IOutputStream* _getOutputStream();
		protected:
			IOutputStream* mpCachedOutputStream;
	};
}
