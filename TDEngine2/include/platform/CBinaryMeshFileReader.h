/*!
	\file CBinaryMeshFileReader.h
	\date 15.12.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "CBinaryFileReader.h"
#include <fstream>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CBinaryMeshFileReader's type

		\return A pointer to CBinaryMeshFileReader's implementation
	*/

	TDE2_API IFile* CreateBinaryMeshFileReader(IMountableStorage* pStorage, IStream* pStream, E_RESULT_CODE& result);


	/*!
		class CBinaryMeshFileReader

		\brief The class represents a base reader of binary mesh's files
		
		\todo Add support of different endianness
	*/

	class CBinaryMeshFileReader : public CBinaryFileReader, public IBinaryMeshFileReader
	{
		public:
			friend TDE2_API IFile* CreateBinaryMeshFileReader(IMountableStorage*, IStream*, E_RESULT_CODE&);
		private:
			typedef struct TMeshFileHeader
			{
				C8  mTag[4];
				U32 mVersion;
				U32 mSceneDescOffset;
				U32 mPadding;
			} TMeshFileHeader, *TMeshFileHeaderPtr;
		public:
			TDE2_REGISTER_TYPE(CBinaryMeshFileReader)

			/*!
				\brief The method reads binary mesh file into the given memory

				\param[out] pDestMesh A pointer to IMesh implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE LoadMesh(IMesh*& pDestMesh) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBinaryMeshFileReader)

			TDE2_API E_RESULT_CODE _onInit() override;
			TDE2_API E_RESULT_CODE _onFree() override;

			TDE2_API TResult<TMeshFileHeader> _readMeshFileHeader();

			TDE2_API E_RESULT_CODE _readGeometryBlock(IMesh*& pMesh);

			TDE2_API bool _readMeshEntryBlock(IMesh*& pMesh);

			TDE2_API E_RESULT_CODE _readSceneDescBlock(IMesh*& pMesh, U32 offset);
		protected:
			static const U32 mMeshVersion;
	};
}
