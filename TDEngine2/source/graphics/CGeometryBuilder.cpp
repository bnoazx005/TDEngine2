#include "./../../include/graphics/CGeometryBuilder.h"
#include "./../../include/math/MathUtils.h"
#include "./../../include/graphics/IDebugUtility.h"
#include <cmath>


namespace TDEngine2
{
	CGeometryBuilder::CGeometryBuilder() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CGeometryBuilder::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CGeometryBuilder::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	CGeometryBuilder::TGeometryData CGeometryBuilder::CreateCubeGeometry(const TVector3& position, F32 size)
	{
		std::vector<TGeometryData::TVertex> vertices;

		// clock-wise order is used, bottom face
		vertices.push_back({ { 0.5f, -0.5f, -0.5f, 1.0f }, { 1.0f, 1.0f, 0.0f } });
		vertices.push_back({ { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f } });
		vertices.push_back({ { -0.5f, -0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f } });
		vertices.push_back({ { 0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f } });

		// top face
		vertices.push_back({ { 0.5f, 0.5f, -0.5f, 1.0f }, { 1.0f, 1.0f, 0.0f } });
		vertices.push_back({ { -0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f } });
		vertices.push_back({ { -0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f } });
		vertices.push_back({ { 0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f } });

		// front face
		vertices.push_back({ { 0.5f, -0.5f, -0.5f, 1.0f }, { 1.0f, 1.0f, 0.0f } });
		vertices.push_back({ { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f } });
		vertices.push_back({ { -0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f } });
		vertices.push_back({ { 0.5f, 0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f } });

		// back face
		vertices.push_back({ { 0.5f, -0.5f, 0.5f, 1.0f }, { 1.0f, 1.0f, 0.0f } });
		vertices.push_back({ { -0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f } });
		vertices.push_back({ { -0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f } });
		vertices.push_back({ { 0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f } });

		// left face
		vertices.push_back({ { -0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f } });
		vertices.push_back({ { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f } });
		vertices.push_back({ { -0.5f, -0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f } });
		vertices.push_back({ { -0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 1.0f, 0.0f } });

		// right face
		vertices.push_back({ { 0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f } });
		vertices.push_back({ { 0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f } });
		vertices.push_back({ { 0.5f, -0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f } });
		vertices.push_back({ { 0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 1.0f, 0.0f } });

		std::vector<U16> faces
		{
			0, 1, 2,
			0, 2, 3,
			4, 5, 6,
			4, 6, 7,
			8, 9, 10,
			8, 10, 11,
			12, 13, 14,
			12, 14, 15,
			16, 17, 18,
			16, 18, 19,
			20, 21, 22,
			20, 22, 23,
		};

		return { std::move(vertices), std::move(faces) };
	}
	
	CGeometryBuilder::TGeometryData CGeometryBuilder::CreateCylinderGeometry(const TVector3& position, const TVector3& axis, F32 radius, F32 height, U16 segmentsCount)
	{
		F32 deltaAngle = 2.0f * CMathConstants::Pi / segmentsCount;

		F32 currAngle = 0.0f;

		TVector3 normal = Normalize(axis);

		// compute plane's tangent basis
		TVector3 u = Normalize(Cross(std::abs(Dot(normal, RightVector3)) > 0.98f ? ForwardVector3 : RightVector3, normal));
		TVector3 v = Normalize(Cross(u, normal));

		std::vector<TGeometryData::TVertex> vertices;

		vertices.push_back({ { position, 1.0f }, TVector3(0.0f, 0.0f, 0.0f) });

		for (U16 k = 0; k < segmentsCount; ++k)
		{
			currAngle = k * deltaAngle;
			vertices.push_back({ { position + u * (radius * cosf(currAngle)) + v * (radius * sinf(currAngle)), 1.0f }, TVector3(0.0f, 0.0f, 0.0f) });
		}

		U16 baseVerticesCount = static_cast<U16>(vertices.size() - 1);

		TVector3 topPosition = position + height * normal;

		for (U16 k = 0; k < segmentsCount; ++k)
		{
			currAngle = k * deltaAngle;
			vertices.push_back({ { topPosition + u * (radius * cosf(currAngle)) + v * (radius * sinf(currAngle)), 1.0f }, TVector3(0.0f, 0.0f, 0.0f) });
		}

		vertices.push_back({ { position + height * normal, 1.0f }, TVector3(0.0f, 0.0f, 0.0f) });

		std::vector<U16> faces;

		// \note form base's tris
		for (U16 i = 1; i <= baseVerticesCount; ++i)
		{
			faces.push_back(0);
			faces.push_back(i);
			faces.push_back((i + 1) % (baseVerticesCount + 1) == 0 ? 1 : (i + 1));
		}

		U16 topSideOffset = baseVerticesCount + 1;

		for (U16 i = topSideOffset; i <= vertices.size() - 1; ++i)
		{
			faces.push_back(static_cast<U16>(vertices.size()) - 1);
			faces.push_back(i);
			faces.push_back((i + 1) % (vertices.size() - 1) == 0 ? topSideOffset : (i + 1));
		}

		// \note form side polygons
		for (U16 k = 1; k <= segmentsCount; ++k)
		{
			faces.push_back(k);
			faces.push_back((k + 1) % (baseVerticesCount + 1) == 0 ? 1 : (k + 1));
			faces.push_back((k + topSideOffset + 1) % (baseVerticesCount + 1) == 0 ? topSideOffset : (k + topSideOffset));
			
			faces.push_back(k);
			faces.push_back((k + topSideOffset + 1) % (baseVerticesCount + 1) == 0 ? topSideOffset : (k + topSideOffset));
			faces.push_back(k + topSideOffset - 1);
		}

		return { std::move(vertices), std::move(faces) };
	}

	CGeometryBuilder::TGeometryData CGeometryBuilder::CreateConeGeometry(const TVector3& position, const TVector3& axis, F32 radius, F32 height, U16 segmentsCount)
	{
		F32 deltaAngle = 2.0f * CMathConstants::Pi / segmentsCount;

		F32 currAngle = 0.0f;

		TVector3 normal = Normalize(axis);

		// compute plane's tangent basis
		TVector3 u = Normalize(Cross(std::abs(Dot(normal, RightVector3)) > 0.98f ? ForwardVector3 : RightVector3, normal));
		TVector3 v = Normalize(Cross(u, normal));

		std::vector<TGeometryData::TVertex> vertices;

		vertices.push_back({ { position, 1.0f }, TVector3(0.0f, 0.0f, 0.0f) });

		for (U16 k = 0; k < segmentsCount; ++k)
		{
			currAngle = k * deltaAngle;
			vertices.push_back({ { position + u * (radius * cosf(currAngle)) + v * (radius * sinf(currAngle)), 1.0f }, TVector3(0.0f, 0.0f, 0.0f) });
		}

		// \note Add cone's tip vertex
		vertices.push_back({ { position + height * normal, 1.0f }, TVector3(0.0f, 0.0f, 0.0f) });

		std::vector<U16> faces;

		// \note form base's tris
		U16 baseVerticesCount = static_cast<U16>(vertices.size() - 1);

		for (U16 i = 1; i < baseVerticesCount; ++i)
		{
			faces.push_back(0);
			faces.push_back(i);
			faces.push_back((i + 1) % baseVerticesCount == 0 ? 1 : (i + 1));
		}

		const U16 tipVertexId = static_cast<U16>(vertices.size() - 1);

		for (U16 i = 1; i <= segmentsCount; ++i)
		{
			faces.push_back((i + 1) % (segmentsCount + 1) == 0 ? 1 : (i + 1));
			faces.push_back(tipVertexId);
			faces.push_back(i);
		}

		return { std::move(vertices), std::move(faces) };
	}

	CGeometryBuilder::TGeometryData CGeometryBuilder::CreatePlaneGeometry(const TVector3& position, const TVector3& normal, F32 width, F32 height, U16 segmentsPerSide)
	{
		std::vector<TGeometryData::TVertex> vertices;
		std::vector<U16> faces;

		// \note compute plane's basis
		TVector3 randVec = TVector3(-normal.y, normal.x, normal.z);
		TVector3 forward = Normalize(Cross(normal, randVec));
		TVector3 right = Normalize(Cross(normal, forward));

		F32 halfWidth  = 0.5f * width;
		F32 halfHeight = 0.5f * height;

		F32 segmentWidth  = width / segmentsPerSide;
		F32 segmentHeight = height / segmentsPerSide;

		F32 y = halfHeight;
		
		for (U16 i = 0; i < segmentsPerSide; ++i)
		{
			F32 x = -halfWidth;

			for (U16 j = 0; j < segmentsPerSide; ++j)
			{
				vertices.push_back({ { position + x * right + y * forward, 1.0f }, TVector3(0.0f, 0.0f, 0.0f) });

				x += segmentWidth;
			}

			y -= segmentHeight;
		}

		for (U16 i = 0; i < segmentsPerSide - 1; ++i)
		{
			for (U16 j = 0; j < segmentsPerSide - 1; ++j)
			{
				faces.push_back(i * segmentsPerSide + j);
				faces.push_back(i * segmentsPerSide + j + 1);
				faces.push_back((i + 1) * segmentsPerSide + j);

				faces.push_back((i + 1) * segmentsPerSide + j);
				faces.push_back(i * segmentsPerSide + j + 1);
				faces.push_back((i + 1) * segmentsPerSide + j + 1);
			}
		}

		return { std::move(vertices), std::move(faces) };
	}

	CGeometryBuilder::TGeometryData CGeometryBuilder::CreateTranslateGizmo(E_GIZMO_TYPE type)
	{
		constexpr F32 axisRadius = 0.02f;
		constexpr F32 axisTipRadius = 0.1f;

		std::tuple<TVector3, IGeometryBuilder::TGeometryData, IGeometryBuilder::TGeometryData> translateGizmoGeometry[]
		{
			{ RightVector3,   CreateCylinderGeometry(ZeroVector3, RightVector3, axisRadius, 1.0f, 6),    CreateConeGeometry(RightVector3, RightVector3, axisTipRadius, 0.4f, 6) },
			{ UpVector3,      CreateCylinderGeometry(ZeroVector3, UpVector3, axisRadius, 1.0f, 6),       CreateConeGeometry(UpVector3, UpVector3, axisTipRadius, 0.4f, 6) },
			{ ForwardVector3, CreateCylinderGeometry(ZeroVector3, -ForwardVector3, axisRadius, 1.0f, 6), CreateConeGeometry(-ForwardVector3, -ForwardVector3, axisTipRadius, 0.4f, 6) },
		};

		TVector3 currAxisDirection;

		IGeometryBuilder::TGeometryDataPtr pLineGeometry    = nullptr;
		IGeometryBuilder::TGeometryDataPtr pConeTipGeometry = nullptr;

		std::vector<TGeometryData::TVertex> verts;
		std::vector<U16> indices;

		const TColor32F selectedColor { 1.0f, 0.647f, 0.0f, 1.0f };

		for (U8 i = 0; i < 3; ++i)
		{
			auto&& currGizmoPart = translateGizmoGeometry[i];

			currAxisDirection = std::move(std::get<TVector3>(currGizmoPart));
			pLineGeometry     = &std::get<1>(currGizmoPart);
			pConeTipGeometry  = &std::get<2>(currGizmoPart);

			TColor32F meshColor{ currAxisDirection.x, currAxisDirection.y, currAxisDirection.z, 1.0f };

			TColor32F currColor = (type == static_cast<E_GIZMO_TYPE>(static_cast<U8>(E_GIZMO_TYPE::TRANSLATION_X) + i)) ? selectedColor : meshColor;

			U16 offset = static_cast<U16>(verts.size());

			for (auto&& v : pLineGeometry->mVertices)
			{
				verts.push_back({ v.mPosition, ZeroVector3, currColor });
			}

			for (U16 currIndex : pLineGeometry->mIndices)
			{
				indices.push_back(currIndex + offset);
			}

			offset = static_cast<U16>(verts.size());

			for (auto&& v : pConeTipGeometry->mVertices)
			{
				verts.push_back({ v.mPosition, ZeroVector3, currColor });
			}

			for (U16 currIndex : pConeTipGeometry->mIndices)
			{
				indices.push_back(currIndex + offset);
			}
		}

		return { std::move(verts), std::move(indices) };
	}

	CGeometryBuilder::TGeometryData CGeometryBuilder::CreateRotationGizmo(E_GIZMO_TYPE type)
	{
		return { {}, {} };
	}

	CGeometryBuilder::TGeometryData CGeometryBuilder::CreateScaleGizmo(E_GIZMO_TYPE type)
	{
		return { {}, {} };
		//TDE2_ASSERT(type == E_GIZMO_TYPE::SCALING || type == E_GIZMO_TYPE::SCALING_X || type == E_GIZMO_TYPE::SCALING_Y || type == E_GIZMO_TYPE::SCALING_Z);

		//constexpr F32 axisRadius    = 0.02f;
		//constexpr F32 axisTipRadius = 0.1f;

		//std::tuple<TVector3, IGeometryBuilder::TGeometryData, IGeometryBuilder::TGeometryData> scaleGizmoGeometry[]
		//{
		//	{ RightVector3,   CreateCylinderGeometry(ZeroVector3, RightVector3, axisRadius, 1.0f, 6),    CreateCubeGeometry(RightVector3, RightVector3, axisTipRadius, 0.4f, 6) },
		//	{ UpVector3,      CreateCylinderGeometry(ZeroVector3, UpVector3, axisRadius, 1.0f, 6),       CreateCubeGeometry(UpVector3, UpVector3, axisTipRadius, 0.4f, 6) },
		//	{ ForwardVector3, CreateCylinderGeometry(ZeroVector3, -ForwardVector3, axisRadius, 1.0f, 6), CreateCubeGeometry(-ForwardVector3, -ForwardVector3, axisTipRadius, 0.4f, 6) },
		//};

		//TVector3 currAxisDirection;

		//IGeometryBuilder::TGeometryDataPtr pLineGeometry    = nullptr;
		//IGeometryBuilder::TGeometryDataPtr pCubeTipGeometry = nullptr;

		//std::vector<TGeometryData::TVertex> verts;
		//std::vector<U16> indices;

		//const TColor32F selectedColor{ 1.0f, 0.647f, 0.0f, 1.0f }; // \todo move to some common settings place

		//for (U8 i = 0; i < 3; ++i)
		//{
		//	auto&& currGizmoPart = scaleGizmoGeometry[i];

		//	currAxisDirection = std::move(std::get<TVector3>(currGizmoPart));
		//	pLineGeometry = &std::get<1>(currGizmoPart);
		//	pCubeTipGeometry = &std::get<2>(currGizmoPart);

		//	TColor32F meshColor{ currAxisDirection.x, currAxisDirection.y, currAxisDirection.z, 1.0f };

		//	TColor32F currColor = (type == static_cast<E_GIZMO_TYPE>(static_cast<U8>(E_GIZMO_TYPE::TRANSLATION_X) + i)) ? selectedColor : meshColor;

		//	U16 offset = static_cast<U16>(verts.size());

		//	for (auto&& v : pLineGeometry->mVertices)
		//	{
		//		verts.push_back({ v.mPosition, ZeroVector3, currColor });
		//	}

		//	for (U16 currIndex : pLineGeometry->mIndices)
		//	{
		//		indices.push_back(currIndex + offset);
		//	}

		//	offset = static_cast<U16>(verts.size());

		//	for (auto&& v : pCubeTipGeometry->mVertices)
		//	{
		//		verts.push_back({ v.mPosition, ZeroVector3, currColor });
		//	}

		//	for (U16 currIndex : pCubeTipGeometry->mIndices)
		//	{
		//		indices.push_back(currIndex + offset);
		//	}
		//}

		//return { std::move(verts), std::move(indices) };
	}


	TDE2_API IGeometryBuilder* CreateGeometryBuilder(E_RESULT_CODE& result)
	{
		CGeometryBuilder* pGeometryBuilder = new (std::nothrow) CGeometryBuilder();

		if (!pGeometryBuilder)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pGeometryBuilder->Init();

		if (result != RC_OK)
		{
			delete pGeometryBuilder;

			pGeometryBuilder = nullptr;
		}

		return pGeometryBuilder;
	}
}