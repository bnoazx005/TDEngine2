#include "./../../include/platform/CImageFileWriter.h"
#include "./../../include/core/IFileSystem.h"
#include "./../../include/core/IJobManager.h"
#include "./../../include/graphics/ITexture2D.h"
#include "./../../include/utils/Utils.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable:4996)
#include "./../../deps/stb/stb_image_write.h"
#pragma warning(pop)
#undef STB_IMAGE_WRITE_IMPLEMENTATION
#include <filesystem>
#include <string>


namespace TDEngine2
{
	CImageFileWriter::CImageFileWriter() :
		CBaseFile()
	{
		mCreationFlags = std::ios::out | std::ios::binary;
	}

	E_RESULT_CODE CImageFileWriter::Write(ITexture2D* pTexture)
	{
		if (!pTexture)
		{
			return RC_INVALID_ARGS;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		if (!mFile.is_open())
		{
			return RC_FAIL;
		}		

		E_IMAGE_FILE_TYPE imageType = _getImageFileType(mName);

		I32 width  = static_cast<I32>(pTexture->GetWidth());
		I32 height = static_cast<I32>(pTexture->GetHeight());

		I8 numOfChannels = CFormatUtils::GetNumOfChannelsOfFormat(pTexture->GetFormat());

		auto pTextureData = pTexture->GetInternalData();
		U8* pData = pTextureData.get();
		
		switch (imageType)
		{
			case E_IMAGE_FILE_TYPE::BMP:
				stbi_write_bmp(mName.c_str(), width, height, numOfChannels, pData);
				break;
			case E_IMAGE_FILE_TYPE::TGA:
				stbi_write_tga(mName.c_str(), width, height, numOfChannels, pData);
				break;
			case E_IMAGE_FILE_TYPE::PNG:
				stbi_write_png(mName.c_str(), width, height, numOfChannels, pData, width * numOfChannels);
				break;	
			case E_IMAGE_FILE_TYPE::HDR:
				stbi_write_hdr(mName.c_str(), width, height, numOfChannels, reinterpret_cast<const F32*>(pData));
					break;
			default:
				TDE2_UNIMPLEMENTED();
				break;
		}

		if (mFile.bad())
		{
			return RC_FAIL;
		}

		return RC_OK;
	}
	
	E_RESULT_CODE CImageFileWriter::_onFree()
	{
		return RC_OK;
	}

	CImageFileWriter::E_IMAGE_FILE_TYPE CImageFileWriter::_getImageFileType(const std::string& filename) const
	{
		E_IMAGE_FILE_TYPE imageType = E_IMAGE_FILE_TYPE::HDR;

		std::string fileExtension = std::filesystem::path(filename).extension().string();

		if (fileExtension == ".png")
		{
			imageType = E_IMAGE_FILE_TYPE::PNG;
		}
		else if (fileExtension == ".tga")
		{
			imageType = E_IMAGE_FILE_TYPE::TGA;
		}
		else if (fileExtension == ".bmp")
		{
			imageType = E_IMAGE_FILE_TYPE::BMP;
		}
		else if (fileExtension == ".hdr")
		{
			imageType = E_IMAGE_FILE_TYPE::HDR;
		}
		else
		{
			TDE2_UNIMPLEMENTED();
		}

		return imageType;
	}


	IFile* CreateImageFileWriter(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result)
	{
		CImageFileWriter* pFileInstance = new (std::nothrow) CImageFileWriter();

		if (!pFileInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pFileInstance->Open(pFileSystem, filename);

		if (result != RC_OK)
		{
			delete pFileInstance;

			pFileInstance = nullptr;
		}

		return dynamic_cast<IFile*>(pFileInstance);
	}
}