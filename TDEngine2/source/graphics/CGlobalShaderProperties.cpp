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

		result = _initializeShaderBuffers(pGraphicsObjectManager);
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

		auto pCurrUniformsBuffer = mpGraphicsObjectManager->GetBufferPtr(mInternalEngineUniforms[slot]);
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

		return pGraphicsContext->SetConstantBuffer(slot, mInternalEngineUniforms[slot]);
	}

	E_RESULT_CODE CGlobalShaderProperties::SetInternalShaderBuffer(E_INTERNAL_SHADER_BUFFERS_REGISTERS slot, const U8* pData, U32 dataSize)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		auto pCurrTypedBuffer = mpGraphicsObjectManager->GetBufferPtr(mInternalShaderBuffers[slot]);
		if (!pCurrTypedBuffer)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = pCurrTypedBuffer->Map(BMT_WRITE_DISCARD);

		if (result != RC_OK)
		{
			return result;
		}

		if ((result = pCurrTypedBuffer->Write(pData, dataSize)) != RC_OK)
		{
			return result;
		}

		pCurrTypedBuffer->Unmap();

		auto pGraphicsContext = mpGraphicsObjectManager->GetGraphicsContext();
		if (!pGraphicsContext)
		{
			return RC_FAIL;
		}

		return pGraphicsContext->SetStructuredBuffer(static_cast<U32>(slot), mInternalShaderBuffers[slot]);
	}


	static U32 GetInternalBufferSize(E_INTERNAL_UNIFORM_BUFFER_REGISTERS slot)
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

		TDE2_UNREACHABLE();
		return 0;
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
					E_BUFFER_USAGE_TYPE::DYNAMIC,
					E_BUFFER_TYPE::CONSTANT, 
					GetInternalBufferSize(currSlot), 
					nullptr,
				});

			if (createBufferResult.HasError())
			{
				result = result | createBufferResult.GetError();
				continue;
			}

			mInternalEngineUniforms[i] = createBufferResult.Get();
		}

		return RC_OK;
	}

	E_RESULT_CODE CGlobalShaderProperties::_initializeShaderBuffers(IGraphicsObjectManager* pGraphicsObjectManager)
	{
		auto pGraphicsContext = pGraphicsObjectManager->GetGraphicsContext();
		if (!pGraphicsContext)
		{
			return RC_FAIL;
		}

		static const std::vector<std::tuple<E_INTERNAL_SHADER_BUFFERS_REGISTERS, E_BUFFER_USAGE_TYPE, USIZE, USIZE>> allRegisters
		{
			{ E_INTERNAL_SHADER_BUFFERS_REGISTERS::LIGHTS_SLOT, E_BUFFER_USAGE_TYPE::DYNAMIC, MaxLightsCount * sizeof(TLightData), sizeof(TLightData) },
		};

		E_RESULT_CODE result = RC_OK;

		E_INTERNAL_SHADER_BUFFERS_REGISTERS currSlot = E_INTERNAL_SHADER_BUFFERS_REGISTERS::COUNT;
		E_BUFFER_USAGE_TYPE usageType = E_BUFFER_USAGE_TYPE::DEFAULT;
		USIZE bufferSize = 0;
		USIZE strideSize = 0;

		for (USIZE i = 0; i < allRegisters.size(); ++i)
		{			
			std::tie(currSlot, usageType, bufferSize, strideSize) = allRegisters[i];

			auto createBufferResult = pGraphicsObjectManager->CreateBuffer(
				{
					usageType,
					E_BUFFER_TYPE::STRUCTURED,
					bufferSize,
					nullptr,
					bufferSize, 
					false, 
					strideSize, 
					E_STRUCTURED_BUFFER_TYPE::DEFAULT
				});

			if (createBufferResult.HasError())
			{
				result = result | createBufferResult.GetError();
				continue;
			}

			mInternalShaderBuffers.emplace(currSlot, createBufferResult.Get());
		}

		return result;
	}


	TDE2_API IGlobalShaderProperties* CreateGlobalShaderProperties(IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGlobalShaderProperties, CGlobalShaderProperties, result, pGraphicsObjectManager);
	}
}