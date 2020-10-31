#include "./../../include/platform/CBinaryMeshFileReader.h"
#include "./../../include/graphics/IMesh.h"
#include "../../include/platform/IOStreams.h"
#include <cstring>


namespace TDEngine2
{
	const U32 CBinaryMeshFileReader::mMeshVersion = 256; // 00.01.0000 


	CBinaryMeshFileReader::CBinaryMeshFileReader() :
		CBinaryFileReader()
	{
	}

	E_RESULT_CODE CBinaryMeshFileReader::LoadMesh(IMesh*& pDestMesh)
	{
		auto headerResult = _readMeshFileHeader();
		if (headerResult.HasError()) // \note read and verify header
		{
			return headerResult.GetError();
		}

		E_RESULT_CODE result = RC_OK;
		
		if ((result = _readGeometryBlock(pDestMesh)) != RC_OK)
		{
			return result;
		}

#if 0
		if ((result = _readSceneDescBlock(pDestMesh, headerResult.Get().mSceneDescOffset)) != RC_OK)
		{
			return result;
		}
#endif

		return RC_OK;
	}

	E_RESULT_CODE CBinaryMeshFileReader::_onFree()
	{
		return RC_OK;
	}

	TResult<CBinaryMeshFileReader::TMeshFileHeader> CBinaryMeshFileReader::_readMeshFileHeader()
	{
		TMeshFileHeader header;

		E_RESULT_CODE result = Read(&header, sizeof(header));
		result = result | ((strcmp(header.mTag, "MESH") != 0 || header.mVersion != mMeshVersion) ? RC_INVALID_FILE : RC_OK);

		if (result != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<TMeshFileHeader>(header);
	}

	E_RESULT_CODE CBinaryMeshFileReader::_readGeometryBlock(IMesh*& pMesh)
	{
		E_RESULT_CODE result = SetPosition(16);

		U16 id = 0;

		if ((result = Read(&id, sizeof(U16))) != RC_OK)
		{
			return result;
		}

		assert(id == 0x2F);

		while (_readMeshEntryBlock(pMesh)) {}

		return RC_OK;
	}

	bool CBinaryMeshFileReader::_readMeshEntryBlock(IMesh*& pMesh)
	{
		E_RESULT_CODE result = RC_OK;

		U16 tag = 0x0;
		if ((result = Read(&tag, 2)) != RC_OK)
		{
			return result;
		}

		if (tag != 0x484D)
		{
			SetPosition(GetPosition() - 2); // roll back to previous position
			return false;
		}

		U16 objectId = 0x0;
		if ((result = Read(&objectId, 2)) != RC_OK)
		{
			return result;
		}

		U32 vertexCount = 0;
		U32 facesCount  = 0;

		if ((result = Read(&vertexCount, sizeof(U32))) != RC_OK)
		{
			return result;
		}

		if ((result = Read(&facesCount, sizeof(U32))) != RC_OK)
		{
			return result;
		}

		U32 padding = 0x0;
		if ((result = Read(&padding, sizeof(U32))) != RC_OK)
		{
			return result;
		}

		/// \note read vertices
		if ((result = Read(&tag, sizeof(U16))) != RC_OK)
		{
			return result;
		}
		assert(tag == 0xCD01);

		auto pStaticMesh = dynamic_cast<IStaticMesh*>(pMesh);

		TVector4 vecData;

		for (U32 i = 0; i < vertexCount; ++i)
		{
			result = result | Read(&vecData, sizeof(TVector4));
			pStaticMesh->AddPosition(vecData);
		}

		/// \note read first uv channel
		if ((result = Read(&tag, sizeof(U16))) != RC_OK)
		{
			return result;
		}
		assert(tag == 0xF002);
		
		for (U32 i = 0; i < vertexCount; ++i)
		{
			result = result | Read(&vecData, sizeof(TVector4));
			pStaticMesh->AddTexCoord0(TVector2(vecData.x, vecData.y));
		}

		/// \note read faces
		if ((result = Read(&tag, sizeof(U16))) != RC_OK)
		{
			return result;
		}
		assert(tag == 0xFF03);
		
		U16 format = 0x0;
		if ((result = Read(&format, sizeof(U16))) != RC_OK)
		{
			return result;
		}
		
		U32 faceIndices[3];

		for (U32 i = 0; i < facesCount / 3; ++i)
		{
			memset(faceIndices, 0, sizeof(faceIndices));

			result = result | Read(&faceIndices[0], format);
			result = result | Read(&faceIndices[1], format);
			result = result | Read(&faceIndices[2], format);

			pStaticMesh->AddFace(faceIndices);
		}

		return true;
	}

	E_RESULT_CODE CBinaryMeshFileReader::_readSceneDescBlock(IMesh*& pMesh, U32 offset)
	{
		return RC_NOT_IMPLEMENTED_YET;
	}


	IFile* CreateBinaryMeshFileReader(IFileSystem* pFileSystem, IStream* pStream, E_RESULT_CODE& result)
	{
		CBinaryMeshFileReader* pFileInstance = new (std::nothrow) CBinaryMeshFileReader();

		if (!pFileInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pFileInstance->Open(pFileSystem, pStream);

		if (result != RC_OK)
		{
			delete pFileInstance;

			pFileInstance = nullptr;
		}

		return dynamic_cast<IFile*>(pFileInstance);
	}
}