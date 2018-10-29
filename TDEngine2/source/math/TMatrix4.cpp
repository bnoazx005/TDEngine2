#include "./../../include/math/TMatrix4.h"
#include "./../../include/math/TMatrix3.h"
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

		F32 det = Det(mat4);

		if (fabs(det) < FloatEpsilon)
		{
			return inversed;
		}

		F32 invDet = 1.0f / det;

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
		F32 minorDet00 = Det(TMatrix3(mat4.m[1][1], mat4.m[1][2], mat4.m[1][3], mat4.m[2][1], mat4.m[2][2], mat4.m[2][3], mat4.m[3][1], mat4.m[3][2], mat4.m[3][3]));
		F32 minorDet01 = Det(TMatrix3(mat4.m[1][0], mat4.m[1][2], mat4.m[1][3], mat4.m[2][0], mat4.m[2][2], mat4.m[2][3], mat4.m[3][0], mat4.m[3][2], mat4.m[3][3]));
		F32 minorDet02 = Det(TMatrix3(mat4.m[1][0], mat4.m[1][1], mat4.m[1][3], mat4.m[2][0], mat4.m[2][1], mat4.m[2][3], mat4.m[3][0], mat4.m[3][1], mat4.m[3][3]));
		F32 minorDet03 = Det(TMatrix3(mat4.m[1][0], mat4.m[1][1], mat4.m[1][2], mat4.m[2][0], mat4.m[2][1], mat4.m[2][2], mat4.m[3][0], mat4.m[3][1], mat4.m[3][2]));

		return mat4.m[0][0] * minorDet00 - mat4.m[0][1] * minorDet01 + mat4.m[0][2] * minorDet02 - mat4.m[0][2] * minorDet03;
	}
}