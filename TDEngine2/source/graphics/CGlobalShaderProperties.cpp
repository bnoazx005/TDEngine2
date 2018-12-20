#include "./../../include/graphics/CGlobalShaderProperties.h"
#include "./../../include/graphics/IGraphicsObjectManager.h"
#include "./../../include/graphics/IConstantBuffer.h"
#include "./../../include/graphics/InternalShaderData.h"


namespace TDEngine2
{
	CGlobalShaderProperties::CGlobalShaderProperties():
		CBaseObject()
	{
	}

	E_RESULT_CODE CGlobalShaderProperties::Init(IGraphicsObjectManager* pGraphicsObjectManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsObjectManager)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = _initializeUniformsBuffers(pGraphicsObjectManager, TotalNumberOfInternalConstantBuffers);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;
		
		return RC_OK;
	}

	E_RESULT_CODE CGlobalShaderProperties::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = _freeAllUniformsBuffers();

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CGlobalShaderProperties::SetInternalUniformsBuffer(E_INTERNAL_UNIFORM_BUFFER_REGISTERS slot, const U8* pData, U32 dataSize)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		IConstantBuffer* pCurrUniformsBuffer = mpInternalEngineUniforms[slot];

		if (!pCurrUniformsBuffer)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = pCurrUniformsBuffer->Map(BMT_WRITE_DISCARD);

		if (result != RC_OK)
		{
			return result;
		}

		if ((result = pCurrUniformsBuffer->Write(pData, dataSize)) != RC_OK)
		{
			return result;
		}

		pCurrUniformsBuffer->Unmap();

		pCurrUniformsBuffer->Bind(slot);

		return RC_OK;
	}

	E_RESULT_CODE CGlobalShaderProperties::_initializeUniformsBuffers(IGraphicsObjectManager* pGraphicsObjectManager, U8 numOfBuffers)
	{
		E_INTERNAL_UNIFORM_BUFFER_REGISTERS currSlot;

		for (U8 i = 0; i < numOfBuffers; ++i)
		{
			currSlot = static_cast<E_INTERNAL_UNIFORM_BUFFER_REGISTERS>(i);

			mpInternalEngineUniforms[i] = pGraphicsObjectManager->CreateConstantBuffer(_getInternalBufferUsageType(currSlot),
																					   _getInternalBufferSize(currSlot), 
																					   nullptr).Get();
		}

		return RC_OK;
	}

	E_RESULT_CODE CGlobalShaderProperties::_freeAllUniformsBuffers()
	{
		E_RESULT_CODE result = RC_OK;

		IConstantBuffer* pCurrUniformsBuffer = nullptr;

		for (U8 i = 0; i < TotalNumberOfInternalConstantBuffers; ++i)
		{
			pCurrUniformsBuffer = mpInternalEngineUniforms[i];

			if (!pCurrUniformsBuffer)
			{
				continue;
			}

			if ((result = pCurrUniformsBuffer->Free()) != RC_OK)
			{
				return result;
			}
		}

		return result;
	}

	E_BUFFER_USAGE_TYPE CGlobalShaderProperties::_getInternalBufferUsageType(E_INTERNAL_UNIFORM_BUFFER_REGISTERS slot)
	{
		/// all the buffers except IUBR_CONSTANTS can be updated at any moment, IUBR_CONSTANTS buffer is attached on application's start

		switch (slot)
		{
			case IUBR_PER_FRAME:
			case IUBR_PER_OBJECT:
			case IUBR_RARE_UDATED:
				return BUT_DYNAMIC;
			case IUBR_CONSTANTS:
				return BUT_DEFAULT;
		}

		return BUT_DEFAULT;
	}

	U32 CGlobalShaderProperties::_getInternalBufferSize(E_INTERNAL_UNIFORM_BUFFER_REGISTERS slot)
	{
		switch (slot)
		{
			case IUBR_PER_FRAME:
				return sizeof(TPerFrameShaderData);
			case IUBR_PER_OBJECT:
				return sizeof(TPerObjectShaderData);
			case IUBR_RARE_UDATED:
				return sizeof(TRareUpdateShaderData);
			case IUBR_CONSTANTS:
				return sizeof(TConstantShaderData);
		}

		return 0;
	}


	TDE2_API IGlobalShaderProperties* CreateGlobalShaderProperties(IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		CGlobalShaderProperties* pRenderer = new (std::nothrow) CGlobalShaderProperties();

		if (!pRenderer)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pRenderer->Init(pGraphicsObjectManager);

		if (result != RC_OK)
		{
			delete pRenderer;

			pRenderer = nullptr;
		}

		return pRenderer;
	}
}