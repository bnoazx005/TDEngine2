#include "./../../include/graphics/CDebugUtility.h"
#include "./../../include/graphics/IGraphicsObjectManager.h"
#include "./../../include/graphics/IRenderer.h"
#include "./../../include/graphics/CRenderQueue.h"
#include "./../../include/graphics/IVertexDeclaration.h"
#include "./../../include/graphics/IVertexBuffer.h"


namespace TDEngine2
{
	CDebugUtility::CDebugUtility():
		CBaseObject()
	{
	}

	E_RESULT_CODE CDebugUtility::Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pRenderer || !pGraphicsObjectManager)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsObjectManager = pGraphicsObjectManager;

		mpRenderQueue = pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_DEBUG);

		mpLinesVertDeclaration = mpGraphicsObjectManager->CreateVertexDeclaration().Get();
		mpLinesVertDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });
		mpLinesVertDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_COLOR });

		mpLinesVertexBuffer = mpGraphicsObjectManager->CreateVertexBuffer(BUT_DYNAMIC, sizeof(TVector3) * 4096, nullptr).Get();

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CDebugUtility::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CDebugUtility::PreRender()
	{
		mpLinesVertexBuffer->Map(BMT_WRITE_DISCARD);
		mpLinesVertexBuffer->Write(&mLinesDataBuffer[0], sizeof(TLineVertex) * mLinesDataBuffer.size());
		mpLinesVertexBuffer->Unmap();

		auto pDrawLinesCommand = mpRenderQueue->SubmitDrawCommand<TDrawCommand>(0);

		pDrawLinesCommand->mpVertexBuffer      = mpLinesVertexBuffer;
		pDrawLinesCommand->mPrimitiveType      = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_LINE_LIST;
		pDrawLinesCommand->mMaterialName       = "DebugMaterial.material";
		pDrawLinesCommand->mpVertexDeclaration = mpLinesVertDeclaration;
		pDrawLinesCommand->mNumOfVertices      = mLinesDataBuffer.size();
	}

	void CDebugUtility::PostRender()
	{
		mLinesDataBuffer.clear();
	}

	void CDebugUtility::DrawLine(const TVector3& start, const TVector3& end, const TColor32F& color)
	{
		if (!mIsInitialized)
		{
			return;
		}

		mLinesDataBuffer.push_back({ { start, 1.0f }, color });
		mLinesDataBuffer.push_back({ { end, 1.0f }, color });
	}

	void CDebugUtility::DrawText(const TVector2& screenPos, const CU8String& str, const TColor32F& color)
	{
		TDE2_UNIMPLEMENTED();
	}


	TDE2_API IDebugUtility* CreateDebugUtility(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		CDebugUtility* pDebugUtilityInstance = new (std::nothrow) CDebugUtility();

		if (!pDebugUtilityInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pDebugUtilityInstance->Init(pRenderer, pGraphicsObjectManager);

		if (result != RC_OK)
		{
			delete pDebugUtilityInstance;
			
			pDebugUtilityInstance = nullptr;
		}

		return pDebugUtilityInstance;
	}
}