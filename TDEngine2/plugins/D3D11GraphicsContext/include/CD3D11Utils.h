/*!
	\file CD3D11Utils.h
	\date 23.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <utils/Types.h>


namespace TDEngine2
{
	/*!
		\brief The function is used to release COM pointers in safe manner

		\param[in, out] pPtr A pointer to IUnknown implemenation

		\return RC_OK if everything went ok, or some other code, which describes an error
	*/

	template <typename T>
	inline E_RESULT_CODE SafeReleaseCOMPtr(T** pPtr)
	{
		if (!pPtr)
		{
			return RC_INVALID_ARGS;
		}

		if (FAILED((*pPtr)->Release()))
		{
			return RC_FAIL;
		}

		*pPtr = nullptr;

		return RC_OK;
	}
}