#include "../../include/math/TVector2.h"
#include "../../include/math/MathUtils.h"
#include <cmath>


namespace TDEngine2
{
	F32 Dot(const TVector2& lvec2, const TVector2& rvec2)
	{
		return lvec2.x * rvec2.x + lvec2.y * rvec2.y;
	}

	TVector2 Normalize(const TVector2& vec2)
	{
		F32 x = vec2.x;
		F32 y = vec2.y;

		const F32 sqrLength = x * x + y * y;

		if (sqrLength < 1e-3f)
		{
			return ZeroVector2;
		}

		F32 invLength = 1.0f / sqrtf(sqrLength);

		return TVector2(x * invLength, y * invLength);
	}

	F32 Length(const TVector2& vec2)
	{
		F32 x = vec2.x;
		F32 y = vec2.y;

		return sqrtf(x * x + y * y);
	}

	TVector2 Negative(const TVector2& vec2)
	{
		return TVector2(-vec2.x, -vec2.y);
	}

	TVector2 Scale(const TVector2& vec2, const F32& coeff)
	{
		return TVector2(coeff * vec2.x, coeff * vec2.y);
	}

	TVector2 Scale(const TVector2& vec2l, const TVector2& vec2r)
	{
		return TVector2(vec2l.x * vec2r.x, vec2l.y * vec2r.y);
	}

	TVector2 Reflect(const TVector2& vec, const TVector2& normal)
	{
		return 2.0f * Dot(vec, normal) * Normalize(normal) - vec;
	}

	TVector2 SnapToGrid(const TVector2& vec, const TVector2& gridCellSizes)
	{
		return TVector2
		{
			gridCellSizes.x * roundf(vec.x / CMathUtils::Max(FloatEpsilon, gridCellSizes.x)),
			gridCellSizes.y * roundf(vec.y / CMathUtils::Max(FloatEpsilon, gridCellSizes.y))
		};
	}


	TResult<TVector2> LoadVector2(IArchiveReader* pReader)
	{
		constexpr I32 MissingPropertyTag = 0xcdcdcdcd;

		if (!pReader)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		// \todo Replace with efficient check up later
		if (MissingPropertyTag == pReader->GetInt32("x", MissingPropertyTag) && MissingPropertyTag == pReader->GetInt32("y", MissingPropertyTag))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		TVector2 vec;

		vec.x = pReader->GetFloat("x");
		vec.y = pReader->GetFloat("y");

		return Wrench::TOkValue<TVector2>(vec);
	}

	E_RESULT_CODE SaveVector2(IArchiveWriter* pWriter, const TVector2& object)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		bool anyWritten = false;

		if (std::fabs(object.y) > 1e-3f)
		{
			pWriter->SetFloat("y", object.y); 
			anyWritten = true;
		}

		if (!anyWritten || (anyWritten && std::fabs(object.x) > 1e-3f))
		{
			pWriter->SetFloat("x", object.x); /// \note Always write at least x component to prevent the bug of serialization in the Yaml library
		}

		return RC_OK;
	}


	TDE2_API E_RESULT_CODE Serialize(IArchiveWriter* pWriter, const TVector2& value)
	{
		return pWriter->SetUInt32("type_id", static_cast<U32>(GetTypeId<TVector2>::mValue)) | SaveVector2(pWriter, value);
	}

	TDE2_API E_RESULT_CODE Serialize(IArchiveWriter* pWriter, const std::string& name, const TVector2& value)
	{
		E_RESULT_CODE result = pWriter->BeginGroup(name, false);
		result = result | SaveVector2(pWriter, value);
		result = result | pWriter->EndGroup();

		return result;
	}

	template <> TDE2_API TResult<TVector2> Deserialize<TVector2>(IArchiveReader* pReader)
	{
		return LoadVector2(pReader);
	}

	TDE2_API TResult<TVector2> TDeserializer<TVector2>::Deserialize(IArchiveReader* pReader, const std::string& name)
	{
		pReader->BeginGroup(name);
		auto result = LoadVector2(pReader);
		pReader->EndGroup();

		return result;
	}
}