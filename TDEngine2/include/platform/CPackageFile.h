/*!
	\file CPackageFile.h
	\date 06.11.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../platform/CBinaryFileReader.h"
#include "../platform/CBinaryFileWriter.h"
#include <string>


namespace TDEngine2
{
	/*!
		\brief The structure of a simple package file looks like the following below

		> beginning of a file ===========================
		
		      PackageFileHeader 
		----------------------------
		        Files Data
		....
		----------------------------
		    FilesTableDescription
		Entry1, Entry2, .... EntryN

		< end of the file ===============================
	*/

#pragma pack(push, 1)

	typedef struct TPackageFileHeader
	{
		const C8 mTag[4] { "PAK" };

		const U16 mVersion = 0x100;
		const U16 mPadding = 0x0;

		U32 mEntitiesCount = 0;

		U64 mFilesTableOffset = 0;
		U64 mFilesTableSize = 0;
	} TPackageFileHeader, *TPackageFileHeaderPtr;

#pragma pack(pop)


	typedef struct TPackageFileEntryInfo
	{
		std::string mFilename;

		U64 mDataBlockOffset = 0;
		U64 mDataBlockSize = 0;
	} TPackageFileEntryInfo, *TPackageFileEntryInfoPtr;


	/*!
		\brief A factory function for creation objects of CPackageFileReader's type

		\return A pointer to CPackageFileReader's implementation
	*/

	TDE2_API IFile* CreatePackageFileReader(IMountableStorage* pStorage, IStream* pStream, E_RESULT_CODE& result);


	/*!
		class CPackageFileReader

		\brief The class represents a base reader of packages files
	*/

	class CPackageFileReader : public CBinaryFileReader, public IPackageFileReader
	{
		public:
			friend TDE2_API IFile* CreatePackageFileReader(IMountableStorage*, IStream*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CPackageFileReader)

			TDE2_API const TPackageFileHeader& GetPackageHeader() const override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPackageFileReader)

			TDE2_API E_RESULT_CODE _onInit() override;
			//TDE2_API E_RESULT_CODE _onFree() override;

			TDE2_API E_RESULT_CODE _readPackageHeader();
			TDE2_API E_RESULT_CODE _readFilesTableDescription();
		private:
			TPackageFileHeader mCurrHeader;
			std::vector<TPackageFileEntryInfo> mFilesTable;
	};


	/*!
		\brief A factory function for creation objects of CPackageFileWriter's type

		\return A pointer to CPackageFileWriter's implementation
	*/

	TDE2_API IFile* CreatePackageFileWriter(IMountableStorage* pStorage, IStream* pStream, E_RESULT_CODE& result);


	/*!
		class CPackageFileWriter

		\brief The class represents a base reader of packages files
	*/

	class CPackageFileWriter : public CBinaryFileWriter, public IPackageFileWriter
	{
		public:
			friend TDE2_API IFile* CreatePackageFileWriter(IMountableStorage*, IStream*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CPackageFileWriter)

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPackageFileWriter)

			TDE2_API E_RESULT_CODE _onInit() override;
			TDE2_API E_RESULT_CODE _onFree() override;
			
			TDE2_API E_RESULT_CODE _writeFileInternal(TypeId fileTypeId, const std::string& path, const IFileReader& file) override;

			TDE2_API E_RESULT_CODE _writePackageHeader();
			TDE2_API E_RESULT_CODE _writeFilesTableDescription();
		private:
			TPackageFileHeader mCurrHeader;
			std::vector<TPackageFileEntryInfo> mFilesTable;
	};
}