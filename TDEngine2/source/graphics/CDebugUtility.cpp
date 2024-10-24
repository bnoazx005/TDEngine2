#include "../../include/graphics/CDebugUtility.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/IVertexBuffer.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/CFont.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/math/TAABB.h"
#include "../../include/math/MathUtils.h"
#include "../../include/math/TVector3.h"
#include "../../include/graphics/CGeometryBuilder.h"
#include "../../include/editor/CPerfProfiler.h"
#include <algorithm>
#include <iterator>
#include <functional>
#include <cmath>
#include <tuple>


namespace TDEngine2
{
	// \todo Remove these decls and replace them with some options in settings file
	const std::string CDebugUtility::mDefaultDebugMaterialName = "DefaultMaterials/DebugMaterial.material";
	const std::string CDebugUtility::mTextMaterialName         = "DefaultMaterials/DebugTextMaterial.material";

	const U32 CDebugUtility::mMaxLinesVerticesCount = 65536;

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

		E_RESULT_CODE result = RC_OK;

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

		mpLinesVertexBuffer = mpGraphicsObjectManager->CreateVertexBuffer(BUT_DYNAMIC, sizeof(TLineVertex) * mMaxLinesVerticesCount, nullptr).Get();

		mSystemFontHandle = mpResourceManager->Load<IFont>("OpenSans.font"); /// \note load system font, which is "OpenSans" font

		mpTextVertDeclaration = mpGraphicsObjectManager->CreateVertexDeclaration().Get();
		mpTextVertDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });

		mpTextVertexBuffer = mpGraphicsObjectManager->CreateVertexBuffer(BUT_DYNAMIC, sizeof(TTextVertex) * 4096, nullptr).Get();
		mpTextIndexBuffer  = mpGraphicsObjectManager->CreateIndexBuffer(BUT_DYNAMIC, TDEngine2::IFT_INDEX16, sizeof(U16) * 9072, &_buildTextIndexBuffer(2048)[0]).Get();

		mpCrossesVertexBuffer = mpGraphicsObjectManager->CreateVertexBuffer(BUT_DYNAMIC, sizeof(TLineVertex) * mMaxLinesVerticesCount, nullptr).Get();

		mpGeometryBuilder = CreateGeometryBuilder(result);

		if ((result != RC_OK) || 
			(result = _initGizmosBuffers()) != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CDebugUtility::_onFreeInternal()
	{
		return mpGeometryBuilder ? mpGeometryBuilder->Free() : RC_FAIL;
	}

	void CDebugUtility::PreRender()
	{
		if (!mLinesDataBuffer.empty())
		{
			mpLinesVertexBuffer->Map(BMT_WRITE_DISCARD);
			mpLinesVertexBuffer->Write(&mLinesDataBuffer[0], sizeof(TLineVertex) * mLinesDataBuffer.size());
			mpLinesVertexBuffer->Unmap();

			auto pDrawLinesCommand = mpRenderQueue->SubmitDrawCommand<TDrawCommand>(-2);

			pDrawLinesCommand->mpVertexBuffer           = mpLinesVertexBuffer;
			pDrawLinesCommand->mPrimitiveType           = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_LINE_LIST;
			pDrawLinesCommand->mMaterialHandle          = mpResourceManager->Load<IMaterial>(mDefaultDebugMaterialName);
			pDrawLinesCommand->mpVertexDeclaration      = mpLinesVertDeclaration;
			pDrawLinesCommand->mNumOfVertices           = static_cast<U32>(mLinesDataBuffer.size());
			pDrawLinesCommand->mObjectData.mModelMatrix = IdentityMatrix4;
		}

		/// \note draw crosses 
		if (!mCrossesDataBuffer.empty())
		{
			mpCrossesVertexBuffer->Map(BMT_WRITE_DISCARD);
			mpCrossesVertexBuffer->Write(&mCrossesDataBuffer[0], sizeof(TLineVertex) * mCrossesDataBuffer.size());
			mpCrossesVertexBuffer->Unmap();

			auto pDrawCrossesCommand = mpRenderQueue->SubmitDrawCommand<TDrawCommand>(-1);

			pDrawCrossesCommand->mpVertexBuffer           = mpCrossesVertexBuffer;
			pDrawCrossesCommand->mPrimitiveType           = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_LINE_LIST;
			pDrawCrossesCommand->mMaterialHandle          = mpResourceManager->Load<IMaterial>(mDefaultDebugMaterialName);
			pDrawCrossesCommand->mpVertexDeclaration      = mpLinesVertDeclaration;
			pDrawCrossesCommand->mNumOfVertices           = static_cast<U32>(mCrossesDataBuffer.size());
			pDrawCrossesCommand->mObjectData.mModelMatrix = IdentityMatrix4;
		}

		/// \note draw debug text onto the screen
		if (!mTextDataBuffer.empty())
		{
			mpTextVertexBuffer->Map(BMT_WRITE_DISCARD);
			mpTextVertexBuffer->Write(&mTextDataBuffer[0], sizeof(TTextVertex) * mTextDataBuffer.size());
			mpTextVertexBuffer->Unmap();

			auto pDrawTextCommand = mpRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(2);

			pDrawTextCommand->mpVertexBuffer           = mpTextVertexBuffer;
			pDrawTextCommand->mpIndexBuffer            = mpTextIndexBuffer;
			pDrawTextCommand->mPrimitiveType           = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
			pDrawTextCommand->mMaterialHandle          = mpResourceManager->Load<IMaterial>(mTextMaterialName);
			pDrawTextCommand->mpVertexDeclaration      = mpTextVertDeclaration;
			pDrawTextCommand->mStartIndex              = 0;
			pDrawTextCommand->mStartVertex             = 0;
			pDrawTextCommand->mNumOfIndices            = static_cast<U32>(mTextDataBuffer.size() * 1.5f); // \note 1.5 is hand-coded optimisation of 3 / 2 fracture
			pDrawTextCommand->mObjectData.mModelMatrix = IdentityMatrix4;
		}
		
		for (auto&& currGizmoInfo : mGizmosData)
		{
			auto pDrawGizmoCommand = mpRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(3);
			
			auto&& gizmoGeometryInfo = mGizmosTypesMap[currGizmoInfo.mType];

			pDrawGizmoCommand->mpVertexBuffer           = mpGizmosVertexBuffer;
			pDrawGizmoCommand->mpIndexBuffer            = mpGizmosIndexBuffer;
			pDrawGizmoCommand->mPrimitiveType           = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
			pDrawGizmoCommand->mMaterialHandle          = mpResourceManager->Load<IMaterial>(mDefaultDebugMaterialName);
			pDrawGizmoCommand->mpVertexDeclaration      = mpLinesVertDeclaration;
			pDrawGizmoCommand->mStartIndex              = gizmoGeometryInfo.mStartIndex;
			pDrawGizmoCommand->mStartVertex             = gizmoGeometryInfo.mStartVertex;
			pDrawGizmoCommand->mNumOfIndices            = gizmoGeometryInfo.mIndicesCount;
			pDrawGizmoCommand->mObjectData.mModelMatrix = Transpose(currGizmoInfo.mTransform);

			// \todo Add command for EDITOR_ONLY command buffer
		}		
	}

	void CDebugUtility::PostRender()
	{
		TDE2_PROFILER_SCOPE("CDebugUtility::PostRender");

		mLinesDataBuffer.clear();
		mTextDataBuffer.clear();
		mCrossesDataBuffer.clear();
		mGizmosData.clear();
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

	void CDebugUtility::DrawText(const TVector2& screenPos, F32 scale, const std::string& str, const TColor32F& color)
	{
		if (mSystemFontHandle == TResourceId::Invalid)
		{
			mSystemFontHandle = mpResourceManager->Load<IFont>("OpenSans.font");
		}

		auto pSystemFontResource = mpResourceManager->GetResource<IFont>(mSystemFontHandle);

		auto&& generatedMesh = pSystemFontResource->GenerateMesh({ { screenPos, MaxVector2 }, scale }, str).mVerts;

		std::transform(generatedMesh.begin(), generatedMesh.end(), std::back_inserter(mTextDataBuffer), [](const TVector4& v)
		{
			return TTextVertex { v };
		});
	}

	void CDebugUtility::DrawCross(const TVector3& position, F32 size, const TColor32F& color)
	{
		if (!mIsInitialized)
		{
			return;
		}

		auto createLine = [](std::vector<TLineVertex>& vertices, const TVector3& position, const TVector3& axis, F32 size, const TColor32F& color)
		{
			F32 halfSize = 0.5f * size;

			vertices.push_back({ { position - axis * halfSize, 1.0f }, color });
			vertices.push_back({ { position + axis * halfSize, 1.0f }, color });
		};

		createLine(mCrossesDataBuffer, position, RightVector3, size, color);
		createLine(mCrossesDataBuffer, position, UpVector3, size, color);
		createLine(mCrossesDataBuffer, position, ForwardVector3, size, color);
	}

	void CDebugUtility::DrawRect(const TRectF32& rect, const TColor32F& color)
	{
		if (!mIsInitialized)
		{
			return;
		}

		// \note Top line
		mLinesDataBuffer.push_back({ { rect.x, rect.y, 0.0f, 1.0f }, color });
		mLinesDataBuffer.push_back({ { rect.x + rect.width, rect.y, 0.0f, 1.0f }, color });

		// \note Right line
		mLinesDataBuffer.push_back({ { rect.x + rect.width, rect.y, 0.0f, 1.0f }, color });
		mLinesDataBuffer.push_back({ { rect.x + rect.width, rect.y + rect.height, 0.0f, 1.0f }, color });

		// \note Bottom line
		mLinesDataBuffer.push_back({ { rect.x + rect.width, rect.y + rect.height, 0.0f, 1.0f }, color });
		mLinesDataBuffer.push_back({ { rect.x, rect.y + rect.height, 0.0f, 1.0f }, color });

		// \note Left line
		mLinesDataBuffer.push_back({ { rect.x, rect.y + rect.height, 0.0f, 1.0f }, color });
		mLinesDataBuffer.push_back({ { rect.x, rect.y, 0.0f, 1.0f }, color });
	}

	void CDebugUtility::DrawAABB(const TAABB& aabb, const TColor32F& color)
	{
		if (!mIsInitialized)
		{
			return;
		}

		TVector3 min = aabb.min;
		TVector3 max = aabb.max;

		auto draw3DRect = [min, max](std::vector<TLineVertex>& vertices, F32 height, const TColor32F& c)
		{
			vertices.push_back({ { min.x, height, min.z, 1.0f }, c });
			vertices.push_back({ { min.x, height, max.z, 1.0f }, c });
			vertices.push_back({ { min.x, height, max.z, 1.0f }, c });
			vertices.push_back({ { max.x, height, max.z, 1.0f }, c });
			vertices.push_back({ { max.x, height, max.z, 1.0f }, c });
			vertices.push_back({ { max.x, height, min.z, 1.0f }, c });
			vertices.push_back({ { max.x, height, min.z, 1.0f }, c });
			vertices.push_back({ { min.x, height, min.z, 1.0f }, c });
		};
		
		draw3DRect(mLinesDataBuffer, min.y, color);
		draw3DRect(mLinesDataBuffer, max.y, color);

		mLinesDataBuffer.push_back({ { min, 1.0f }, color });
		mLinesDataBuffer.push_back({ { min.x, max.y, min.z, 1.0f }, color });
		mLinesDataBuffer.push_back({ { min.x, min.y, max.z, 1.0f }, color });
		mLinesDataBuffer.push_back({ { min.x, max.y, max.z, 1.0f }, color });
		mLinesDataBuffer.push_back({ { max.x, min.y, max.z, 1.0f }, color });
		mLinesDataBuffer.push_back({ { max, 1.0f }, color });		
		mLinesDataBuffer.push_back({ { max.x, min.y, min.z, 1.0f }, color });
		mLinesDataBuffer.push_back({ { max.x, max.y, min.z, 1.0f }, color });
	}

	void CDebugUtility::DrawCircle(const TVector3& position, const TVector3& planeNormal, F32 radius, const TColor32F& color, U16 segmentsCount)
	{
		if (!mIsInitialized)
		{
			return;
		}

		F32 deltaAngle = 2.0f * CMathConstants::Pi / segmentsCount;

		F32 currAngle = 0.0f;
		F32 nextAngle = 0.0f;

		// compute plane's tangent basis
		TVector3 u = Normalize(Cross(RightVector3, planeNormal));
		TVector3 v = Normalize(Cross(u, planeNormal));

		for (U16 k = 0; k < segmentsCount; ++k)
		{
			currAngle = nextAngle;
			nextAngle = (k + 1) * deltaAngle;

			mLinesDataBuffer.push_back({ { position + u * (radius * cosf(currAngle)) + v * (radius * sinf(currAngle)), 1.0f }, color });
			mLinesDataBuffer.push_back({ { position + u * (radius * cosf(nextAngle)) + v * (radius * sinf(nextAngle)), 1.0f }, color });
		}
	}

	void CDebugUtility::DrawSphere(const TVector3& position, F32 radius, const TColor32F& color, U16 triangulationFactor)
	{
		if (!mIsInitialized)
		{
			return;
		}

		auto drawTriangle = [](std::vector<TLineVertex>& vertices, const TVector3& p0, const TVector3& p1, const TVector3& p2, const TColor32F& color)
		{
			vertices.push_back({ { p0, 1.0f }, color });
			vertices.push_back({ { p1, 1.0f }, color });
			vertices.push_back({ { p0, 1.0f }, color });
			vertices.push_back({ { p2, 1.0f }, color });
			vertices.push_back({ { p1, 1.0f }, color });
			vertices.push_back({ { p2, 1.0f }, color });
		};
		
		const TVector3 basisVertices[]
		{
			TVector3(0.0f, radius, 0.0f),
			-TVector3(0.0f, radius, 0.0f),
			TVector3(radius, 0.0f, 0.0f),
			-TVector3(radius, 0.0f, 0.0f),
			TVector3(0.0f, 0.0f, radius),
			-TVector3(0.0f, 0.0f, radius),
		};

		std::vector<TVector3> vertices;

		std::function<void(std::vector<TVector3>&, const TVector3&, const TVector3&, const TVector3&, U16)> triangulateSphereSegment = 
		[&triangulateSphereSegment](auto&& vertices, const TVector3& v0, const TVector3& v1, const TVector3& v2, U16 depth)
		{
			if (!depth)
			{
				vertices.push_back(v0);
				vertices.push_back(v1);
				vertices.push_back(v2);

				return;
			}

			triangulateSphereSegment(vertices, v0, Lerp(v0, v1, 0.5f), Lerp(v0, v2, 0.5f), depth - 1);
			triangulateSphereSegment(vertices, v1, Lerp(v1, v0, 0.5f), Lerp(v1, v2, 0.5f), depth - 1);
			triangulateSphereSegment(vertices, v2, Lerp(v2, v0, 0.5f), Lerp(v2, v1, 0.5f), depth - 1);
			triangulateSphereSegment(vertices, Lerp(v0, v1, 0.5f), Lerp(v0, v2, 0.5f), Lerp(v1, v2, 0.5f), depth - 1);
		};

		U8 indices[][3]
		{
			{ 0, 2, 4 },
			{ 0, 2, 5 },
			{ 0, 3, 4 },
			{ 0, 3, 5 },
			{ 1, 2, 4 },
			{ 1, 2, 5 },
			{ 1, 3, 4 },
			{ 1, 3, 5 },
		};

		for (const auto& currIndices: indices)
		{
			triangulateSphereSegment(vertices, basisVertices[currIndices[0]], basisVertices[currIndices[1]], basisVertices[currIndices[2]], triangulationFactor - 1);
		}

		for (U16 i = 0; i < vertices.size(); i += 3)
		{
			auto&& v0 = vertices[i];
			auto&& v1 = vertices[i + 1];
			auto&& v2 = vertices[i + 2];

			v0 = Normalize(v0) * radius + position;
			v1 = Normalize(v1) * radius + position;
			v2 = Normalize(v2) * radius + position;

			drawTriangle(mLinesDataBuffer, v0, v1, v2, color);
		}
	}

	void CDebugUtility::DrawTransformGizmo(E_GIZMO_TYPE type, const TMatrix4& transform, F32 size)
	{
		mGizmosData.push_back({ type, transform, size });
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

	E_RESULT_CODE CDebugUtility::_initGizmosBuffers()
	{
		std::vector<TLineVertex> verts;
		std::vector<U16> indices;

		U32 startIndex   = 0;
		U32 vertexOffset = 0;

		std::function<IGeometryBuilder::TGeometryData(E_GIZMO_TYPE)> gizmoFactory = nullptr;

		for (U8 i = static_cast<U8>(E_GIZMO_TYPE::TRANSLATION); i != static_cast<U8>(E_GIZMO_TYPE::LAST); ++i)
		{
			startIndex   = static_cast<U32>(indices.size());
			vertexOffset = static_cast<U32>(verts.size());

			E_GIZMO_TYPE gizmoType = static_cast<E_GIZMO_TYPE>(i);

			switch (gizmoType)
			{
				case E_GIZMO_TYPE::TRANSLATION:
				case E_GIZMO_TYPE::TRANSLATION_X:
				case E_GIZMO_TYPE::TRANSLATION_Y:
				case E_GIZMO_TYPE::TRANSLATION_Z:
				case E_GIZMO_TYPE::TRANSLATION_XY:
				case E_GIZMO_TYPE::TRANSLATION_XZ:
				case E_GIZMO_TYPE::TRANSLATION_YZ:
					gizmoFactory = std::bind(&IGeometryBuilder::CreateTranslateGizmo, mpGeometryBuilder, std::placeholders::_1);
					break;
				case E_GIZMO_TYPE::ROTATION:
				case E_GIZMO_TYPE::ROTATION_X:
				case E_GIZMO_TYPE::ROTATION_Y:
				case E_GIZMO_TYPE::ROTATION_Z:
					gizmoFactory = std::bind(&IGeometryBuilder::CreateRotationGizmo, mpGeometryBuilder, std::placeholders::_1);
					break;
				case E_GIZMO_TYPE::SCALING:
				case E_GIZMO_TYPE::SCALING_X:
				case E_GIZMO_TYPE::SCALING_Y:
				case E_GIZMO_TYPE::SCALING_Z:
					gizmoFactory = std::bind(&IGeometryBuilder::CreateScaleGizmo, mpGeometryBuilder, std::placeholders::_1);
					break;
			}

			auto&& gizmoGeometry = gizmoFactory(gizmoType);

			std::transform(gizmoGeometry.mVertices.begin(), gizmoGeometry.mVertices.end(), std::back_inserter(verts), [](auto&& v)
			{
				return TLineVertex{ v.mPosition, v.mColor };
			});
			std::copy(gizmoGeometry.mIndices.begin(), gizmoGeometry.mIndices.end(), std::back_inserter(indices));

			mGizmosTypesMap[gizmoType] = { static_cast<U32>(gizmoGeometry.mIndices.size()), vertexOffset, startIndex };
		}
		
		auto gizmosVertexBufferResult = mpGraphicsObjectManager->CreateVertexBuffer(BUT_STATIC, sizeof(TLineVertex) * verts.size(), &verts[0]);
		if (gizmosVertexBufferResult.HasError())
		{
			return gizmosVertexBufferResult.GetError();
		}

		mpGizmosVertexBuffer = gizmosVertexBufferResult.Get();
		mpGizmosIndexBuffer  = mpGraphicsObjectManager->CreateIndexBuffer(BUT_STATIC, TDEngine2::IFT_INDEX16, sizeof(U16) * indices.size(), &indices[0]).Get();

		return RC_OK;
	}


	TDE2_API IDebugUtility* CreateDebugUtility(IResourceManager* pResourceManager, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IDebugUtility, CDebugUtility, result, pResourceManager, pRenderer, pGraphicsObjectManager);
	}
}