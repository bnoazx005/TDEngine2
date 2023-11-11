#include "../../include/graphics/CGlobalShaderProperties.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/InternalShaderData.h"


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

		mpGraphicsObjectManager = pGraphicsObjectManager;

		E_RESULT_CODE result = _initializeUniformsBuffers(pGraphicsObjectManager, TotalNumberOfInternalConstantBuffers);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;
		
		return RC_OK;
	}

	E_RESULT_CODE CGlobalShaderProperties::SetInternalUniformsBuffer(E_INTERNAL_UNIFORM_BUFFER_REGISTERS slot, const U8* pData, U32 dataSize)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		auto pCurrUniformsBuffer = mpGraphicsObjectManager->GetBufferPtr(mpInternalEngineUniforms[slot]);
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

		auto pGraphicsContext = mpGraphicsObjectManager->GetGraphicsContext();
		if (!pGraphicsContext)
		{
			return RC_FAIL;
		}

		return pGraphicsContext->SetConstantBuffer(slot, mpInternalEngineUniforms[slot]);
	}

	E_RESULT_CODE CGlobalShaderProperties::_initializeUniformsBuffers(IGraphicsObjectManager* pGraphicsObjectManager, U8 numOfBuffers)
	{
		E_INTERNAL_UNIFORM_BUFFER_REGISTERS currSlot;

		auto pGraphicsContext = pGraphicsObjectManager->GetGraphicsContext();
		if (!pGraphicsContext)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		for (U8 i = 0; i < numOfBuffers; ++i)
		{
			currSlot = static_cast<E_INTERNAL_UNIFORM_BUFFER_REGISTERS>(i);
						
			auto createBufferResult = pGraphicsObjectManager->CreateBuffer(
				{					 
					_getInternalBufferUsageType(currSlot), 
					E_BUFFER_TYPE::BT_CONSTANT_BUFFER, 
					_getInternalBufferSize(currSlot), 
					nullptr 
				});

			if (createBufferResult.HasError())
			{
				result = result | createBufferResult.GetError();
				continue;
			}

			mpInternalEngineUniforms[i] = createBufferResult.Get();
		}

		return RC_OK;
	}

	E_BUFFER_USAGE_TYPE CGlobalShaderProperties::_getInternalBufferUsageType(E_INTERNAL_UNIFORM_BUFFER_REGISTERS slot)
	{
		/// all the buffers except IUBR_CONSTANTS can be updated at any moment, IUBR_CONSTANTS buffer is attached on application's start

		return E_BUFFER_USAGE_TYPE::DYNAMIC;
		/*switch (slot)
		{
			case IUBR_PER_FRAME:
			case IUBR_PER_OBJECT:
			case IUBR_RARE_UDATED:
				return BUT_DYNAMIC;
			case IUBR_CONSTANTS:
				return BUT_DEFAULT;
		}

		return BUT_DEFAULT;*/
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
		return CREATE_IMPL(IGlobalShaderProperties, CGlobalShaderProperties, result, pGraphicsObjectManager);
	}
}