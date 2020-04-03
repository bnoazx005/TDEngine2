#include "./../../include/graphics/CGeometryBuilder.h"
#include "./../../include/math/MathUtils.h"
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

		U16 baseVerticesCount = vertices.size() - 1;

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
			faces.push_back(vertices.size() - 1);
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
		U16 baseVerticesCount = vertices.size() - 1;

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
				vertices.push_back({ { position + TVector3(x, 0.0f, y), 1.0f }, TVector3(0.0f, 0.0f, 0.0f) });

				x += segmentWidth;
			}

			y += segmentHeight;
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