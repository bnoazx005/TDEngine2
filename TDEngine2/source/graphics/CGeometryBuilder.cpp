#include "../../include/graphics/CGeometryBuilder.h"
#include "../../include/math/MathUtils.h"
#include "../../include/graphics/IDebugUtility.h"
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

	CGeometryBuilder::TGeometryData CGeometryBuilder::CreateCubeGeometry(const TVector3& position, F32 size)
	{
		std::vector<TGeometryData::TVertex> vertices;

		F32 halfSize = 0.5f * size;

		const TVector4 origin { position, 0 };

		// clock-wise order is used, bottom face
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { halfSize, -halfSize, -halfSize, 1.0f }, { 1.0f, 1.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { -halfSize, -halfSize, -halfSize, 1.0f }, { 0.0f, 1.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { -halfSize, -halfSize, halfSize, 1.0f }, { 1.0f, 0.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { halfSize, -halfSize, halfSize, 1.0f }, { 0.0f, 0.0f, 0.0f } });

		// top face
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { halfSize, halfSize, -halfSize, 1.0f }, { 1.0f, 1.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { -halfSize, halfSize, -halfSize, 1.0f }, { 0.0f, 1.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { -halfSize, halfSize, halfSize, 1.0f }, { 1.0f, 0.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { halfSize, halfSize, halfSize, 1.0f }, { 0.0f, 0.0f, 0.0f } });

		// front face
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { halfSize, -halfSize, -halfSize, 1.0f }, { 1.0f, 1.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { -halfSize, -halfSize, -halfSize, 1.0f }, { 0.0f, 1.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { -halfSize, halfSize, -halfSize, 1.0f }, { 0.0f, 0.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { halfSize, halfSize, -halfSize, 1.0f }, { 1.0f, 0.0f, 0.0f } });

		// back face
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { halfSize, -halfSize, halfSize, 1.0f }, { 1.0f, 1.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { -halfSize, -halfSize, halfSize, 1.0f }, { 0.0f, 1.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { -halfSize, halfSize, halfSize, 1.0f }, { 0.0f, 0.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { halfSize, halfSize, halfSize, 1.0f }, { 1.0f, 0.0f, 0.0f } });

		// left face
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { -halfSize, halfSize, -halfSize, 1.0f }, { 0.0f, 0.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { -halfSize, -halfSize, -halfSize, 1.0f }, { 0.0f, 1.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { -halfSize, -halfSize, halfSize, 1.0f }, { 1.0f, 0.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { -halfSize, halfSize, halfSize, 1.0f }, { 1.0f, 1.0f, 0.0f } });

		// right face
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { halfSize, halfSize, -halfSize, 1.0f }, { 0.0f, 0.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { halfSize, -halfSize, -halfSize, 1.0f }, { 0.0f, 1.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { halfSize, -halfSize, halfSize, 1.0f }, { 1.0f, 0.0f, 0.0f } });
		vertices.push_back(TGeometryData::TVertex { origin + TVector4 { halfSize, halfSize, halfSize, 1.0f }, { 1.0f, 1.0f, 0.0f } });

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

		return CGeometryBuilder::TGeometryData { std::move(vertices), std::move(faces) };
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

		vertices.push_back(TGeometryData::TVertex { TVector4 { position, 1.0f }, TVector3(0.0f, 0.0f, 0.0f) });

		for (U16 k = 0; k < segmentsCount; ++k)
		{
			currAngle = k * deltaAngle;
			vertices.push_back(TGeometryData::TVertex { TVector4 { position + u * (radius * cosf(currAngle)) + v * (radius * sinf(currAngle)), 1.0f }, TVector3(0.0f, 0.0f, 0.0f) });
		}

		U16 baseVerticesCount = static_cast<U16>(vertices.size() - 1);

		TVector3 topPosition = position + height * normal;

		for (U16 k = 0; k < segmentsCount; ++k)
		{
			currAngle = k * deltaAngle;
			vertices.push_back(TGeometryData::TVertex{ TVector4 { topPosition + u * (radius * cosf(currAngle)) + v * (radius * sinf(currAngle)), 1.0f }, TVector3(0.0f, 0.0f, 0.0f) });
		}

		vertices.push_back(TGeometryData::TVertex{ TVector4 { position + height * normal, 1.0f }, TVector3(0.0f, 0.0f, 0.0f) });

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

		return CGeometryBuilder::TGeometryData { std::move(vertices), std::move(faces) };
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

		vertices.push_back(TGeometryData::TVertex { TVector4 { position, 1.0f }, TVector3(0.0f, 0.0f, 0.0f) });

		for (U16 k = 0; k < segmentsCount; ++k)
		{
			currAngle = k * deltaAngle;
			vertices.push_back(TGeometryData::TVertex { TVector4 { position + u * (radius * cosf(currAngle)) + v * (radius * sinf(currAngle)), 1.0f }, TVector3(0.0f, 0.0f, 0.0f) });
		}

		// \note Add cone's tip vertex
		vertices.push_back(TGeometryData::TVertex { TVector4 { position + height * normal, 1.0f }, TVector3(0.0f, 0.0f, 0.0f) });

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

		return CGeometryBuilder::TGeometryData { std::move(vertices), std::move(faces) };
	}

	CGeometryBuilder::TGeometryData CGeometryBuilder::CreatePlaneGeometry(const TVector3& position, const TVector3& normal, F32 width, F32 height, U16 segmentsPerSide)
	{
		std::vector<TGeometryData::TVertex> vertices;
		std::vector<U16> faces;

		// \note compute plane's basis
		TVector3 forward = Normalize(Cross(normal, Normalize(TVector3(-normal.y + 0.2f, normal.x, normal.z))));
		TVector3 right = Normalize(Cross(normal, forward));

		F32 halfWidth  = 0.5f * width;
		F32 halfHeight = 0.5f * height;

		F32 segmentWidth  = width / segmentsPerSide;
		F32 segmentHeight = height / segmentsPerSide;

		F32 y = halfHeight;
		
		for (U16 i = 0; i <= segmentsPerSide; ++i)
		{
			F32 x = -halfWidth;

			for (U16 j = 0; j <= segmentsPerSide; ++j)
			{
				vertices.push_back({ { position + x * right + y * forward, 1.0f }, TVector3((x + halfWidth) / width, CMathUtils::Abs(y - halfHeight) / height, 0.0f) });

				x += segmentWidth;
			}

			y -= segmentHeight;
		}

		const U16 stride = segmentsPerSide + 1;

		for (U16 i = 0; i < segmentsPerSide; ++i)
		{
			for (U16 j = 0; j < segmentsPerSide; ++j)
			{
				faces.push_back(i * stride + j);
				faces.push_back(i * stride + j + 1);
				faces.push_back((i + 1) * stride + j);

				faces.push_back((i + 1) * stride + j);
				faces.push_back(i * stride + j + 1);
				faces.push_back((i + 1) * stride + j + 1);
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
		TDE2_ASSERT(type == E_GIZMO_TYPE::ROTATION || type == E_GIZMO_TYPE::ROTATION_X || type == E_GIZMO_TYPE::ROTATION_Y || type == E_GIZMO_TYPE::ROTATION_Z);

		std::vector<TGeometryData::TVertex> verts;
		std::vector<U16> indices;

		std::tuple<TVector3, IGeometryBuilder::TGeometryData> rotationGizmoGeometry[]
		{
			{ RightVector3, CreateSolidArcGeometry(ZeroVector3, RightVector3, 1.0f, CMathConstants::Deg2Rad * 0.0f, CMathConstants::Deg2Rad * 90.0f, false, 0.5f, 6) },
			{ UpVector3, CreateSolidArcGeometry(ZeroVector3, UpVector3, 1.0f, CMathConstants::Deg2Rad * 180.0f, CMathConstants::Deg2Rad * 270.0f, false, 0.5f, 6) },
			{ ForwardVector3, CreateSolidArcGeometry(ZeroVector3, ForwardVector3, 1.0f, CMathConstants::Deg2Rad * 0.0f, CMathConstants::Deg2Rad * 90.0f, false, 0.5f, 6) }
		};

		const TColor32F selectedColor{ 1.0f, 0.647f, 0.0f, 1.0f };

		for (U8 i = 0; i < 3; ++i)
		{
			auto&& currAxisDirection = std::get<0>(rotationGizmoGeometry[i]);
			auto&& currGizmoPart     = std::get<1>(rotationGizmoGeometry[i]);
			
			TColor32F meshColor{ currAxisDirection.x, currAxisDirection.y, currAxisDirection.z, 1.0f };

			TColor32F currColor = (type == static_cast<E_GIZMO_TYPE>(static_cast<U8>(E_GIZMO_TYPE::ROTATION_X) + i)) ? selectedColor : meshColor;

			U16 offset = static_cast<U16>(verts.size());

			for (auto&& v : currGizmoPart.mVertices)
			{
				verts.push_back({ v.mPosition, ZeroVector3, currColor });
			}

			for (U16 currIndex : currGizmoPart.mIndices)
			{
				indices.push_back(currIndex + offset);
			}
		}

		return { std::move(verts), std::move(indices) };
	}

	CGeometryBuilder::TGeometryData CGeometryBuilder::CreateScaleGizmo(E_GIZMO_TYPE type)
	{
		TDE2_ASSERT(type == E_GIZMO_TYPE::SCALING || type == E_GIZMO_TYPE::SCALING_X || type == E_GIZMO_TYPE::SCALING_Y || type == E_GIZMO_TYPE::SCALING_Z);

		constexpr F32 axisRadius    = 0.02f;
		constexpr F32 axisTipRadius = 0.1f;

		std::tuple<TVector3, IGeometryBuilder::TGeometryData, IGeometryBuilder::TGeometryData> scaleGizmoGeometry[]
		{
			{ RightVector3,   CreateCylinderGeometry(ZeroVector3, RightVector3, axisRadius, 1.0f, 6),    CreateCubeGeometry(RightVector3, 0.2f) },
			{ UpVector3,      CreateCylinderGeometry(ZeroVector3, UpVector3, axisRadius, 1.0f, 6),       CreateCubeGeometry(UpVector3, 0.2f) },
			{ ForwardVector3, CreateCylinderGeometry(ZeroVector3, -ForwardVector3, axisRadius, 1.0f, 6), CreateCubeGeometry(-ForwardVector3, 0.2f) },
		};

		TVector3 currAxisDirection;

		IGeometryBuilder::TGeometryDataPtr pLineGeometry    = nullptr;
		IGeometryBuilder::TGeometryDataPtr pCubeTipGeometry = nullptr;

		std::vector<TGeometryData::TVertex> verts;
		std::vector<U16> indices;

		const TColor32F selectedColor{ 1.0f, 0.647f, 0.0f, 1.0f }; // \todo move to some common settings place

		for (U8 i = 0; i < 3; ++i)
		{
			auto&& currGizmoPart = scaleGizmoGeometry[i];

			currAxisDirection = std::move(std::get<TVector3>(currGizmoPart));
			pLineGeometry = &std::get<1>(currGizmoPart);
			pCubeTipGeometry = &std::get<2>(currGizmoPart);

			TColor32F meshColor{ currAxisDirection.x, currAxisDirection.y, currAxisDirection.z, 1.0f };

			TColor32F currColor = (type == static_cast<E_GIZMO_TYPE>(static_cast<U8>(E_GIZMO_TYPE::SCALING_X) + i)) ? selectedColor : meshColor;

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

			for (auto&& v : pCubeTipGeometry->mVertices)
			{
				verts.push_back({ v.mPosition, ZeroVector3, currColor });
			}

			for (U16 currIndex : pCubeTipGeometry->mIndices)
			{
				indices.push_back(currIndex + offset);
			}
		}

		return { std::move(verts), std::move(indices) };
	}

	CGeometryBuilder::TGeometryData CGeometryBuilder::CreateSolidArcGeometry(const TVector3& position, const TVector3& normal, F32 radius,
										 F32 fromAngle, F32 toAngle, bool isLooped, F32 width, U16 segmentsCount)
	{
		std::vector<TGeometryData::TVertex> verts;
		std::vector<U16> indices;

		F32 deltaAngle = (toAngle - fromAngle) / segmentsCount;

		const F32 halfWidth    = 0.5f * width;
		const F32 outterRadius = radius + halfWidth;
		const F32 innerRadius  = radius - halfWidth;

		F32 currAngle = fromAngle;
		F32 nextAngle = 0.0f;

		// compute plane's tangent basis
		TVector3 u = Normalize(Cross(TVector3(1.0f, 1.0f, 1.0f), normal));
		TVector3 v = Normalize(Cross(u, normal));

		U16 index = 0;

		for (U16 k = 0; k < segmentsCount; ++k)
		{
			currAngle = nextAngle;
			nextAngle = (k + 1) * deltaAngle;

			// \note add firsly outter vertex then inner one
			verts.push_back({ { position + u * (outterRadius * cosf(currAngle)) + v * (outterRadius * sinf(currAngle)), 1.0f }, ZeroVector3 });
			verts.push_back({ { position + u * (innerRadius * cosf(currAngle)) + v * (innerRadius * sinf(currAngle)), 1.0f }, ZeroVector3 });

			verts.push_back({ { position + u * (outterRadius * cosf(nextAngle)) + v * (outterRadius * sinf(nextAngle)), 1.0f }, ZeroVector3 });
			verts.push_back({ { position + u * (innerRadius * cosf(nextAngle)) + v * (innerRadius * sinf(nextAngle)), 1.0f }, ZeroVector3 });

			indices.push_back(index);
			indices.push_back(index + 2);
			indices.push_back(index + 1);

			indices.push_back(index + 2);
			indices.push_back(index + 3);
			indices.push_back(index + 1);

			index += 2;
		}

		return { std::move(verts), std::move(indices) };
	}


	TDE2_API IGeometryBuilder* CreateGeometryBuilder(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGeometryBuilder, CGeometryBuilder, result);
	}
}