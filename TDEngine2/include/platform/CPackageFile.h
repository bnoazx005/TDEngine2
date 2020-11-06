/*!
	\file CPackageFile.h
	\date 06.11.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../platform/CBinaryFileReader.h"
#include "../platform/CBinaryFileWriter.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CPackageFileReader's type

		\return A pointer to CPackageFileReader's implementation
	*/

	TDE2_API IFile* CreatePackageFileReader(IMountableStorage* pStorage, IStream* pStream, E_RESULT_CODE& result);


	/*!
		class CPackageFileReader

		\brief The class represents a base reader of packages files
	*/

	class CPackageFileReader : public CBinaryFileReader
	{
		public:
			friend TDE2_API IFile* CreatePackageFileReader(IMountableStorage*, IStream*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CPackageFileReader)

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPackageFileReader)

			//TDE2_API E_RESULT_CODE _onFree() override;
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

			//TDE2_API E_RESULT_CODE _onFree() override;
			
			TDE2_API E_RESULT_CODE _writeFileInternal(TypeId fileTypeId, const std::string& path, const IFile& file) override;
	};
}