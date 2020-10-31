/*!
	\brief CImageFileWriter.h

	\date 15.09.2019
	\author Ildar Kasimov
*/

#pragma once


#include "CBaseFile.h"
#include <fstream>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CImageFileWriter's type

		\return A pointer to CImageFileWriter's implementation
	*/

	TDE2_API IFile* CreateImageFileWriter(IFileSystem* pFileSystem, IStream* pStream, E_RESULT_CODE& result);


	/*!
		class CImageFileWriter

		\brief The class represents a writer of image files (png, tga, bmp, hdr)
	*/

	class CImageFileWriter : public IImageFileWriter, public CBaseFile
	{
		protected:
			enum class E_IMAGE_FILE_TYPE: U8
			{
				PNG,
				BMP,
				TGA,
				HDR,
			};
		public:
			friend TDE2_API IFile* CreateImageFileWriter(IFileSystem*, IStream*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CImageFileWriter)

			/*!
				\brief The method writes image data into a file

				\param[in, out] pTexture A pointer to ITexture2D implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Write(class ITexture2D* pTexture) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CImageFileWriter)

			TDE2_API E_RESULT_CODE _onFree() override;

			TDE2_API E_IMAGE_FILE_TYPE _getImageFileType(const std::string& filename) const;
	};
}
