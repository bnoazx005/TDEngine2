#include "./../../include/graphics/CGeometryBuilder.h"
#include "./../../include/math/MathUtils.h"


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

	CGeometryBuilder::TGeometryData CGeometryBuilder::CreateConeGeometry(const TVector3& position, const TVector3& axis, F32 radius, F32 height, U16 segmentsCount)
	{
		F32 deltaAngle = 2.0f * CMathConstants::Pi / segmentsCount;

		F32 currAngle = 0.0f;

		TVector3 normal = Normalize(axis);

		// compute plane's tangent basis
		TVector3 u = Normalize(Cross(RightVector3, normal));
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

		const U16 tipVertexId = vertices.size() - 1;

		for (U16 i = 1; i <= segmentsCount; ++i)
		{
			faces.push_back((i + 1) % (segmentsCount + 1) == 0 ? 1 : (i + 1));
			faces.push_back(tipVertexId);
			faces.push_back(i);
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