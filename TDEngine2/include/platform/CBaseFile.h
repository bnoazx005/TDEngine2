/*!
	\file CBaseFile.h
	\date 21.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/IFile.h"
#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include <fstream>


namespace TDEngine2
{
	class IFileSystem;


	/*!
		class CBaseFile

		\brief The class implements a common functionality of IFile interface
	*/

	class CBaseFile: public virtual IFile
	{
		public:
			TDE2_REGISTER_TYPE(CBaseFile)

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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseFile)

			TDE2_API virtual E_RESULT_CODE _onFree() = 0;
		protected:
			std::ifstream mFile;

			std::string   mName;

			IFileSystem*  mpFileSystemInstance;
	};
}
