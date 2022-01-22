/*!
	\file CCsvFileReader.h
	\date 22.01.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseFile.h"
#include <fstream>
#include <memory>
#include <sstream>
#include "../../deps/rapidcsv/rapidcsv.h"


namespace TDEngine2
{
	class IInputStream;


	/*!
		\brief A factory function for creation objects of CCsvFileReader's type

		\return A pointer to CCsvFileReader's implementation
	*/

	TDE2_API IFile* CreateCsvFileReader(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result);


	/*!
		class CCsvFileReader

		\brief The class represents a base reader of CSV files
	*/

	class CCsvFileReader: public ICsvFileReader, public CBaseFile
	{
		public:
			friend TDE2_API IFile* CreateCsvFileReader(IMountableStorage*, TPtr<IStream>, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CCsvFileReader)

			/*!
				\brief The method opens specified file

				\param[in,out] pStorage A pointer to implementation of IMountableStorage
				\param[in,out] pStream A pointer to IStream implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Open(IMountableStorage* pStorage, TPtr<IStream> pStream) override;

			TDE2_API std::string GetCellValue(const std::string& colId, U32 rowId) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CCsvFileReader)

			TDE2_API E_RESULT_CODE _onFree() override;

			TDE2_API IInputStream* _getInputStream();
		protected:
			IInputStream*                       mpCachedInputStream;

			std::unique_ptr<rapidcsv::Document> mpCsvDocument;

			std::stringstream                   mIntermediateDataStream;
	};
}
