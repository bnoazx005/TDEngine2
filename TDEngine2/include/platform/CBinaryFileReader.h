/*!
	\file CBinaryFileReader.h
	\date 18.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseFile.h"
#include <fstream>


namespace TDEngine2
{
	class IInputStream;


	/*!
		\brief A factory function for creation objects of CBinaryFileReader's type

		\return A pointer to CBinaryFileReader's implementation
	*/

	TDE2_API IFile* CreateBinaryFileReader(IMountableStorage* pStorage, IStream* pStream, E_RESULT_CODE& result);


	/*!
		class CBinaryFileReader

		\brief The class represents a base reader of binary files
	*/

	class CBinaryFileReader : public virtual IBinaryFileReader, public CBaseFile
	{
		public:
			friend TDE2_API IFile* CreateBinaryFileReader(IMountableStorage*, IStream*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CBinaryFileReader)

			/*!
				\brief The method reads a continuous block of a file of specified size
				into a given buffer

				\param[out] pBuffer A buffer which will keep the read block of a file

				\paramp[in] bufferSize A size of a block that should be read

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Read(void* pBuffer, U32 bufferSize) override;

			/*!
				\brief The method asynchronously reads a continuous block of a file of specified size
				into a given buffer

				\param[in] size A size of a block that should be read

				\param[in] successCallback A callback that will be called if everything went fine

				\param[in] errorCallback A callback will be called if something went wrong
			*/

			TDE2_API void ReadAsync(U32 size, const TSuccessReadCallback& successCallback, const TErrorReadCallback& errorCallback) override;

			/*!
				\brief The method sets up a position of a file pointer

				\param[in] pos A new position of a file pointer

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetPosition(U32 pos) override;

			/*!
				\brief The method is a predicate that returns true if the end of a file has reached

				\return The method is a predicate that returns true if the end of a file has reached
			*/

			TDE2_API bool IsEOF() const override;

			/*!
				\brief The method returns a current position of a file pointer

				\return The method returns a current position of a file pointer
			*/

			TDE2_API U32 GetPosition() const override;

			/*!
				\brief The method returns a total size of a file in bytes

				\return The method returns a total size of a file in bytes
			*/

			TDE2_API U64 GetFileLength() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBinaryFileReader)

			TDE2_API E_RESULT_CODE _onFree() override;

			TDE2_API IInputStream* _getInputStream();
		private:
			IInputStream* mpCachedInputStream;
	};
}
