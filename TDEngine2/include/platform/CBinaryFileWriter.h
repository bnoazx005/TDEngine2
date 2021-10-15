/*!
	\file CBinaryFileWriter.h
	\date 19.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseFile.h"
#include <fstream>


namespace TDEngine2
{
	class IOutputStream;

	/*!
		\brief A factory function for creation objects of CBinaryFileWriter's type

		\return A pointer to CBinaryFileWriter's implementation
	*/

	TDE2_API IFile* CreateBinaryFileWriter(IMountableStorage* pStorage, IStream* pStream, E_RESULT_CODE& result);


	/*!
		class CBinaryFileWriter

		\brief The class represents a base writer of binary files
	*/

	class CBinaryFileWriter : public virtual IBinaryFileWriter, public CBaseFile
	{
		public:
			friend TDE2_API IFile* CreateBinaryFileWriter(IMountableStorage*, IStream*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CBinaryFileWriter)

			/*!
				\brief The method writes data of specified size into a file

				\param[in] pBuffer A pointer to data's buffer

				\param[in] bufferSize A size in bytes of data should be written

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Write(const void* pBuffer, TSizeType bufferSize) override;

			/*!
				\brief The method writes data in asynchronous manner into a given file

				\param[in] pBuffer A pointer to data's buffer

				\param[in] bufferSize A size in bytes of data should be written

				\param[in] successCallback A callback that should be called when data was successfully written into the file

				\param[in] errorCallback A callback that will be called when some error's happened during the process
			*/

			TDE2_API void WriteAsync(const void* pBuffer, TSizeType bufferSize, const TSuccessWriteCallback& successCallback,
									 const TErrorWriteCallback& errorCallback) override;

			/*!
				\brief The method writes all unsaved buffer data into the destination file

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Flush() override;

			/*!
				\brief The method sets up a position of a file pointer

				\param[in] pos A new position of a file pointer

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetPosition(TSizeType pos) override;

			/*!
				\brief The method is a predicate that returns true if the end of a file has reached

				\return The method is a predicate that returns true if the end of a file has reached
			*/

			TDE2_API bool IsEOF() const override;

			/*!
				\brief The method returns a current position of a file pointer

				\return The method returns a current position of a file pointer
			*/

			TDE2_API TSizeType GetPosition() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBinaryFileWriter)

			TDE2_API E_RESULT_CODE _onInit() override;
			TDE2_API E_RESULT_CODE _onFree() override;

			TDE2_API IOutputStream* _getOutputStream();
		private:
			IOutputStream* mpCachedOutputStream;
	};
}
