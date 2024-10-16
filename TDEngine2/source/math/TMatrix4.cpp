#include "../../include/math/TMatrix4.h"
#include "../../include/math/TMatrix3.h"
#include "../../include/core/Serialization.h"
#include "../../include/math/MathUtils.h"
#include "stringUtils.hpp"
#include <cstring>
#include <cmath>


namespace TDEngine2
{
	TMatrix4::TMatrix4()
	{
		memset(&m, 0, sizeof(m));
	}

	TMatrix4::TMatrix4(const F32 arr[16])
	{
		if (arr)
		{
			for (I32 i = 0; i < 4; ++i)
			{
				for (I32 j = 0; j < 4; ++j)
				{
					m[i][j] = arr[i * 4 + j];
				}
			}
		}
	}

	TMatrix4::TMatrix4(F32 _11, F32 _12, F32 _13, F32 _14,
					   F32 _21, F32 _22, F32 _23, F32 _24,
					   F32 _31, F32 _32, F32 _33, F32 _34,
					   F32 _41, F32 _42, F32 _43, F32 _44)
	{
		m[0][0] = _11; m[0][1] = _12; m[0][2] = _13; m[0][3] = _14;
		m[1][0] = _21; m[1][1] = _22; m[1][2] = _23; m[1][3] = _24;
		m[2][0] = _31; m[2][1] = _32; m[2][2] = _33; m[2][3] = _34;
		m[3][0] = _41; m[3][1] = _42; m[3][2] = _43; m[3][3] = _44;
	}

	TMatrix4::TMatrix4(const TVector4& diagElements)
	{
		memset(&m, 0, sizeof(m));

		m[0][0] = diagElements.x;
		m[1][1] = diagElements.y;
		m[2][2] = diagElements.z;
		m[3][3] = diagElements.w;
	}

	TMatrix4::TMatrix4(const TMatrix4& mat)
	{
		for (I32 i = 0; i < 4; ++i)
		{
			for (I32 j = 0; j < 4; ++j)
			{
				m[i][j] = mat.m[i][j];
			}
		}
	}

	TMatrix4::TMatrix4(TMatrix4&& mat)
	{
		for (I32 i = 0; i < 4; ++i)
		{
			for (I32 j = 0; j < 4; ++j)
			{
				m[i][j] = mat.m[i][j];

				mat.m[i][j] = 0.0f;
			}
		}
	}

	TMatrix4 TMatrix4::operator= (const TMatrix4& mat)
	{
		for (I32 i = 0; i < 4; ++i)
		{
			for (I32 j = 0; j < 4; ++j)
			{
				m[i][j] = mat.m[i][j];
			}
		}

		return *this;
	}

	TMatrix4& TMatrix4::operator= (TMatrix4&& mat)
	{
		for (I32 i = 0; i < 4; ++i)
		{
			for (I32 j = 0; j < 4; ++j)
			{
				m[i][j] = mat.m[i][j];

				mat.m[i][j] = 0.0f;
			}
		}

		return *this;
	}

	std::string TMatrix4::ToString() const
	{
		static const std::string header = "TMatrix4\n(";

		std::string currStr;

		for (I32 i = 0; i < 4; ++i)
		{
			for (I32 j = 0; j < 4; ++j)
			{
				currStr.append(std::to_string(m[i][j])).append((i == 3 && j == 3) ? Wrench::StringUtils::GetEmptyStr() : ", ");
			}

			currStr.append(i < 3 ? "\n" : ")");
		}

		return header + currStr;
	}


	TMatrix4 operator+ (const TMatrix4& lmat4, const TMatrix4& rmat4)
	{
		TMatrix4 result;

		for (I32 i = 0; i < 4; ++i)
		{
			for (I32 j = 0; j < 4; ++j)
			{
				result.m[i][j] = lmat4.m[i][j] + rmat4.m[i][j];
			}
		}

		return result;
	}

	TMatrix4 operator- (const TMatrix4& lmat4, const TMatrix4& rmat4)
	{
		TMatrix4 result;

		for (I32 i = 0; i < 4; ++i)
		{
			for (I32 j = 0; j < 4; ++j)
			{
				result.m[i][j] = lmat4.m[i][j] - rmat4.m[i][j];
			}
		}

		return result;
	}

	TMatrix4 operator* (const TMatrix4& lmat4, const TMatrix4& rmat4)
	{
		TMatrix4 result;

		for (I32 i = 0; i < 4; ++i)
		{
			for (I32 j = 0; j < 4; ++j)
			{
				for (I32 k = 0; k < 4; ++k)
				{
					result.m[i][j] += lmat4.m[i][k] * rmat4.m[k][j];
				}
			}
		}

		return result;
	}

	TVector4 operator* (const TMatrix4& mat4, const TVector4& vec4)
	{
		F32 result[4];

		for (I32 i = 0; i < 4; ++i)
		{
			result[i] = mat4.m[i][0] * vec4.x + mat4.m[i][1] * vec4.y + mat4.m[i][2] * vec4.z + mat4.m[i][3] * vec4.w;
		}

		return TVector4(result);
	}

	TVector3 operator* (const TMatrix4& mat4, const TVector3& vec3)
	{
		F32 result[4];

		for (I32 i = 0; i < 4; ++i)
		{
			result[i] = mat4.m[i][0] * vec3.x + mat4.m[i][1] * vec3.y + mat4.m[i][2] * vec3.z + mat4.m[i][3];
		}

		return TVector3(result);
	}

	TVector2 operator* (const TMatrix4& mat4, const TVector2& vec2)
	{
		F32 result[4];

		for (I32 i = 0; i < 4; ++i)
		{
			result[i] = mat4.m[i][0] * vec2.x + mat4.m[i][1] * vec2.y + mat4.m[i][3];
		}

		return TVector2(result[0], result[1]);
	}

	TMatrix4 operator* (const TMatrix4& mat4, const F32& coeff)
	{
		TMatrix4 result;

		for (I32 i = 0; i < 4; ++i)
		{
			for (I32 j = 0; j < 4; ++j)
			{
				result.m[i][j] = mat4.m[i][j] * coeff;
			}
		}

		return result;
	}

	TMatrix4 operator* (const F32& coeff, const TMatrix4& mat4)
	{
		TMatrix4 result;

		for (I32 i = 0; i < 4; ++i)
		{
			for (I32 j = 0; j < 4; ++j)
			{
				result.m[i][j] = mat4.m[i][j] * coeff;
			}
		}

		return result;
	}


	TDE2_API bool operator== (const TMatrix4& lmat4, const TMatrix4& rmat4)
	{
		for (I32 i = 0; i < 4; ++i)
		{
			for (I32 j = 0; j < 4; ++j)
			{
				if (CMathUtils::Abs(lmat4.m[i][j] - rmat4.m[i][j]) > 1e-3f) 
				{
					return false;
				}
			}
		}

		return true;
	}

	TDE2_API bool operator!= (const TMatrix4& lmat4, const TMatrix4& rmat4)
	{
		for (I32 i = 0; i < 4; ++i)
		{
			for (I32 j = 0; j < 4; ++j)
			{
				if (CMathUtils::Abs(lmat4.m[i][j] - rmat4.m[i][j]) > 1e-3f)
				{
					return true;
				}
			}
		}

		return false;
	}

	TDE2_API TMatrix4 Mul(const TMatrix4& lmat4, const TMatrix4& rmat4)
	{
		return lmat4 * rmat4;
	}

	TDE2_API TVector4 Mul(const TMatrix4& mat4, const TVector4& vec4)
	{
		return mat4 * vec4;
	}

	TDE2_API TMatrix4 Inverse(const TMatrix4& mat4)
	{
		TMatrix4 inversed = ZeroMatrix4;

		const F32 det = Det(mat4);

		if (CMathUtils::Abs(det) < 0.0f)
		{
			return inversed;
		}

		const F32 invDet = 1.0f / det;

		F32 minorDet11 = Det(TMatrix3(mat4.m[1][1], mat4.m[1][2], mat4.m[1][3], mat4.m[2][1], mat4.m[2][2], mat4.m[2][3], mat4.m[3][1], mat4.m[3][2], mat4.m[3][3]));
		F32 minorDet12 = Det(TMatrix3(mat4.m[1][0], mat4.m[1][2], mat4.m[1][3], mat4.m[2][0], mat4.m[2][2], mat4.m[2][3], mat4.m[3][0], mat4.m[3][2], mat4.m[3][3]));
		F32 minorDet13 = Det(TMatrix3(mat4.m[1][0], mat4.m[1][1], mat4.m[1][3], mat4.m[2][0], mat4.m[2][1], mat4.m[2][3], mat4.m[3][0], mat4.m[3][1], mat4.m[3][3]));
		F32 minorDet14 = Det(TMatrix3(mat4.m[1][0], mat4.m[1][1], mat4.m[1][2], mat4.m[2][0], mat4.m[2][1], mat4.m[2][2], mat4.m[3][0], mat4.m[3][1], mat4.m[3][2]));

		F32 minorDet21 = Det(TMatrix3(mat4.m[0][1], mat4.m[0][2], mat4.m[0][3], mat4.m[2][1], mat4.m[2][2], mat4.m[2][3], mat4.m[3][1], mat4.m[3][2], mat4.m[3][3]));
		F32 minorDet22 = Det(TMatrix3(mat4.m[0][0], mat4.m[0][2], mat4.m[0][3], mat4.m[2][0], mat4.m[2][2], mat4.m[2][3], mat4.m[3][0], mat4.m[3][2], mat4.m[3][3]));
		F32 minorDet23 = Det(TMatrix3(mat4.m[0][0], mat4.m[0][1], mat4.m[0][3], mat4.m[2][0], mat4.m[2][1], mat4.m[2][3], mat4.m[3][0], mat4.m[3][1], mat4.m[3][3]));
		F32 minorDet24 = Det(TMatrix3(mat4.m[0][0], mat4.m[0][1], mat4.m[0][2], mat4.m[2][0], mat4.m[2][1], mat4.m[2][2], mat4.m[3][0], mat4.m[3][1], mat4.m[3][2]));

		F32 minorDet31 = Det(TMatrix3(mat4.m[0][1], mat4.m[0][2], mat4.m[0][3], mat4.m[1][1], mat4.m[1][2], mat4.m[1][3], mat4.m[3][1], mat4.m[3][2], mat4.m[3][3]));
		F32 minorDet32 = Det(TMatrix3(mat4.m[0][0], mat4.m[0][2], mat4.m[0][3], mat4.m[1][0], mat4.m[1][2], mat4.m[1][3], mat4.m[3][0], mat4.m[3][2], mat4.m[3][3]));
		F32 minorDet33 = Det(TMatrix3(mat4.m[0][0], mat4.m[0][1], mat4.m[0][3], mat4.m[1][0], mat4.m[1][1], mat4.m[1][3], mat4.m[3][0], mat4.m[3][1], mat4.m[3][3]));
		F32 minorDet34 = Det(TMatrix3(mat4.m[0][0], mat4.m[0][1], mat4.m[0][2], mat4.m[1][0], mat4.m[1][1], mat4.m[1][2], mat4.m[3][0], mat4.m[3][1], mat4.m[3][2]));

		F32 minorDet41 = Det(TMatrix3(mat4.m[0][1], mat4.m[0][2], mat4.m[0][3], mat4.m[1][1], mat4.m[1][2], mat4.m[1][3], mat4.m[2][1], mat4.m[2][2], mat4.m[2][3]));
		F32 minorDet42 = Det(TMatrix3(mat4.m[0][0], mat4.m[0][2], mat4.m[0][3], mat4.m[1][0], mat4.m[1][2], mat4.m[1][3], mat4.m[2][0], mat4.m[2][2], mat4.m[2][3]));
		F32 minorDet43 = Det(TMatrix3(mat4.m[0][0], mat4.m[0][1], mat4.m[0][3], mat4.m[1][0], mat4.m[1][1], mat4.m[1][3], mat4.m[2][0], mat4.m[2][1], mat4.m[2][3]));
		F32 minorDet44 = Det(TMatrix3(mat4.m[0][0], mat4.m[0][1], mat4.m[0][2], mat4.m[1][0], mat4.m[1][1], mat4.m[1][2], mat4.m[2][0], mat4.m[2][1], mat4.m[2][2]));


		//compute adjoint matrix adjA = (A*)^T, where A* is composed from minors of A
		inversed.m[0][0] = invDet * minorDet11;
		inversed.m[0][1] = -invDet * minorDet21;
		inversed.m[0][2] = invDet * minorDet31;
		inversed.m[0][3] = -invDet * minorDet41;

		inversed.m[1][0] = -invDet * minorDet12;
		inversed.m[1][1] = invDet * minorDet22;
		inversed.m[1][2] = -invDet * minorDet32;
		inversed.m[1][3] = invDet * minorDet42;

		inversed.m[2][0] = invDet * minorDet13;
		inversed.m[2][1] = -invDet * minorDet23;
		inversed.m[2][2] = invDet * minorDet33;
		inversed.m[2][3] = -invDet * minorDet43;

		inversed.m[3][0] = -invDet * minorDet14;
		inversed.m[3][1] = invDet * minorDet24;
		inversed.m[3][2] = -invDet * minorDet34;
		inversed.m[3][3] = invDet * minorDet44;

		return inversed;
	}

	TDE2_API TMatrix4 Transpose(const TMatrix4& mat4)
	{
		TMatrix4 result;

		for (I32 i = 0; i < 4; i++)
		{
			for (I32 j = 0; j < 4; j++)
			{
				result.m[i][j] = mat4.m[j][i];
			}
		}

		return result;
	}

	TDE2_API F32 Det(const TMatrix4& mat4)
	{
		const F32 minorDet00 = Det(TMatrix3(mat4.m[1][1], mat4.m[1][2], mat4.m[1][3], mat4.m[2][1], mat4.m[2][2], mat4.m[2][3], mat4.m[3][1], mat4.m[3][2], mat4.m[3][3]));
		const F32 minorDet01 = Det(TMatrix3(mat4.m[1][0], mat4.m[1][2], mat4.m[1][3], mat4.m[2][0], mat4.m[2][2], mat4.m[2][3], mat4.m[3][0], mat4.m[3][2], mat4.m[3][3]));
		const F32 minorDet02 = Det(TMatrix3(mat4.m[1][0], mat4.m[1][1], mat4.m[1][3], mat4.m[2][0], mat4.m[2][1], mat4.m[2][3], mat4.m[3][0], mat4.m[3][1], mat4.m[3][3]));
		const F32 minorDet03 = Det(TMatrix3(mat4.m[1][0], mat4.m[1][1], mat4.m[1][2], mat4.m[2][0], mat4.m[2][1], mat4.m[2][2], mat4.m[3][0], mat4.m[3][1], mat4.m[3][2]));

		return mat4.m[0][0] * minorDet00 - mat4.m[0][1] * minorDet01 + mat4.m[0][2] * minorDet02 - mat4.m[0][3] * minorDet03;
	}

	TDE2_API TMatrix4 TranslationMatrix(const TVector3& t)
	{
		TMatrix4 translationMat(IdentityMatrix4);

		translationMat.m[0][3] = t.x;
		translationMat.m[1][3] = t.y;
		translationMat.m[2][3] = t.z;

		return translationMat;
	}

	TDE2_API TMatrix4 ScaleMatrix(const TVector3& s)
	{
		return TMatrix4(TVector4(s, 1.0f));
	}

	TDE2_API TMatrix4 PerspectiveProj(F32 fov, F32 aspect, F32 zn, F32 zf, F32 zNDCMin, F32 zNDCMax, F32 handedness)
	{
		TMatrix4 projMatrix;

		F32 d = 1.0f / tanf(fov * 0.5f);

		F32 dz = zn - zf;

		projMatrix.m[0][0] = d / aspect;
		projMatrix.m[1][1] = d;
		projMatrix.m[2][2] = handedness * (zNDCMax * zf - zNDCMin * zn) / dz;
		projMatrix.m[2][3] = zn * zf * (zNDCMax - zNDCMin) / dz;

		projMatrix.m[3][2] = -handedness;

		return projMatrix;
	}

	TDE2_API TMatrix4 OrthographicProj(F32 left, F32 top, F32 right, F32 bottom, F32 zn, F32 zf, F32 zNDCMin, F32 zNDCMax, F32 handedness, bool isDepthless)
	{
		TMatrix4 projMatrix;

		F32 width  = right - left;
		F32 height = top - bottom;
		F32 depth  = zf - zn;

		projMatrix.m[0][0] = 2.0f / width;
		projMatrix.m[1][1] = 2.0f / height;
		projMatrix.m[2][2] = isDepthless ? 0.5f * (zf + zn) : -handedness * fabs(zNDCMax - zNDCMin) / depth;

		projMatrix.m[0][3] = -(right + left) / width;
		projMatrix.m[1][3] = -(top + bottom) / height;
		projMatrix.m[2][3] = isDepthless ? 0.0f : -(zn - zf * zNDCMin) / depth;

		projMatrix.m[3][3] = 1.0f;

		return projMatrix;
	}

	TDE2_API TMatrix4 LookAt(const TVector3& eye, const TVector3& up, const TVector3& target, F32 handedness)
	{
		TMatrix4 mat4 = IdentityMatrix4;

		const TVector3 dir   = Normalize(target - eye);
		const TVector3 right = Normalize(up * dir);
		const TVector3 upVec = dir * right;

		mat4.m[0][0] = right.x;
		mat4.m[1][0] = upVec.x;
		mat4.m[2][0] = dir.x;

		mat4.m[0][1] = right.y;
		mat4.m[1][1] = upVec.y;
		mat4.m[2][1] = dir.y;

		mat4.m[0][2] = right.z;
		mat4.m[1][2] = upVec.z;
		mat4.m[2][2] = dir.z;

		mat4.m[0][3] = handedness * Dot(right, eye);
		mat4.m[1][3] = handedness * Dot(upVec, eye);
		mat4.m[2][3] = handedness * Dot(dir, eye);

		return mat4;
	}


	static const std::array<std::string, 16> MatrixSerializationKeys
	{
		"_11", "_12", "_13", "_14",
		"_21", "_22", "_23", "_24",
		"_31", "_32", "_33", "_34",
		"_41", "_42", "_43", "_44",
	};


	TDE2_API TResult<TMatrix4> LoadMatrix4(class IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		TMatrix4 mat;

		U32 currIndex = 0;

		while (pReader->HasNextItem())
		{
			pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
			{
				for (U8 i = 0; i < 4; ++i, ++currIndex)
				{
					mat.arr[currIndex] = pReader->GetFloat(MatrixSerializationKeys[currIndex]);
				}
			}
			pReader->EndGroup();
		}

		return Wrench::TOkValue<TMatrix4>(mat);
	}

	TDE2_API E_RESULT_CODE SaveMatrix4(class IArchiveWriter* pWriter, const TMatrix4& object)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		for (U8 i = 0; i < 4; ++i)
		{
			pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
			{
				for (U8 j = 0; j < 4; ++j)
				{
					const U32 index = i * 4 + j;

					pWriter->SetFloat(MatrixSerializationKeys[index], object.arr[index]);
				}
			}
			pWriter->EndGroup();
		}

		return RC_OK;
	}
}