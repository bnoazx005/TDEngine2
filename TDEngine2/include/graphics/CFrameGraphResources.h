/*!
	\file CFrameGraphResources.h
	\date 13.05.2024
	\authors Kasimov Ildar
*/

#pragma once


#include "IGraphicsObjectManager.h"
#include "ITexture.h"
#include "IBuffer.h"


namespace TDEngine2
{
	/*!
		struct TFrameGraphTexture
	*/

	struct TFrameGraphTexture
	{
		struct TDesc : TInitTextureImplParams {};

		TDE2_API E_RESULT_CODE Acquire(IGraphicsObjectManager* pGraphicsObjectManager, const TDesc& desc);
		TDE2_API E_RESULT_CODE Release(IGraphicsObjectManager* pGraphicsObjectManager);

		TDE2_API void BeforeReadOp();
		TDE2_API void BeforeWriteOp();

		TTextureHandleId mTextureHandle = TTextureHandleId::Invalid;
	};


	/*!
		struct TFrameGraphBuffer
	*/

	struct TFrameGraphBuffer
	{
		struct TDesc : TInitBufferParams {};

		TDE2_API E_RESULT_CODE Acquire(IGraphicsObjectManager* pGraphicsObjectManager, const TDesc& desc);
		TDE2_API E_RESULT_CODE Release(IGraphicsObjectManager* pGraphicsObjectManager);

		TDE2_API void BeforeReadOp();
		TDE2_API void BeforeWriteOp();

		TBufferHandleId mBufferHandle = TBufferHandleId::Invalid;
	};
}