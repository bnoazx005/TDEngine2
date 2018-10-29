#include "./../../include/math/TMatrix3.h"
#include <cstring>
#include <cmath>


namespace TDEngine2
{
	TMatrix3::TMatrix3()
	{
		memset(&m, 0, sizeof(m));
	}

	TMatrix3::TMatrix3(const F32 arr[9])
	{
		if (arr)
		{
			for (I32 i = 0; i < 3; ++i)
			{
				for (I32 j = 0; j < 3; ++j)
				{
					m[i][j] = arr[i * 3 + j];
				}
			}
		}
	}

	TMatrix3::TMatrix3(F32 m00, F32 m01, F32 m02, F32 m10, F32 m11, F32 m12, F32 m20, F32 m21, F32 m22)
	{
		m[0][0] = m00;
		m[0][1] = m01;
		m[0][2] = m02;

		m[1][0] = m10;
		m[1][1] = m11;
		m[1][2] = m12;

		m[2][0] = m20;
		m[2][1] = m21;
		m[2][2] = m22;
	}

	TMatrix3::TMatrix3(const TVector3& diagElements)
	{
		memset(&m, 0, sizeof(m));

		m[0][0] = diagElements.x;
		m[1][1] = diagElements.y;
		m[2][2] = diagElements.z;
	}

	TMatrix3::TMatrix3(const TMatrix3& mat)
	{
		for (I32 i = 0; i < 3; ++i)
		{
			for (I32 j = 0; j < 3; ++j)
			{
				m[i][j] = mat.m[i][j];
			}
		}
	}

	TMatrix3::TMatrix3(TMatrix3&& mat)
	{
		for (I32 i = 0; i < 3; ++i)
		{
			for (I32 j = 0; j < 3; ++j)
			{
				m[i][j] = mat.m[i][j];

				mat.m[i][j] = 0.0f;
			}
		}
	}

	TMatrix3 TMatrix3::operator= (const TMatrix3& mat)
	{
		for (I32 i = 0; i < 3; ++i)
		{
			for (I32 j = 0; j < 3; ++j)
			{
				m[i][j] = mat.m[i][j];
			}
		}

		return *this;
	}

	TMatrix3& TMatrix3::operator= (TMatrix3&& mat)
	{
		for (I32 i = 0; i < 3; ++i)
		{
			for (I32 j = 0; j < 3; ++j)
			{
				m[i][j] = mat.m[i][j];

				mat.m[i][j] = 0.0f;
			}
		}

		return *this;
	}


	TMatrix3 operator+ (const TMatrix3& lmat3, const TMatrix3& rmat3)
	{
		TMatrix3 result;

		for (I32 i = 0; i < 3; ++i)
		{
			for (I32 j = 0; j < 3; ++j)
			{
				result.m[i][j] = lmat3.m[i][j] + rmat3.m[i][j];
			}
		}

		return result;
	}

	TMatrix3 operator- (const TMatrix3& lmat3, const TMatrix3& rmat3)
	{
		TMatrix3 result;

		for (I32 i = 0; i < 3; ++i)
		{
			for (I32 j = 0; j < 3; ++j)
			{
				result.m[i][j] = lmat3.m[i][j] - rmat3.m[i][j];
			}
		}

		return result;
	}

	TMatrix3 operator* (const TMatrix3& lmat3, const TMatrix3& rmat3)
	{
		TMatrix3 result;

		for (I32 i = 0; i < 3; ++i)
		{
			for (I32 j = 0; j < 3; ++j)
			{
				for (I32 k = 0; k < 3; ++k)
				{
					result.m[i][j] += lmat3.m[i][k] * rmat3.m[k][j];
				}
			}
		}

		return result;
	}

	TVector3 operator* (const TMatrix3& mat3, const TVector3& vec4)
	{
		F32 result[4];

		for (I32 i = 0; i < 3; ++i)
		{
			result[i] = mat3.m[i][0] * vec4.x + mat3.m[i][1] * vec4.y + mat3.m[i][2] * vec4.z;
		}

		return TVector3(result);
	}

	TMatrix3 operator* (const TMatrix3& mat3, const F32& coeff)
	{
		TMatrix3 result;

		for (I32 i = 0; i < 3; ++i)
		{
			for (I32 j = 0; j < 3; ++j)
			{
				result.m[i][j] = mat3.m[i][j] * coeff;
			}
		}

		return result;
	}

	TMatrix3 operator* (const F32& coeff, const TMatrix3& mat3)
	{
		TMatrix3 result;

		for (I32 i = 0; i < 3; ++i)
		{
			for (I32 j = 0; j < 3; ++j)
			{
				result.m[i][j] = mat3.m[i][j] * coeff;
			}
		}

		return result;
	}


	TDE2_API TMatrix3 Mul(const TMatrix3& lmat3, const TMatrix3& rmat3)
	{
		return lmat3 * rmat3;
	}

	TDE2_API TVector3 Mul(const TMatrix3& mat3, const TVector3& vec4)
	{
		return mat3 * vec4;
	}

	TDE2_API TMatrix3 Inverse(const TMatrix3& mat3)
	{
		TMatrix3 inversed = ZeroMatrix3;

		F32 det = Det(mat3);

		if (fabs(det) < FloatEpsilon)
		{
			return inversed;
		}

		F32 invDet = 1.0f / det;

		//compute adjoint matrix adjA = (A*)^T, where A* is composed from minors of A
		inversed.m[0][0] = invDet * (mat3.m[1][1] * mat3.m[2][2] - mat3.m[1][2] * mat3.m[2][1]);
		inversed.m[0][1] = invDet * (mat3.m[0][2] * mat3.m[2][1] - mat3.m[0][1] * mat3.m[2][2]);
		inversed.m[0][2] = invDet * (mat3.m[0][2] * mat3.m[1][1] - mat3.m[0][1] * mat3.m[1][2]);
												  				 			    
		inversed.m[1][0] = invDet * (mat3.m[1][2] * mat3.m[2][0] - mat3.m[1][0] * mat3.m[2][2]);
		inversed.m[1][1] = invDet * (mat3.m[0][0] * mat3.m[2][2] - mat3.m[0][2] * mat3.m[2][0]);
		inversed.m[1][2] = invDet * (mat3.m[0][2] * mat3.m[1][0] - mat3.m[0][0] * mat3.m[1][2]);
												  				 			    
		inversed.m[2][0] = invDet * (mat3.m[1][0] * mat3.m[2][1] - mat3.m[1][1] * mat3.m[2][0]);
		inversed.m[2][1] = invDet * (mat3.m[0][2] * mat3.m[2][0] - mat3.m[0][0] * mat3.m[2][1]);
		inversed.m[2][2] = invDet * (mat3.m[0][0] * mat3.m[1][1] - mat3.m[0][1] * mat3.m[1][0]);

		return inversed;
	}

	TDE2_API TMatrix3 Transpose(const TMatrix3& mat3)
	{
		TMatrix3 result;

		for (I32 i = 0; i < 3; i++)
		{
			for (I32 j = 0; j < 3; j++)
			{
				result.m[i][j] = mat3.m[j][i];
			}
		}

		return result;
	}

	TDE2_API F32 Det(const TMatrix3& mat3)
	{
		return mat3.m[0][0] * (mat3.m[1][1] * mat3.m[2][2] - mat3.m[1][2] * mat3.m[2][1]) - 
			   mat3.m[0][1] * (mat3.m[1][0] * mat3.m[2][2] - mat3.m[1][2] * mat3.m[2][0]) +
			   mat3.m[0][2] * (mat3.m[1][0] * mat3.m[2][1] - mat3.m[1][1] * mat3.m[2][0]);
	}
}