#include "./../../include/graphics/CDebugUtility.h"
#include "./../../include/graphics/IGraphicsObjectManager.h"
#include "./../../include/graphics/IRenderer.h"
#include "./../../include/graphics/CRenderQueue.h"
#include "./../../include/graphics/IVertexDeclaration.h"
#include "./../../include/graphics/IVertexBuffer.h"
#include "./../../include/core/IResourceManager.h"
#include "./../../include/core/CFont.h"
#include "./../../include/graphics/CBaseMaterial.h"
#include <algorithm>
#include <iterator>


namespace TDEngine2
{
	const std::string CDebugUtility::mDefaultDebugMaterialName = "DebugMaterial.material";
	const std::string CDebugUtility::mTextMaterialName         = "DebugTextMaterial.material";

	CDebugUtility::CDebugUtility():
		CBaseObject()
	{
	}

	E_RESULT_CODE CDebugUtility::Init(IResourceManager* pResourceManager, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pRenderer || !pGraphicsObjectManager || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager       = pResourceManager;
		mpGraphicsObjectManager = pGraphicsObjectManager;

		mpRenderQueue = pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_DEBUG);

		mpLinesVertDeclaration = mpGraphicsObjectManager->CreateVertexDeclaration().Get();
		mpLinesVertDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });
		mpLinesVertDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_COLOR });

		mpLinesVertexBuffer = mpGraphicsObjectManager->CreateVertexBuffer(BUT_DYNAMIC, sizeof(TVector3) * 4096, nullptr).Get();

		mpSystemFont = mpResourceManager->Load<CFont>("Arial"); /// \note load system font, which is "Arial" font

		mpTextVertDeclaration = mpGraphicsObjectManager->CreateVertexDeclaration().Get();
		mpTextVertDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });

		mpTextVertexBuffer = mpGraphicsObjectManager->CreateVertexBuffer(BUT_DYNAMIC, sizeof(TTextVertex) * 4096, nullptr).Get();
		mpTextIndexBuffer  = mpGraphicsObjectManager->CreateIndexBuffer(BUT_DYNAMIC, TDEngine2::IFT_INDEX16, sizeof(U16) * 9072, &_buildTextIndexBuffer(2048)[0]).Get();

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
		if (!mLinesDataBuffer.empty())
		{
			mpLinesVertexBuffer->Map(BMT_WRITE_DISCARD);
			mpLinesVertexBuffer->Write(&mLinesDataBuffer[0], sizeof(TLineVertex) * mLinesDataBuffer.size());
			mpLinesVertexBuffer->Unmap();

			auto pDrawLinesCommand = mpRenderQueue->SubmitDrawCommand<TDrawCommand>(0);

			pDrawLinesCommand->mpVertexBuffer = mpLinesVertexBuffer;
			pDrawLinesCommand->mPrimitiveType = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_LINE_LIST;
			pDrawLinesCommand->mpMaterialHandler = mpResourceManager->Load<CBaseMaterial>(mDefaultDebugMaterialName);
			pDrawLinesCommand->mpVertexDeclaration = mpLinesVertDeclaration;
			pDrawLinesCommand->mNumOfVertices = mLinesDataBuffer.size();
		}

		/// \note draw debug text onto the screen
		if (!mTextDataBuffer.empty())
		{
			mpTextVertexBuffer->Map(BMT_WRITE_DISCARD);
			mpTextVertexBuffer->Write(&mTextDataBuffer[0], sizeof(TTextVertex) * mTextDataBuffer.size());
			mpTextVertexBuffer->Unmap();

			/*auto indices = _buildTextIndexBuffer(mTextDataBuffer.size() / 4);

			mpTextIndexBuffer->Map(BMT_WRITE_DISCARD);
			mpTextIndexBuffer->Write(&indices[0], sizeof(U16) * indices.size());
			mpTextIndexBuffer->Unmap();*/

			auto pDrawTextCommand = mpRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(1);

			pDrawTextCommand->mpVertexBuffer = mpTextVertexBuffer;
			pDrawTextCommand->mpIndexBuffer = mpTextIndexBuffer;
			pDrawTextCommand->mPrimitiveType = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
			pDrawTextCommand->mpMaterialHandler = mpResourceManager->Load<CBaseMaterial>(mTextMaterialName);
			pDrawTextCommand->mpVertexDeclaration = mpTextVertDeclaration;
			pDrawTextCommand->mStartIndex = 0;
			pDrawTextCommand->mStartVertex = 0;
			pDrawTextCommand->mNumOfIndices = mTextDataBuffer.size() * 4;
		}
		
	}

	void CDebugUtility::PostRender()
	{
		mLinesDataBuffer.clear();
		mTextDataBuffer.clear();
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
		if (!mpSystemFont->IsValid())
		{
			mpSystemFont = mpResourceManager->Load<CFont>("Arial");
		}

		auto pSystemFontResource = dynamic_cast<IFont*>(mpSystemFont->Get(RAT_BLOCKING));

		auto& generatedMesh = pSystemFontResource->GenerateMesh(screenPos, str);

		std::transform(generatedMesh.begin(), generatedMesh.end(), std::back_inserter(mTextDataBuffer), [](const TVector4& v)
		{
			return TTextVertex { v };
		});
	}

	std::vector<U16> CDebugUtility::_buildTextIndexBuffer(U32 textLength) const
	{
		std::vector<U16> indices;

		for (U32 i = 0, index = 0; i < textLength; ++i, index += 4)
		{
			indices.push_back(index); /// first face
			indices.push_back(index + 1);
			indices.push_back(index + 2);

			indices.push_back(index + 2); /// second face
			indices.push_back(index + 1);
			indices.push_back(index + 3);
		}

		return indices;
	}


	TDE2_API IDebugUtility* CreateDebugUtility(IResourceManager* pResourceManager, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		CDebugUtility* pDebugUtilityInstance = new (std::nothrow) CDebugUtility();

		if (!pDebugUtilityInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pDebugUtilityInstance->Init(pResourceManager, pRenderer, pGraphicsObjectManager);

		if (result != RC_OK)
		{
			delete pDebugUtilityInstance;
			
			pDebugUtilityInstance = nullptr;
		}

		return pDebugUtilityInstance;
	}
}